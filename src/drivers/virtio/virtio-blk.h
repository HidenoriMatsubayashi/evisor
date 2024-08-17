#ifndef EVISOR_DRIVERS_VIRTIO_VIRTIO_BLK_H_
#define EVISOR_DRIVERS_VIRTIO_VIRTIO_BLK_H_

#include "drivers/common.h"
#include "mm/pgtable.h"
#include "platforms/qemu/peripheral.h"

namespace evisor {

namespace {

constexpr int kVirtQueueSize = 16;
constexpr int kDiskSectorSize = 512;

}  // namespace

class VirtioBlk {
 public:
  VirtioBlk() = default;
  ~VirtioBlk() = default;

  // Prevent copying.
  VirtioBlk(VirtioBlk const&) = delete;
  VirtioBlk& operator=(VirtioBlk const&) = delete;

  static VirtioBlk& Get() {
    static VirtioBlk instance;
    return instance;
  }

  // Initialize the driver/device.
  void Init();
  // Get disk capacity.
  uint64_t GetDiskCapacity();
  // Read data from the disk.
  bool ReadDisk(void* buf, uint32_t sector_idx);
  // Write data to the disk.
  bool WriteDisk(void* buf, uint32_t sector_idx);

 private:
  // Virtqueue Descriptor Table.
  struct VirtqDesc {
    // Address (guest-physical).
    uint64_t addr;
    // Length.
    uint32_t len;
    // The flags as indicated above.
    uint16_t flags;
    // Next field if flags & NEXT
    uint16_t next;
  } __attribute__((packed));

  // Virtqueue Available Ring.
  struct VirtqAvail {
    uint16_t flags;
    uint16_t index;
    uint16_t ring[kVirtQueueSize];
    uint16_t used_event; /* Only if VIRTIO_F_EVENT_IDX */
  } __attribute__((packed));

  // The format of the first descriptor in a disk request.
  // to be followed by two more descriptors containing
  // the block, and a one-byte status.
  struct VirtioBlkReq {
    uint32_t type;
    uint32_t reserved;
    uint64_t sector;
    uint8_t data[kDiskSectorSize];
    uint8_t status;
  } __attribute__((packed));

  struct VirtqUsedElem {
    // Index of start of used descriptor chain.
    uint32_t id;
    // Total length of the descriptor chain which was used (written to).
    uint32_t len;
  } __attribute__((packed));

  // Virtqueue Used Ring.
  struct VirtqUsed {
    uint16_t flags;
    uint16_t index;
    VirtqUsedElem ring[kVirtQueueSize];
    uint16_t avail_event; /* Only if VIRTIO_F_EVENT_IDX */
  } __attribute__((packed));

  // VirtQueue info to manage the entire virtqueue.
  struct Virtq {
    VirtqDesc descs[kVirtQueueSize];
    VirtqAvail avail;
    VirtqUsed used __attribute__((aligned(PAGE_SIZE)));
    VirtioBlkReq* blk_req;

    // This is used to manage the index that we used last.
    uint16_t last_used_index;
  } __attribute__((packed));

  struct Regs {
    // 0x000: Magic value (RO)
    reg32_t MAGIC_VALUE;
    // 0x004: Device version number (RO)
    reg32_t VERSION;
    // 0x008: Virtio Subsystem Device ID (RO)
    reg32_t DEVICE_ID;
    // 0x00c: Virtio Subsystem Vendor ID (RO)
    reg32_t VENDOR_ID;
    // 0x010: Flags representing features the device supports (RO)
    reg32_t DEVICE_FEATURES;
    // 0x014: Device (host) features word selection (WO)
    reg32_t DEVICE_FEATURES_SEL;
    reg32_t __RESERVED_0[2];
    // 0x020: Flags representing device features understood and activated by the
    // driver (RW)
    reg32_t DRIVER_FEATURES;
    // 0x024: Activated (guest) features word selection (WO)
    reg32_t DRIVER_FEATURES_SEL;
    // 0x028: Guest page size (WO)
    reg32_t GUEST_PAGE_SIZE;
    reg32_t __RESERVED_1;
    // 0x030: Virtual queue index (WO)
    reg32_t QUEUE_SEL;
    // 0x034: Maximum virtual queue size (RO)
    reg32_t QUEUE_NUM_MAX;
    // 0x038: Virtual queue size (WO)
    reg32_t QUEUE_NUM;
    // 0x03c: Used Ring alignment in the virtual queue (WO)
    reg32_t QUEUE_ALIGN;
    // 0x040: Guest physical page number of the virtual queue (R/W)
    reg32_t QUEUE_PFN;
    // 0x044: Virtual queue ready bit (RW)
    reg32_t QUEUE_READY;
    reg32_t __RESERVED_2[2];
    // 0x050: Queue notifier (WO)
    reg32_t QUEUE_NOTIFY;
    reg32_t __RESERVED_3[3];
    // 0x060: Interrupt status (RO)
    reg32_t INTERRUPT_STATUS;
    // 0x064: Interrupt acknowledge (WO)
    reg32_t INTERRUPT_ACK;
    reg32_t __RESERVED_4[2];
    // 0x070: device status (R/W)
    reg32_t DEVICE_STATUS;
    reg32_t __RESERVED_5[35];
    // 0x100+: Configuration space (RW)
    reg32_t DEVICE_CONFIG_SPACE0;
  } __attribute__((packed)) __attribute__((aligned(4)));

  volatile Regs* regs_ = reinterpret_cast<Regs*>(VIRTIO_BASE);
  Virtq* virtq_;

  // Common initialization process for Virtio devices.
  void DeviceInit();
  // Initialization process for the Virtio block device.
  void BlockDeviceInit();
  // Read/Write to Disk device.
  bool ReadWriteDisk(void* buf, uint32_t sector_idx, bool is_write);
  // Returns true if the Virtqueue is busy.
  bool VirtqIsBusy();
};

}  // namespace evisor

#endif  // EVISOR_DRIVERS_VIRTIO_VIRTIO_BLK_H_
