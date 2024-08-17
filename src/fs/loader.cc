#include "fs/loader.h"

#include <algorithm>

#include "common/cstring.h"
#include "common/logger.h"
#if defined(BOARD_IS_QEMU)
#include "drivers/virtio/virtio-blk.h"
#else
#include "fs/fat/fat32.h"
#endif
#include "kernel/sched/sched.h"
#include "kernel/task/task.h"
#include "mm/new.h"
#include "mm/pgtable_stage1.h"
#include "platforms/timer.h"

namespace evisor {

namespace {

bool LoaderLoadFile(Tcb* tsk, const char* name, uint64_t va) {
#if defined(BOARD_IS_QEMU)
  auto& virtio = evisor::VirtioBlk::Get();
  virtio.Init();

  int64_t sector_remains = virtio.GetDiskCapacity() / kDiskSectorSize;
  uint32_t sector_offset = 0;
  uint64_t cur = va & PAGE_MASK;

  while (sector_remains > 0) {
    auto* buf = reinterpret_cast<uint8_t*>(PgTableStage1::PageMap(tsk, cur));

    for (int i = 0; i < PAGE_SIZE / kDiskSectorSize; i++) {
      if (sector_remains <= 0) {
        break;
      }

      if (!virtio.ReadDisk(buf, sector_offset)) {
        LOG_ERROR("Failed to read. sector_offset: %d", sector_offset);
        return false;
      }
      sector_remains--;
      sector_offset++;
      buf += kDiskSectorSize;
    }
    cur += PAGE_SIZE;
  }

  tsk->name = name;
  LOG_INFO("Successfully loaded %s", tsk->name);

  return true;
#else
  Fat32Fs* fs = new Fat32Fs();

  if (!fs->Init()) {
    LOG_ERROR("Failed to init FAT32 filesystem");
    return false;
  }

  fat32_file_t file;
  if (!fs->Open(&file, name)) {
    LOG_ERROR("Failed to open %s.", name);
    return false;
  }
  int remains = fs->GetFileSize(&file);
  int offset = 0;
  uint64_t cur = va & PAGE_MASK;
  const auto total_size = remains;

  LOG_INFO("Start loading %s - %d KB", name, total_size / 1024);
  printf("Progress ");
  int progress_prev = 0;

  const auto start = Timer::GetSystemUsec();
  while (remains > 0) {
    auto* buf = reinterpret_cast<uint8_t*>(PgTableStage1::PageMap(tsk, cur));
    auto req_len = std::min(static_cast<int>(PAGE_SIZE), remains);
    auto reads = fs->Read(&file, buf, offset, req_len);
    if (req_len != reads) {
      LOG_ERROR("Failed to read. requested size: %d, actual size: %d", req_len,
                reads);
      return false;
    }

    remains -= reads;
    offset += reads;
    cur += PAGE_SIZE;

    int progress = static_cast<uint32_t>(
        ((total_size - remains) / static_cast<float>(total_size)) * 100);
    if (progress / 10 > progress_prev) {
      printf(".");
      progress_prev = progress / 10;
    }
  }
  const auto end = Timer::GetSystemUsec();

  printf("\n");
  LOG_INFO("Transfer speed: %d KB/S", total_size * 1000 / (end - start));

  tsk->name = name;
  LOG_INFO("Successfully loaded %s", tsk->name);

  return true;
#endif
}

}  // namespace

bool LoaderLoadVcpu(void* config, uint64_t* pc, uint64_t* sp) {
  auto* cfg = reinterpret_cast<LoaderVcpuConfig*>(config);
  auto* tsk = Sched::Get().GetCurrentTask();

  if (!LoaderLoadFile(tsk, cfg->filename, cfg->file_load_va)) {
    return false;
  }

  *pc = cfg->pc;
  *sp = cfg->sp;
  return true;
}

}  // namespace evisor
