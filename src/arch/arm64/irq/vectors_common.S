/****************************************************************************
 * Included Files
 ****************************************************************************/
#include "arch/arm64/irq/vectors_local_def.h"

/****************************************************************************
 * Assembly Macros
 ****************************************************************************/
/* Context switch to EL2 from EL1.
 * Save registers with exception context from EL1 task stack.
 * note: allocate stackframe with CPU_CONTEXT_GP_REGS
 */
.macro enter_hypervisor
  sub sp, sp, #8 * CPU_CONTEXT_GP_REGS

  // Save the current task's x0 - x29
  stp x0,   x1, [sp, #16 *  0]
  stp x2,   x3, [sp, #16 *  1]
  stp x4,   x5, [sp, #16 *  2]
  stp x6,   x7, [sp, #16 *  3]
  stp x8,   x9, [sp, #16 *  4]
  stp x10, x11, [sp, #16 *  5]
  stp x12, x13, [sp, #16 *  6]
  stp x14, x15, [sp, #16 *  7]
  stp x16, x17, [sp, #16 *  8]
  stp x18, x19, [sp, #16 *  9]
  stp x20, x21, [sp, #16 * 10]
  stp x22, x23, [sp, #16 * 11]
  stp x24, x25, [sp, #16 * 12]
  stp x26, x27, [sp, #16 * 13]
  stp x28, x29, [sp, #16 * 14]

  // Save the current task's x30 and SP
  add x21, sp, #8 * CPU_CONTEXT_GP_REGS
  stp x30, x21, [sp, #16 * 15]
  
  // Save the current task's ELR and SPSR
  mrs x22, elr_el2
  mrs x23, spsr_el2
  stp x22, x23, [sp, #16 * 16]

  bl VmLeave
.endm

/* Context switch to EL1 from EL2.
 * Restore registers with exception context from EL2 task stack.
 * note: allocate stackframe with CPU_CONTEXT_GP_REGS
 */
.macro leave_hypervisor
  bl VmEnter

  // Restore the next task's x30
  ldp x30, x21, [sp, #16 * 15]

  // Restore the next task's ELR and SPSR
  ldp x22, x23, [sp, #16 * 16]
  msr elr_el2, x22
  msr spsr_el2, x23

  // Restore the next task's x0 - x29
  ldp x0,   x1, [sp, #16 *  0]
  ldp x2,   x3, [sp, #16 *  1]
  ldp x4,   x5, [sp, #16 *  2]
  ldp x6,   x7, [sp, #16 *  3]
  ldp x8,   x9, [sp, #16 *  4]
  ldp x10, x11, [sp, #16 *  5]
  ldp x12, x13, [sp, #16 *  6]
  ldp x14, x15, [sp, #16 *  7]
  ldp x16, x17, [sp, #16 *  8]
  ldp x18, x19, [sp, #16 *  9]
  ldp x20, x21, [sp, #16 * 10]
  ldp x22, x23, [sp, #16 * 11]
  ldp x24, x25, [sp, #16 * 12]
  ldp x26, x27, [sp, #16 * 13]
  ldp x28, x29, [sp, #16 * 14]

  // Restore the next task's SP
  add sp, sp, #8 * CPU_CONTEXT_GP_REGS

  // Mode change from EL2 to EL1
  eret
.endm
