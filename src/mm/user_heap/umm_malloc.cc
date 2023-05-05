#include "mm/user_heap/umm_malloc.h"

#include <cstdbool>
#include <cstdint>

#include "arch/ld_symbols.h"
#include "common/logger.h"
#include "mm/pgtable.h"

namespace evisor {

namespace {
// FIXME: use kUserSize
constexpr uint32_t kPagingPages =
    static_cast<uint64_t>(1024) * 1024 * 1024 * 2 / PAGE_SIZE;
constexpr uint32_t kMemMapSize = kPagingPages / 8;

uint8_t userMemoryRegionMap_[kPagingPages] = {0};
uint32_t nextFreeSpaceIndex = 0;
}  // namespace

void* umm_malloc(size_t size) {
  if (size < PAGE_SIZE) {
    LOG_ERROR("Requested size (%d) is less than page size(%d)", size,
              PAGE_SIZE);
    return nullptr;
  }

  if (size % PAGE_SIZE != 0) {
    LOG_ERROR("Requested size (%d) is not aligned to page size(%d)", size,
              PAGE_SIZE);
    return nullptr;
  }

  for (uint32_t i = nextFreeSpaceIndex; i < kPagingPages; i++) {
    const uint32_t j = i / 8;
    const uint8_t bit = 1 << (i % 8);
    if ((userMemoryRegionMap_[j] & bit) == 0) {
      userMemoryRegionMap_[j] |= bit;
      nextFreeSpaceIndex = i + 1;
      auto* page = reinterpret_cast<uint64_t*>(kUserStart + i * PAGE_SIZE);
      return page;
    }
  }

  PANIC("No free pages in user memory region!");
  return nullptr;
}

void umm_free(void* va) {
  const uint32_t page =
      (reinterpret_cast<uint64_t>(va) - kUserStart) / PAGE_SIZE;
  const uint32_t i = page / 8;
  const uint8_t bit = 1 << (page % 8);
  userMemoryRegionMap_[i] &= ~bit;
}

}  // namespace evisor
