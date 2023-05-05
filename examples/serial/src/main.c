#include <stdbool.h>

#include "pl011_uart.h"

#define STRINGIFY(x) #x

#define cpu_read_sysreg(reg)                                             \
  ({                                                                     \
    uint64_t __val;                                                      \
    __asm__ volatile("mrs %0, " STRINGIFY(reg) : "=r"(__val)::"memory"); \
    __val;                                                               \
  })

#define cpu_write_sysreg(reg, val) \
  ({ __asm__ volatile("msr " STRINGIFY(reg) ", %0" : : "r"(val) : "memory"); })

static pl011_uart_regs_t* UART0 = (pl011_uart_regs_t*)UART0_BASE;

static size_t strlen(const char* s) {
  size_t count = 0;
  while (*s != '\0') {
    count++;
    s++;
  }
  return count;
}

static size_t putc(uint8_t c) {
  if (c == '\n') {
    uint8_t cc = '\r';
    pl011_write(UART0, &cc, 1);
  }
  return pl011_write(UART0, &c, 1);
}

static size_t puts(const char* s) {
  size_t size = strlen(s);

  while (size--) {
    putc(*(s++));
  }

  return size;
}

int main() {
  pl011_receive_interrupt_enable(UART0);
  pl011_uart_enable(UART0, UART_REFERENCE_CLOCK, 115200);

  puts("Hello, serial test app.\n");
  puts("EL = ");

  /** 
  * CurrentEL: Current Exception Level
  *  EL, bits [3:2]
  *   Possible values of this field are:
  *   00: EL0
  *   01: EL1
  *   10: EL2
  *   11: EL3
  **/
  uint8_t el = cpu_read_sysreg(CurrentEL) >> 2;
  putc(el + '0');
  puts("\n");

  while (true) {
    ;
    // uart_send(uart_recv());
  }

  return 0;
}
