/****************************************************************************
 * Included Files
 ****************************************************************************/
#include "arch/arm64/irq/vectors_local_def.h"
#include "arch/common_asm_macro.h"

/****************************************************************************
 * Assembly Macros
 ****************************************************************************/
#include "arch/arm64/irq/vectors_common.S"

.macro handle_el2_invalid_ventry type
  enter_hypervisor
  mov x0, #\type
  mrs x1, esr_el2
  mrs x2, elr_el2
  mrs x3, far_el2
  bl  CpuCatchUnExpectedIrqs
  wfe
.endm

.macro ventry label
  b \label
  .balign 0x80
.endm

/****************************************************************************
 * Public Functions
 ****************************************************************************/
/*
 * Exception vectors for EL2.
 */
.balign 0x800
GLOBAL_FUNCTION(vector_table_el2)
  // Current EL with SP0
  ventry current_el_sp0_sync     // Synchronous
  ventry current_el_sp0_irq      // IRQ or vIRQ
  ventry current_el_sp0_fiq      // FIQ or vFIQ
  ventry current_el_sp0_serror   // SError or vSError
  // Current EL with SPx
  ventry current_el_spx_sync     // Synchronous
  ventry current_el_spx_irq      // IRQ or vIRQ
  ventry current_el_spx_fiq      // FIQ or vFIQ
  ventry current_el_spx_serror   // SError or vSError
  // Lower EL (64-bit EL0 or 1)
  ventry lower_el_aarch64_sync   // Synchronous
  ventry lower_el_aarch64_irq    // IRQ or vIRQ
  ventry lower_el_aarch64_fiq    // FIQ or vFIQ
  ventry lower_el_aarch64_serror // SError or vSError
  // Lower EL (32-bit EL0 or 1)
  ventry lower_el_aarch32_sync   // Synchronous
  ventry lower_el_aarch32_irq    // IRQ or vIRQ
  ventry lower_el_aarch32_fiq    // FIQ or vFIQ
  ventry lower_el_aarch32_serror // SError or vSError

current_el_sp0_sync:
  handle_el2_invalid_ventry SYNC_INVALID_SP0_EL2
current_el_sp0_irq:
  handle_el2_invalid_ventry IRQ_INVALID_SP0_EL2
current_el_sp0_fiq:
  handle_el2_invalid_ventry FIQ_INVALID_SP0_EL2
current_el_sp0_serror:
  handle_el2_invalid_ventry ERROR_INVALID_SP0_EL2

current_el_spx_sync:
  handle_el2_invalid_ventry SYNC_INVALID_SPX_EL2
current_el_spx_fiq:
  handle_el2_invalid_ventry FIQ_INVALID_SPX_EL2
current_el_spx_serror:
  handle_el2_invalid_ventry ERROR_INVALID_SPX_EL2

lower_el_aarch64_fiq:
  handle_el2_invalid_ventry FIQ_INVALID_EL01_64
lower_el_aarch64_serror:
  handle_el2_invalid_ventry ERROR_INVALID_EL01_64

lower_el_aarch32_sync:
  handle_el2_invalid_ventry SYNC_INVALID_EL01_32
lower_el_aarch32_irq:
  handle_el2_invalid_ventry IRQ_INVALID_EL01_32
lower_el_aarch32_fiq:
  handle_el2_invalid_ventry FIQ_INVALID_EL01_32
lower_el_aarch32_serror:
  handle_el2_invalid_ventry ERROR_INVALID_EL01_32
