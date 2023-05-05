#include "kernel/vm/vm.h"

#include "arch/arm64/cpu_regs.h"
#include "common/logger.h"
#include "kernel/sched/sched.h"
#include "platforms/board.h"

void VmEnter() {
  auto& sched = evisor::Sched::Get();
  auto* next_vcpu = sched.GetCurrentTask();
  if (!next_vcpu) {
    return;
  }

  if (next_vcpu->board) {
    next_vcpu->board->VmEnter(next_vcpu);
  }

  if (next_vcpu->pid == sched.GetCurrentPidUsingConsole()) {
    sched.FlushConsole(next_vcpu);
  }

  sched.RunVcpu(next_vcpu);
}

void VmLeave() {
  auto& sched = evisor::Sched::Get();
  auto* cur_vcpu = sched.GetCurrentTask();
  if (!cur_vcpu) {
    return;
  }

  if (cur_vcpu->board) {
    cur_vcpu->board->VmLeave(cur_vcpu);
  }

  if (cur_vcpu->pid == sched.GetCurrentPidUsingConsole()) {
    sched.FlushConsole(cur_vcpu);
  }

  sched.StopVcpu(cur_vcpu);
}
