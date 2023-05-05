#include "mm/kmm_trap.h"

#include "kernel/sched/sched.h"
#include "mm/pgtable_stage2.h"
#include "mm/user_heap/umm_zalloc.h"

namespace evisor {

bool HandleMmTrapMemoryAccessFault(va_t addr) {
  auto& sched = Sched::Get();
  auto* tsk = sched.GetCurrentTask();
  auto page = reinterpret_cast<va_t>(umm_zalloc(PAGE_SIZE));
  if (!page) {
    return false;
  }

  PgTableStage2::MapNewPage(tsk, addr, page);
  tsk->stat.page_faults++;
  return true;
}

bool HandleMmTrapRegisterAccess(va_t addr, uint8_t srt, bool read) {
  auto& sched = Sched::Get();
  auto* tsk = sched.GetCurrentTask();

  if (tsk->board) {
    auto* vcpu = sched.GetVCpuRegs(tsk);
    if (read) {
      vcpu->regs[srt] = tsk->board->MmioRead(tsk, addr);
    } else {
      tsk->board->MmioWrite(tsk, addr, vcpu->regs[srt]);
    }
  }

  sched.IncrementCurrentTaskPc(4);
  tsk->stat.mmios++;
  return true;
}

}  // namespace evisor
