/**
 * Linker Script for Raspberry Pi 4 Model B / 4GB RAM
 **/

OUTPUT_ARCH(aarch64)
ENTRY(_start)

MEMORY
{
  /**
   * - Raspberry Pi 4 has 4GiB RAM
   * - "0x00080000" is the Kernel load address for aarch64
   **/
  RESERVED (r)    : ORIGIN = 0x00000000, LENGTH = 32K
	RAM      (xrw)  : ORIGIN = 0x00080000, LENGTH = (0x100000000 - 0x00080000)
}

SECTIONS {
  __start = .;
  .text : {
    __text_start = .;
    KEEP(*(.text.start))
    *(.text*)
    . = ALIGN(0x1000);
    __text_end = .;
  } > RAM
  __text_size = __text_end - __text_start;

  .rodata : {
    __rodata_start = .;
    *(.rodata*)
    . = ALIGN(0x1000);
    __rodata_end = .;
  } > RAM
  __rodata_size = __rodata_end - __rodata_start;

  .data : {
    __data_start = . ;
    *(.data*)
    . = ALIGN(0x1000);
    __data_end = . ;
  } > RAM
  __data_size = __data_end - __data_start;

  .bss : {
    __bss_start = . ;
    *(.bss*)
    . = ALIGN(0x1000);
    __bss_end = . ;
  } > RAM
  __bss_size = __bss_end - __bss_start;

  .heap : {
    __heap_start = .;
    /* 64MiB heap */
    . = . + 1024 * 1024 * 64; 
    . = ALIGN(0x1000);
    __heap_end = .;
  } > RAM
  __heap_size = __heap_end - __heap_start;

  .uncached_space : {
    __uncached_space_start = .;
    /* 1MB uncached regin */
    . = . + 1024 * 1024 * 1; 
    . = ALIGN(0x1000);
    __uncached_space_end = .;
  } > RAM
  __uncached_space_size = __uncached_space_end - __uncached_space_start;

  .user_space : {
    __user_space_start = .;
    /* 2GiB user space */
    . = . + 1024 * 1024 * 1024 * 2;
    . = ALIGN(0x1000);
    __user_space_end = .;
  } > RAM
  __user_space_size = __user_space_end - __user_space_start;

  .stack : {
    __stack_start = .;
    /* 64MiB space */ 
    . = ALIGN(0x1000);
    __stack_end = __stack_start + (1024 * 1024) * 64;
  } > RAM
  __stack_size = __stack_end - __stack_start;
}
