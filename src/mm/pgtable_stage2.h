#ifndef EVISOR_MM_PGTABLE_STAGE2_H_
#define EVISOR_MM_PGTABLE_STAGE2_H_

#include "kernel/task/task.h"
#include "mm/pgtable.h"
#include "platforms/platform.h"

namespace evisor {

class PgTableStage2 {
 public:
  static void MapPageAccessible(Tcb* task, ipa_t ipa, pa_t page);
  static void MapNewPage(Tcb* tsk, ipa_t ipa, va_t page);
  static void MapNewDevicePage(Tcb* task,
                               ipa_t ipa,
                               pa_t page,
                               bool accessable);
  static pa_t GetIpa(va_t va);

 private:
  PgTableStage2() = default;
  ~PgTableStage2() = default;

  static void* MapPage(Tcb* task, ipa_t ipa, pa_t page, uint64_t flags);
  static pa_t CreatePageTable(va_t table, uint64_t shift, ipa_t ipa);
  static void* SetPageTableEntry(va_t pte, ipa_t ipa, pa_t pa, uint64_t flags);

  // Flush D-Cache
  static void FlushDCache(void* start);

  static void FlushTlbVMID();
};

}  // namespace evisor

#endif  // EVISOR_MM_PGTABLE_STAGE2_H_
