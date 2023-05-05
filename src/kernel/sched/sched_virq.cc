#include "arch/arm64/irq/cpu_irq.h"
#include "kernel/sched/sched.h"
#include "platforms/platform.h"

namespace evisor {

void Sched::SetCpuVirtualInterrupt(Tcb* tsk) {
  // FIXME
  static bool is_asserted = false;

  if (!is_asserted && tsk->board && tsk->board->IsIrqAsserted(tsk)) {
    CpuEnableVIrqEl1();
    is_asserted = true;
  } else if (is_asserted && tsk->board && !tsk->board->IsIrqAsserted(tsk)) {
    CpuDisableVirqEl1();
    is_asserted = false;
  }

  if (tsk->board && tsk->board->IsFiqAsserted(tsk)) {
    CpuEnableVFiqEl1();
    tsk->stat.fiq_pending = false;
  } else {
    CpuDisableVFiqEl1();
  }
}

}  // namespace evisor
