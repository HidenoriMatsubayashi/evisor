#include "mm/heap/kmm_malloc.h"

#include <cstdbool>
#include <cstdint>

#include "arch/ld_symbols.h"
#include "common/logger.h"
#include "mm/pgtable.h"

// FIXME
/* 64MiB user space */
#define HEAP_PAGES (static_cast<uint64_t>(1024 * 1024 * 64) / PAGE_SIZE)

namespace evisor {

namespace {
bool kernelHeapMemMap[HEAP_PAGES] = {false};
}  // namespace

void* kmm_malloc([[maybe_unused]] size_t size) {
  if (size < 1) {
    return nullptr;
  }

  // FIXME: need to consider 'size' var
  for (uint32_t i = 0; i < HEAP_PAGES; i++) {
    if (!kernelHeapMemMap[i]) {
      kernelHeapMemMap[i] = true;
      return reinterpret_cast<uint64_t*>(kHeapStart + i * PAGE_SIZE);
    }
  }
  PANIC("No free space!");
  return nullptr;
}

void kmm_free(void* va) {
  kernelHeapMemMap[(reinterpret_cast<uint64_t>(va) - kHeapStart) / PAGE_SIZE] =
      false;
}

}  // namespace evisor
