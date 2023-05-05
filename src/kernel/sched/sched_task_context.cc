#include "arch/arm64/mmu.h"
#include "kernel/sched/sched.h"
#include "kernel/task/task_config.h"

namespace evisor {

void Sched::IncrementCurrentTaskPc(int offset) {
  auto* tsk = GetCurrentTask();
  auto* regs = GetVCpuRegs(tsk);
  regs->pc += offset;
}

void Sched::RunVcpu(Tcb* tsk) {
  Mmu::SetStage2PageTable(tsk->mm.page_table, tsk->pid);
  CpuRegLoadVCpuSysregs(&tsk->vcpu_sysregs);
  SetCpuVirtualInterrupt(tsk);
}

void Sched::StopVcpu(Tcb* tsk) { CpuRegStoreVCpuSysregs(&tsk->vcpu_sysregs); }

Sched::VCpuContext* Sched::GetVCpuRegs(Tcb* tsk) {
  auto p = reinterpret_cast<uint64_t>(tsk) + TCB_SIZE - sizeof(VCpuContext);
  return reinterpret_cast<VCpuContext*>(p);
}

}  // namespace evisor
