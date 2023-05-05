/****************************************************************************
 * Included Files
 ****************************************************************************/
#include "pl011_uart.h"

#include <stdbool.h>

void pl011_uart_disable(pl011_uart_regs_t* uart) {
  // Clear UART setting
  uart->CR = 0;

  // disable FIFO
  uart->LCRH = 0;
}

void pl011_baudrate_setup(pl011_uart_regs_t* uart, uint64_t uart_clock,
                          uint64_t baudrate) {
  uint32_t bauddiv = (1000 * uart_clock) / (16 * baudrate);
  uint32_t ibrd = bauddiv / 1000;
  uint32_t fbrd = ((bauddiv - ibrd * 1000) * 64 + 500) / 1000;
  uart->IBRD = ibrd;
  uart->FBRD = fbrd;
}

void pl011_uart_enable(pl011_uart_regs_t* uart, uint64_t uart_clock,
                       uint64_t baudrate) {
  pl011_uart_disable(uart);
  pl011_baudrate_setup(uart, uart_clock, baudrate);

  // 8bit, FIFO
  uart->LCRH = PL011_LCRH_WLEN_8;
  // uart enable, TX/RX enable
  uart->CR = PL011_CR_UARTEN | PL011_CR_TXE | PL011_CR_RXE;
}

size_t pl011_write(pl011_uart_regs_t* uart, uint8_t* buf, size_t size) {
  size_t i;
  for (i = 0; i < size; i++) {
    while (pl011_is_fifo_tx_full(uart)) {
      ;
    }
    uart->DR = (uint8_t)buf[i];
  }
  return i;
}

size_t pl011_read(pl011_uart_regs_t* uart, uint8_t* buf, size_t size) {
  size_t i;
  for (i = 0; i < size; i++) {
    while (pl011_is_fifo_rx_empty(uart)) {
      ;
    }
    buf[i] = (uint8_t)uart->DR;
  }
  return i;
}

int pl011_receive_interrupt_enable(pl011_uart_regs_t* uart) {
  uart->IMSC |= PL011_IMSC_RXIM;
  return 0;
}

int pl011_receive_interrupt_disable(pl011_uart_regs_t* uart) {
  uart->IMSC &= ~((uint32_t)PL011_IMSC_RXIM);
  return 0;
}
