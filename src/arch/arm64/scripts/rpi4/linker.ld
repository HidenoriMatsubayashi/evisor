/**
 * Linker Script for Raspberry Pi 4 Model B / 4GB RAM
 **/

OUTPUT_ARCH(aarch64)
ENTRY(_start)

MEMORY
{
  /*
   * PRI boot loader loadds kernel.bin to 0x40280000
   * Physical memory size: 4GB
   */
  RESERVED (r)    : ORIGIN = 0x00000000, LENGTH = 32K
	RAM      (xrw)  : ORIGIN = 0x00080000, LENGTH = (0x100000000 - 0x00080000)
}

SECTIONS {
  __start = .;

  /* text region for Hypervisor */
  .text : ALIGN(0x1000) {
    __text_start = .;
    KEEP(*(.text.start))
    *(.text*)
    . = ALIGN(0x1000);
    __text_end = .;
  } > RAM
  __text_size = __text_end - __text_start;

  /* rodata region for Hypervisor */
  .rodata : ALIGN(0x1000) {
    __rodata_start = .;
    *(.rodata*)
    . = ALIGN(0x1000);
    __rodata_end = .;
  } > RAM
  __rodata_size = __rodata_end - __rodata_start;

  /* data region for Hypervisor */
  .data : ALIGN(0x1000) {
    __data_start = .;
    *(.data*)
    . = ALIGN(0x1000);
    __data_end = .;
  } > RAM
  __data_size = __data_end - __data_start;

  /* bss region for Hypervisor */
  .bss : ALIGN(0x1000) {
    __bss_start = .;
    *(.bss*)
    . = ALIGN(0x1000);
    __bss_end = .;
  } > RAM
  __bss_size = __bss_end - __bss_start;

  /* 64MB stack region for Hypervisor */
  .stack : ALIGN(0x1000) {
    __stack_start = .;
    . = ALIGN(0x1000);
    __stack_end = __stack_start + (1024 * 1024) * 64;
  } > RAM
  __stack_size = __stack_end - __stack_start;

  /* 64MB heap region for Hypervisor */
  .heap : ALIGN(0x1000) {
    __heap_start = .;
    . = . + 1024 * 1024 * 64; 
    . = ALIGN(0x1000);
    __heap_end = .;
  } > RAM
  __heap_size = __heap_end - __heap_start;

  /* 1MB uncached region for Hypervisor */
  .uncached_space : ALIGN(0x1000) {
    __uncached_space_start = .;
    . = . + 1024 * 1024 * 1; 
    . = ALIGN(0x1000);
    __uncached_space_end = .;
  } > RAM
  __uncached_space_size = __uncached_space_end - __uncached_space_start;

  /* 2GB user memory region for guests */
  .user_space : ALIGN(0x1000) {
    __user_space_start = .;
    . = . + 1024 * 1024 * 1024 * 2;
    . = ALIGN(0x1000);
    __user_space_end = .;
  } > RAM
  __user_space_size = __user_space_end - __user_space_start;
}
