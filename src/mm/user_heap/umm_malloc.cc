#include "mm/user_heap/umm_malloc.h"

#include <cstdbool>
#include <cstdint>

#include "arch/ld_symbols.h"
#include "common/logger.h"
#include "mm/pgtable.h"

// FIXME
#define PAGING_PAGES (static_cast<uint64_t>(1024) * 1024 * 1024 * 2 / PAGE_SIZE)

namespace evisor {

namespace {

bool userHeapMemMap[PAGING_PAGES] = {false};

}  // namespace

void* umm_malloc(size_t size) {
  if (size < 1) {
    return nullptr;
  }

  // FIXME
  size = size;

  for (uint32_t i = 0; i < PAGING_PAGES; i++) {
    if (!userHeapMemMap[i]) {
      userHeapMemMap[i] = true;
      auto* page = reinterpret_cast<uint64_t*>(kUserStart + i * PAGE_SIZE);
      return page;
    }
  }
  PANIC("No free pages!");
  return nullptr;
}

void umm_free(void* va) {
  userHeapMemMap[(reinterpret_cast<uint64_t>(va) - kUserStart) / PAGE_SIZE] =
      false;
}

}  // namespace evisor
