#ifndef EVISOR_KERNEL_TASK_H_
#define EVISOR_KERNEL_TASK_H_

#include <cstdbool>
#include <cstddef>

#include "arch/arm64/cpu_regs.h"

enum TaskState {
  RUNNING = 0,
  WAITTING = 1,
  ZOMBIE = 2,
  DEAD = 3,
};

struct CpuContext {
  // Callee-saved registers
  uint64_t x19;
  uint64_t x20;
  uint64_t x21;
  uint64_t x22;
  uint64_t x23;
  uint64_t x24;
  uint64_t x25;
  uint64_t x26;
  uint64_t x27;
  uint64_t x28;
  // Frame pointer / x29
  uint64_t fp;
  // Stack pointer
  uint64_t sp;
  // Link register / x30
  uint64_t pc;
};

struct MmContext {
  // Pointer to first page table
  uint64_t page_table;
  // Number of pages used
  uint64_t pages;
};

struct TaskStat {
  bool irq_pending;
  bool fiq_pending;
  uint64_t wfx_traps;
  uint64_t hvc_traps;
  uint64_t sysreg_traps;
  uint64_t page_faults;
  uint64_t mmios;
};

namespace evisor {
class Board;
}

struct Tcb {
  const char* name;
  long pid;
  long priority;
  long counter;
  uint8_t state;
  struct CpuContext cpu_context;
  VCpuSysregs vcpu_sysregs;
  struct MmContext mm;
  struct TaskStat stat;
  evisor::Board* board;
};

#endif  // EVISOR_KERNEL_TASK_H_
