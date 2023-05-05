#ifndef EVISOR_MM_PGTABLE_STAGE1_H_
#define EVISOR_MM_PGTABLE_STAGE1_H_

#include "kernel/task/task.h"
#include "mm/pgtable.h"

namespace evisor {

class PgTableStage1 {
 public:
  static void* PageAllocate();
  static void PageDeallocate(void* page);
  static void* PageMap(Tcb* tsk, ipa_t ipa);

 private:
  PgTableStage1() = default;
  ~PgTableStage1() = default;
};

}  // namespace evisor

#endif  // EVISOR_MM_PGTABLE_STAGE1_H_
