#include "drivers/virtio/virtio-blk.h"

#include <cstdint>

#include "common/cstring.h"
#include "common/logger.h"
#include "common/macro.h"
#include "mm/uncached/kmm_uncached_malloc.h"

// References:
// * http://docs.oasis-open.org/virtio/virtio/v1.3/virtio-v1.3.html
// * https://brennan.io/2020/03/22/sos-block-device/

namespace evisor {

namespace {

/*
 * Device status register
 */
// Indicates that the guest OS has found the device and recognized it as a valid
// virtio device.
constexpr uint32_t kDeviceStatusAck = BIT32(0);
// Indicates that the guest OS knows how to drive the device. Note: There could
// be a significant (or infinite) delay before setting this bit. For example,
// under Linux, drivers can be loadable modules.
constexpr uint32_t kDeviceStatusDriver = BIT32(1);
//
constexpr uint32_t kDeviceStatusDriverOk = BIT32(2);
//
constexpr uint32_t kDeviceStatusDriverFeatOk = BIT32(3);

/*
 * Descriptor flags
 */
// This marks a buffer as continuing via the next field.
constexpr uint16_t kVRingDescFNext = 0x1;
// This marks a buffer as device write-only (otherwise device read-only).
constexpr uint16_t kVRingDescFWrite = 0x2;
// This means the buffer contains a list of buffer descriptors.
[[maybe_unused]] constexpr uint16_t kVRingDescFIndirect = 0x4;

/*
 * Device operation flags
 */
constexpr uint32_t kVirtioBlkTIn = 0;
constexpr uint32_t kVirtioBlkTOut = 1;
[[maybe_unused]] constexpr uint32_t kVirtioBlkTFlush = 4;
[[maybe_unused]] constexpr uint32_t kVirtioBlkTDiscard = 11;
[[maybe_unused]] constexpr uint32_t kVirtioBlkTWriteZeros = 13;

constexpr uint8_t kVirtioBlkStatusOk = 0;
[[maybe_unused]] constexpr uint8_t kVirtioBlkStatusIOError = 1;
[[maybe_unused]] constexpr uint8_t kVirtioBlkStatusUnSupp = 2;

}  // namespace

void VirtioBlk::Init() {
  DeviceInit();
  BlockDeviceInit();

  LOG_INFO("virtio: %d bytes capacity", GetDiskCapacity());

  // Allocate a memory space for a disk request.
  virtq_->blk_req = reinterpret_cast<VirtioBlkReq*>(evisor::kmm_uncached_malloc(
      __builtin_align_up(sizeof(VirtioBlkReq), PAGE_SIZE)));
}

uint64_t VirtioBlk::GetDiskCapacity() {
  return regs_->DEVICE_CONFIG_SPACE0 * kDiskSectorSize;
}

bool VirtioBlk::ReadDisk(void* buf, uint32_t sector_idx) {
  return ReadWriteDisk(buf, sector_idx, false);
}

bool VirtioBlk::WriteDisk(void* buf, uint32_t sector_idx) {
  return ReadWriteDisk(buf, sector_idx, true);
}

void VirtioBlk::DeviceInit() {
  // Leagacy device returns 0x74726976, which means string "virt".
  if (regs_->MAGIC_VALUE != 0x74726976) {
    PANIC("virtio: unexpected magic value: %x", regs_->MAGIC_VALUE);
  }

  // Legacy device returns value 0x1.
  if (regs_->VERSION != 1) {
    PANIC("virtio: unexpected version: %x", regs_->VERSION);
  }

  // Device type: 1 is net, 2 is disk.
  if (regs_->DEVICE_ID != 2) {
    PANIC("virtio: the device is not a disk: %x", regs_->DEVICE_ID);
  }

  // Reset the device.
  regs_->DEVICE_STATUS = 0;

  // Set ACKNOWLEDGE bit which informs the device.
  regs_->DEVICE_STATUS = regs_->DEVICE_STATUS | kDeviceStatusAck;

  // Set DRIVER bit which states that we have a driver for the device.
  regs_->DEVICE_STATUS = regs_->DEVICE_STATUS | kDeviceStatusDriver;

  // Set FEATURES_OK bit.
  regs_->DEVICE_STATUS = regs_->DEVICE_STATUS | kDeviceStatusDriverFeatOk;
  if (!(regs_->DEVICE_STATUS & kDeviceStatusDriverFeatOk)) {
    PANIC("virtio: FEATURES_OK bit was cleared for some reason: %x",
          regs_->DEVICE_STATUS);
  }
}

void VirtioBlk::BlockDeviceInit() {
  virtq_ = reinterpret_cast<Virtq*>(evisor::kmm_uncached_malloc(
      __builtin_align_up(sizeof(Virtq), PAGE_SIZE)));

  // Initialize the index of the Virtqueue Used Ring.
  virtq_->used.index = 0;
  virtq_->last_used_index = 0;

  // Initialize queue 0.
  {
    regs_->QUEUE_SEL = 0;
    // Ensure queue 0 is not in use.
    if (regs_->QUEUE_READY) {
      PANIC("virtio: The disk has already been used!");
    }
  }

  // Set queue size.
  regs_->QUEUE_NUM = kVirtQueueSize;

  // Set the Used Ring alignment in the virtqueue.
  regs_->QUEUE_ALIGN = PAGE_SIZE;

  // Set the descriptor table head address.
  regs_->QUEUE_PFN = reinterpret_cast<uint64_t>(virtq_->descs);

  // Queue is ready.
  regs_->QUEUE_READY = 1;

  // Notify the device of we're ready.
  regs_->DEVICE_STATUS = regs_->DEVICE_STATUS | kDeviceStatusDriverOk;
}

bool VirtioBlk::ReadWriteDisk(void* buf, uint32_t sector_idx, bool is_write) {
  const auto sector_nums = GetDiskCapacity() / kDiskSectorSize;
  if (sector_idx >= sector_nums) {
    LOG_ERROR("virtio: The sector (%d) is over the capacity (%d)", sector_idx,
              sector_nums);
    return false;
  }

  // Format the three descriptors.
  {
    virtq_->blk_req->type = is_write ? kVirtioBlkTOut : kVirtioBlkTIn;
    virtq_->blk_req->reserved = 0;
    virtq_->blk_req->sector = sector_idx;
    if (is_write) {
      memcpy(virtq_->blk_req->data, buf, kDiskSectorSize);
    }

    {
      auto blk_req_addr = reinterpret_cast<uint64_t>(virtq_->blk_req);

      virtq_->descs[0].addr = blk_req_addr;
      virtq_->descs[0].len = sizeof(uint32_t) * 2 + sizeof(uint64_t);
      virtq_->descs[0].flags = kVRingDescFNext;
      virtq_->descs[0].next = 1;

      virtq_->descs[1].addr = blk_req_addr + offsetof(VirtioBlkReq, data);
      virtq_->descs[1].len = kDiskSectorSize;
      if (is_write) {
        virtq_->descs[1].flags = 0;  // device reads the data
      } else {
        virtq_->descs[1].flags = kVRingDescFWrite;  // device writes the data
      }
      virtq_->descs[1].flags |= kVRingDescFNext;
      virtq_->descs[1].next = 2;

      virtq_->blk_req->status = 0xff;  // device writes 0 on success
      virtq_->descs[2].addr = blk_req_addr + offsetof(VirtioBlkReq, status);
      virtq_->descs[2].len = sizeof(uint8_t);
      virtq_->descs[2].flags = kVRingDescFWrite;  // device writes the status
    }
  }

  // Start the virtio operation
  {
    int desc_start_idx = 0;

    // Notify the device of the first index in our chain of descriptors.
    virtq_->avail.ring[virtq_->avail.index % kVirtQueueSize] = desc_start_idx;
    __sync_synchronize();

    // Notify the device of another avail ring entry is available.
    virtq_->avail.index++;
    __sync_synchronize();

    regs_->QUEUE_NOTIFY = 0;
    virtq_->last_used_index++;

    // Wait for the device to say the request has finished.
    while (VirtqIsBusy()) {
      ;
    }
    if (virtq_->blk_req->status != kVirtioBlkStatusOk) {
      LOG_ERROR("virtio: failed to transfor the sector (%d), status (%d)",
                sector_idx, virtq_->blk_req->status);
      return false;
    }
  }

  if (!is_write) {
    memcpy(buf, virtq_->blk_req->data, kDiskSectorSize);
  }

  return true;
}

bool VirtioBlk::VirtqIsBusy() {
  // The device will increment used.index when it adds an entry to the used
  // ring.
  return virtq_->last_used_index != virtq_->used.index;
}

}  // namespace evisor
