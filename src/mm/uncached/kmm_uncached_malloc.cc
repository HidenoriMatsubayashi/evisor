#include "mm/uncached/kmm_uncached_malloc.h"

#include <cstdbool>

#include "arch/ld_symbols.h"
#include "common/assert.h"
#include "common/logger.h"
#include "mm/mm_local.h"
#include "mm/pgtable.h"

namespace evisor {

namespace {

// TODO: use `kUserSize` variable
const size_t kRegionPageNums = static_cast<size_t>(1) * 1024 * 1024 / PAGE_SIZE;
PageBlock kernelUncachedMemoryRegionMap_[kRegionPageNums] = {};

bool kmm_is_spaces_available(size_t start, size_t num_pages) {
  for (size_t i = 0; i < num_pages; i++) {
    if (kernelUncachedMemoryRegionMap_[start + i].ref_count != 0) {
      return false;
    }
  }
  return true;
}

}  // namespace

void* kmm_uncached_malloc(size_t size) {
  ASSERT(size >= 1, "The size value should be 1 or more");

  const size_t aligned_size = __builtin_align_up(size, PAGE_SIZE);
  const size_t num_pages = aligned_size / PAGE_SIZE;

  for (size_t i = 0; i < kRegionPageNums; i++) {
    const uint64_t paddr = kUncachedStart + i * PAGE_SIZE;
    if (kmm_is_spaces_available(i, num_pages)) {
      for (size_t j = 0; j < num_pages; j++) {
        auto& page = kernelUncachedMemoryRegionMap_[i + j];
        page.ref_count++;
        page.size = num_pages;
      }
      return reinterpret_cast<uint64_t*>(paddr);
    }
  }

  PANIC("No free space!");
  return nullptr;
}

void kmm_uncached_free(void* va) {
  ASSERT(va != nullptr, "Address must not be null");

  const auto page_idx =
      (reinterpret_cast<uint64_t>(va) - kUncachedStart) / PAGE_SIZE;
  const size_t page_len = kernelUncachedMemoryRegionMap_[page_idx].size;
  for (size_t i = 0; i < page_len; i++) {
    auto& page = kernelUncachedMemoryRegionMap_[page_idx];
    page.size = 0;
    page.ref_count--;
  }
}

}  // namespace evisor
