#include "mm/pgtable_stage1.h"

#include "mm/user_heap/umm_malloc.h"
#include "mm/user_heap/umm_zalloc.h"
#include "mm/pgtable_stage2.h"

namespace evisor {

void* PgTableStage1::PageAllocate() {
  return reinterpret_cast<void*>(umm_zalloc(PAGE_SIZE));
}

void PgTableStage1::PageDeallocate(void* page) { umm_free(page); }

void* PgTableStage1::PageMap(Tcb* tsk, ipa_t ipa) {
  auto page = reinterpret_cast<pa_t>(PageAllocate());
  PgTableStage2::MapPageAccessible(tsk, ipa, page);
  return reinterpret_cast<void*>(page);
}

}  // namespace evisor
