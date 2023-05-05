#include <cstdbool>

#include "arch/kernel.h"
#include "common/cstring.h"
#include "common/logger.h"
#include "common/queue.h"
#include "kernel/sched/sched.h"
#include "mm/heap/kmm_zalloc.h"
#include "mm/pgtable.h"
#include "platforms/platform.h"

namespace evisor {

namespace {

/* SPSR_EL2 register */
constexpr uint64_t kSpsrEl2_M_El1h = 0b0101;
constexpr uint64_t kSpsrEl2_D_Enable = BIT64(9);
constexpr uint64_t kSpsrEl2_A_Enable = BIT64(8);
constexpr uint64_t kSpsrEl2_I_Enable = BIT64(7);
constexpr uint64_t kSpsrEl2_F_Enable = BIT64(6);

VCpuSysregs initial_vcpu_regs_;

void CreateInitialCpuRegsTemplate(Tcb* tsk) {
  static bool initialized = false;
  if (!initialized) {
    CpuRegLoadVCpuAllSysregs(&initial_vcpu_regs_);
    // Ensure disabling MMU
    initial_vcpu_regs_.sctlr_el1 &= ~1;

    initialized = true;
  }
  memcpy(&tsk->vcpu_sysregs, &initial_vcpu_regs_, sizeof(VCpuSysregs));
}

void LoadTaskImage(loader_func_t loader, void* arg) {
  LOG_INFO("New vCPU loading...");

  auto& sched = Sched::Get();
  auto* tsk = sched.GetCurrentTask();
  auto* regs = sched.GetVCpuRegs(tsk);
  regs->pstate = kSpsrEl2_M_El1h | kSpsrEl2_D_Enable | kSpsrEl2_A_Enable |
                 kSpsrEl2_I_Enable | kSpsrEl2_F_Enable;

  if (!loader(arg, &regs->pc, &regs->sp)) {
    PANIC("Failed to load %s", tsk->name);
  }

  sched.RunVcpu(tsk);
}

}  // namespace

int Sched::CreateTask(loader_func_t loader, void* arg) {
  auto* tsk = static_cast<Tcb*>(kmm_zalloc(PAGE_SIZE));

  tsk->name = "New vCPU";
  tsk->state = RUNNING;
  tsk->cpu_context.x19 = reinterpret_cast<uint64_t>(LoadTaskImage);
  tsk->cpu_context.x20 = reinterpret_cast<uint64_t>(loader);
  tsk->cpu_context.x21 = reinterpret_cast<uint64_t>(arg);
  tsk->priority = 1;
  tsk->counter = tsk->priority;
  tsk->stat.irq_pending = false;
  tsk->stat.fiq_pending = false;

  // Set initial CPU system registers
  CreateInitialCpuRegsTemplate(tsk);

  // Initialize target board
  Board& board = GetPlatformBoard();
  tsk->board = &board;
  if (tsk->board) {
    tsk->board->Init(tsk);
  }

  // Set PC and SP to load the image first when the new vCPU task is dispatched.
  auto& sched = Sched::Get();
  auto* vcpu_context = sched.GetVCpuRegs(tsk);
  tsk->cpu_context.pc = reinterpret_cast<uint64_t>(KernelSwitchFromKThread);
  tsk->cpu_context.sp = reinterpret_cast<uint64_t>(vcpu_context);

  // Register new task to Scheduler
  auto pid = sched.AddTask(tsk);
  return pid;
}

}  // namespace evisor
