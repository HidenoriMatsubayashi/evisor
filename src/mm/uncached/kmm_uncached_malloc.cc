#include "mm/uncached/kmm_uncached_malloc.h"

#include <cstdbool>
#include <cstdint>

#include "arch/ld_symbols.h"
#include "common/logger.h"
#include "mm/pgtable.h"

// FIXME
/* 1MiB uncached space */
#define UNCACHED_PAGES (static_cast<uint64_t>(1024 * 1024 * 1) / PAGE_SIZE)

namespace evisor {

namespace {
bool kernelUncachedMemMap[UNCACHED_PAGES] = {false};
}  // namespace

void* kmm_uncached_malloc(size_t size) {
  if (size < 1) {
    return nullptr;
  }

  // FIXME
  size = size;

  for (uint32_t i = 0; i < UNCACHED_PAGES; i++) {
    if (!kernelUncachedMemMap[i]) {
      kernelUncachedMemMap[i] = true;
      return reinterpret_cast<uint64_t*>(kUncachedStart + i * PAGE_SIZE);
    }
  }
  PANIC("No free space!");
  return nullptr;
}

void kmm_uncached_free(void* va) {
  kernelUncachedMemMap[(reinterpret_cast<uint64_t>(va) - kUncachedStart) /
                       PAGE_SIZE] = false;
}

}  // namespace evisor
