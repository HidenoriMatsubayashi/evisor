/****************************************************************************
 * Included Files
 ****************************************************************************/
#include "arch/arm64/irq/vectors_local_def.h"
#include "arch/common_asm_macro.h"

/****************************************************************************
 * Assembly Macros
 ****************************************************************************/
#include "arch/arm64/irq/vectors_common.S"

/****************************************************************************
 * Public Functions
 ****************************************************************************/
GLOBAL_FUNCTION(current_el_spx_irq)
  enter_hypervisor
  bl CpuHandleIrq
  leave_hypervisor
  msr DAIFClr, #2
  eret

GLOBAL_FUNCTION(lower_el_aarch64_sync)
  enter_hypervisor
  mrs x0, esr_el2
  mrs x1, elr_el2
  mrs x2, far_el2
  mov x3, x8 // HVC number in x8
  bl TrapHandleLowerElAarch64Sync
  leave_hypervisor

GLOBAL_FUNCTION(lower_el_aarch64_irq)
  enter_hypervisor
  bl CpuHandleIrq
  leave_hypervisor
  msr DAIFClr, #2
  eret

// FIXME: move this function from this file to another one...
// This function will be called only one time when the new vCPU task is dispatched first.
// See kernel/sched/sched_create_task.cc
GLOBAL_FUNCTION(KernelSwitchFromKThread)
  mov x0, x20    // loader_func_t loader
  mov x1, x21    // void* arg
  //mov x3, x22    // elr_el2
  //mov x4, x23    // spsr_el2
  blr x19        // LoadTaskImage function pointer in x19
  leave_hypervisor
