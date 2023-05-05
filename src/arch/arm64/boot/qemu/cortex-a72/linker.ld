/**
 * Linker Script for QEMU coretex-a72/virt/4GB
 **/

OUTPUT_ARCH(aarch64)
ENTRY(_start)

MEMORY
{
  /* 0x40280000 => uboot load address*/
  /* 1GB memory */
	RAM      (xrw)  : ORIGIN = 0x40280000, LENGTH = 1 * 1024 * 1024 * 1024
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
    /* 512MiB user space */
    . = . + 1024 * 1024 * 512;
    . = ALIGN(0x1000);
    __user_space_end = .;
  }
  __user_space_size = __user_space_end - __user_space_start;

  .stack : {
    __stack_start = .;
    /* 64MiB space */ 
    . = ALIGN(0x1000);
    __stack_end = __stack_start + (1024 * 1024) * 64;
  } > RAM
  __stack_size = __stack_end - __stack_start;
}
