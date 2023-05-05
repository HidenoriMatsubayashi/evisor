OUTPUT_ARCH(aarch64)
ENTRY(_start)

SECTIONS
{
  . = 0x0000000000000000;

  .text : {
    __text_start = .;
    KEEP(*(.text.start))
    *(.text*)
    . = ALIGN(0x1000);
   }

  .rodata : {
    *(.rodata*)
    . = ALIGN(0x1000);
  }

  .data : {
    *(.data*)
    . = ALIGN(0x1000);
  }

  .bss : {
    __bss_start = . ;
    *(.bss*)
    . = ALIGN(0x1000);
    __bss_end = . ;
  }
  __bss_size = __bss_end - __bss_start;
}
