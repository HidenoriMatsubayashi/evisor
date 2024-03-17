/****************************************************************************
 * Included Files
 ****************************************************************************/
#include "arch/common_asm_macro.h"
#include "platforms/platform_config.h"

/****************************************************************************
 * Public Functions
 ****************************************************************************/
.align 4
.section ".text.start"
GLOBAL_FUNCTION(_start)
  // Disable all interrupts
  // DAIF, Interrupt Mask Bits:
  //   D, bit [3]: Debug mask bit
  //   A, bit [2]: Asynchronous abort mask bit
  //   I, bit [1]: IRQ mask bit
  //   F, bit [0]: FIQ mask bit
  // value: 
  //   0: Interrupt not masked
  //   1: Interrupt masked
  msr DAIFSet, #0xf

  // Set stack pointer
  ldr x0, =__stack_end
  mov sp, x0

  // Use only CPU core-0
  mrs x0, mpidr_el1
  and x0, x0, #0xff

  // Raspberry Pi4/5: RPI bootloader always enters in EL2
  cbz x0, kernel_boot_el2
  b proc_hlt

/****************************************************************************
 * Private Functions
 ****************************************************************************/
.section ".text"
FUNCTION(proc_hlt)
  wfe
  b proc_hlt

FUNCTION(kernel_boot_el2)
  bl bss_clear
  bl BootFromEl2
  bl KernelMain
  b finish

FUNCTION(bss_clear)
  ldr x1, =__bss_start
  ldr x2, =__bss_end
.L__bss_loop:
  cmp x1, x2
  beq .L__bss_loop_done
  str xzr, [x1]
  add x1, x1, #8
  b .L__bss_loop
.L__bss_loop_done:
  ret

FUNCTION(finish)
  wfe
  b finish
