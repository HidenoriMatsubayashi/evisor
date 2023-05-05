.align 4
.section ".text.start"
.globl _start
_start:
  bl main
  hvc #0xcafe

1:
  b 1b
