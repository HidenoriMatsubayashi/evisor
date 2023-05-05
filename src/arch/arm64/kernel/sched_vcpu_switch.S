#include "arch/common_asm_macro.h"

GLOBAL_FUNCTION(SchedContextSwitch)
  // Store callee-saved registers for current vcpu
  mov x8, x0
  mov x9, sp
  stp x19, x20, [x8], #16
  stp x21, x22, [x8], #16
  stp x23, x24, [x8], #16
  stp x25, x26, [x8], #16
  stp x27, x28, [x8], #16
  stp x29, x9, [x8], #16
  str x30, [x8]

  // Re-store callee-saved registers for next vcpu
  mov x8, x1
  ldp x19, x20, [x8], #16		
  ldp x21, x22, [x8], #16
  ldp x23, x24, [x8], #16
  ldp x25, x26, [x8], #16
  ldp x27, x28, [x8], #16
  ldp x29, x9, [x8], #16
  ldr x30, [x8]
  mov sp, x9
  ret
