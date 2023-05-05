#ifndef EVISOR_SCHED_SCHED_H_
#define EVISOR_SCHED_SCHED_H_

#include <array>

#include "kernel/task/task.h"

typedef bool (*loader_func_t)(void*, uint64_t*, uint64_t*);

namespace evisor {

namespace {
constexpr uint8_t kNrTasks = 8;
}  // namespace

class Sched {
 public:
  struct VCpuContext {
    uint64_t regs[31];  // x0 - x30
    uint64_t sp;        // sp
    uint64_t pc;        // ELR_EL2
    uint64_t pstate;    // SPSR_EL2
  };

  Sched() = default;
  ~Sched() = default;

  // Prevent copying.
  Sched(Sched const&) = delete;
  Sched& operator=(Sched const&) = delete;

  static Sched& Get() noexcept {
    static Sched instance;
    return instance;
  }

  void Init();

  void Schedule();

  // Create a new task and load its binary datat
  int CreateTask(loader_func_t loader, void* arg);

  // Add a new task
  int AddTask(Tcb* tsk);

  // Exit a task
  void ExitTask(Tcb* tsk);

  // Increment current task's program counter
  void IncrementCurrentTaskPc(int offset);

  // Set virtual interrupts to vCPU
  void SetCpuVirtualInterrupt(Tcb* tsk);

  // Get current task context block
  Tcb* GetCurrentTask() const;

  // Get a task context block task by specified PID
  Tcb* GetTask(int pid) const;

  // Do context switch to next vCPU from current vCPU.
  // Re-store CPU system registers and Stage2 MMU, etc.
  void RunVcpu(Tcb* tsk);

  // Store current vCPU context.
  void StopVcpu(Tcb* tsk);

  // Get vCPU registers
  VCpuContext* GetVCpuRegs(Tcb* tsk);

  // Assign PID to the active console.
  void ConsoleSwitchTo(uint8_t pid);

  // Get current PID using the active console.
  uint8_t GetCurrentPidUsingConsole();

  // Flush logs in task console
  void FlushConsole(Tcb* tsk);

  // Print task lists
  void PrintTasks();

 private:
  void StartSchedTimer();
  void SchedTimerHandler();
  void ScheduleInternal();
  void SwitchTaskTo(Tcb* next);

  std::array<Tcb*, kNrTasks> tsks_;
  Tcb* cur_tsk_ = nullptr;
  int count_tsks_ = 0;

  // PID is currently assigned to the active console.
  uint8_t console_forwarded_pid_ = 1;
};

}  // namespace evisor

#endif  // EVISOR_SCHED_SCHED_H_
