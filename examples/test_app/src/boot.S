.align 4
.section ".text.start"
.globl _start
_start:
  mov sp, #0x1000
  bl main
  hvc #0xcafe

1:
  b 1b
