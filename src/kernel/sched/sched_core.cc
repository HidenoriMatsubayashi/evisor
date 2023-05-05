#include <cstdbool>

#include "arch/arm64/arm_generic_timer.h"
#include "arch/arm64/irq/gic_v2.h"
#include "arch/sched.h"
#include "common/logger.h"
#include "kernel/sched/sched.h"
#include "mm/pgtable.h"

namespace evisor {

namespace {

constexpr Tcb kInitTask = {
    .name = "init",
    .pid = 0,
    .priority = 1,
    .counter = 0,
    .state = RUNNING,
    .cpu_context =
        {
            .x19 = 0,
            .x20 = 0,
            .x21 = 0,
            .x22 = 0,
            .x23 = 0,
            .x24 = 0,
            .x25 = 0,
            .x26 = 0,
            .x27 = 0,
            .x28 = 0,
            .fp = 0,
            .sp = 0,
            .pc = 0,
        },
    .vcpu_sysregs = {},
    .mm =
        {
            .page_table = 0,
            .pages = 0,
        },
    .stat =
        {
            .irq_pending = false,
            .fiq_pending = false,
            .wfx_traps = 0,
            .hvc_traps = 0,
            .sysreg_traps = 0,
            .page_faults = 0,
            .mmios = 0,
        },
    .board = nullptr,
};

const char* kTaskStateNames[] = {
    "RUNNING",
    "WAITTING"
    "ZOMBIE",
    "DEAD",
};

// Non-secure EL2 physical timer (Hypervisor timer)
constexpr uint16_t kIrqIdEl2PhysicalTimer = 26;
constexpr uint8_t kEl2PhysicalTimerIrqPriority = 0xca;
constexpr uint32_t kSchedTimerIntervalUsec = 1000;

}  // namespace

void Sched::Init() {
  tsks_[0] = const_cast<Tcb*>(&(kInitTask));
  cur_tsk_ = tsks_[0];
  count_tsks_ = 1;
  StartSchedTimer();
}

void Sched::StartSchedTimer() {
  auto& timer = ArmGenericTimer::Get();

  GicV2::Get().RegisterIrq(kIrqIdEl2PhysicalTimer, 0,
                           kEl2PhysicalTimerIrqPriority, [this, &timer]() {
                             timer.HandleIrq();
                             SchedTimerHandler();
                           });

  timer.Start(kSchedTimerIntervalUsec);
}

void Sched::Schedule() {
  cur_tsk_->counter = 0;
  ScheduleInternal();
}

int Sched::AddTask(Tcb* tsk) {
  auto pid = count_tsks_++;
  tsks_[pid] = tsk;
  tsk->pid = pid;
  return pid;
}

void Sched::ExitTask(Tcb* tsk) {
  for (auto i = 0; i < count_tsks_; i++) {
    if (tsks_[i] == tsk) {
      tsks_[i]->state = ZOMBIE;
      break;
    }
  }
  Schedule();
}

void Sched::PrintTasks() {
  printf("\n%3s %12s %8s %8s %7s %7s %7s %9s %7s %7s %7s\n", "PID", "NAME",
         "STATE", "PC", "PAGES", "PF", "MEM", "WFx", "HVC", "REG", "I/O");
  for (auto i = 0; i < count_tsks_; i++) {
    auto* tsk = tsks_[i];
    const auto* cpu_sysregs = GetVCpuRegs(tsk);
    printf("%3d %12s %8s %8x %7d %7d %7d %9d %7d %7d %7d\n", tsk->pid,
           tsk->name, kTaskStateNames[tsk->state], cpu_sysregs->pc,
           tsk->mm.pages, tsk->stat.page_faults,
           (PAGE_SIZE * tsk->mm.pages) / 1024, tsk->stat.wfx_traps,
           tsk->stat.hvc_traps, tsk->stat.sysreg_traps, tsk->stat.mmios);
  }
}

Tcb* Sched::GetCurrentTask() const { return cur_tsk_; }

Tcb* Sched::GetTask(int pid) const {
  if (pid >= count_tsks_) {
    LOG_ERROR("PID should be less than %d", count_tsks_);
    return nullptr;
  }
  return tsks_[pid];
}

void Sched::SchedTimerHandler() {
  if (--cur_tsk_->counter > 0) {
    return;
  }
  cur_tsk_->counter = 0;
  ScheduleInternal();
}

void Sched::ScheduleInternal() {
  int next;

  while (true) {
    int c = -1;
    next = 0;

    // Skip init task.
    for (auto i = 0; i < count_tsks_; i++) {
      auto* task = tsks_[i];
      if (task && task->state == RUNNING && task->counter > c) {
        c = task->counter;
        next = i;
      }
    }

    if (c) {
      break;
    }

    for (auto i = 0; i < count_tsks_; i++) {
      auto* task = tsks_[i];
      if (task) {
        task->counter = (task->counter >> 1) + task->priority;
      }
    }
  }
  SwitchTaskTo(tsks_[next]);
}

void Sched::SwitchTaskTo(Tcb* next) {
  if (cur_tsk_ == next) {
    return;
  }

  auto* prev = cur_tsk_;
  cur_tsk_ = next;
  SchedContextSwitch(&prev->cpu_context, &next->cpu_context);
}

}  // namespace evisor
