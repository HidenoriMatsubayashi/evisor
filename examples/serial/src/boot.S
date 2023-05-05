#include "sysregs.h"

.align 4
.section ".text.start"
.globl _start
_start:
	mrs	x0, mpidr_el1
	ldr	x0, =SCTLR_VALUE_MMU_DISABLED
	msr	sctlr_el1, x0
  isb

	adr	x0, __bss_start
	adr	x1, __bss_end
	sub	x1, x1, x0
	bl 	memzero

	mov	sp, #0x10000
	bl	main

.globl memzero
memzero:
	str xzr, [x0], #8
	subs x1, x1, #8
	b.gt memzero
	ret
