#ifndef BROWNIE_PLATFORMS_COMMON_PL011_UART_H_
#define BROWNIE_PLATFORMS_COMMON_PL011_UART_H_

/****************************************************************************
 * Included Files
 ****************************************************************************/
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef BOARD_IS_RASPI4
#define PERIPHERAL_BASE            0xFE000000
#define UART0_BASE                 (PERIPHERAL_BASE + 0x00201000)
#define UART_REFERENCE_CLOCK       48000000
#else
#define PERIPHERAL_BASE            0x08000000
#define UART0_BASE                 (PERIPHERAL_BASE + 0x01000000)
#define UART_REFERENCE_CLOCK       24000000
#endif

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/
#define PL011_FR_RI (1 << 8)
#define PL011_FR_TXFE (1 << 7)
#define PL011_FR_RXFF (1 << 6)
#define PL011_FR_TXFF (1 << 5)
#define PL011_FR_RXFE (1 << 4)
#define PL011_FR_BUSY (1 << 3)
#define PL011_FR_DCD (1 << 2)
#define PL011_FR_DSR (1 << 1)
#define PL011_FR_CTS (1 << 0)

#define PL011_LCRH_SPS (1 << 7)
#define PL011_LCRH_WLEN_8 (3 << 5)
#define PL011_LCRH_WLEN_7 (2 << 5)
#define PL011_LCRH_WLEN_6 (1 << 5)
#define PL011_LCRH_WLEN_5 (0 << 5)
#define PL011_LCRH_FEN (1 << 4)
#define PL011_LCRH_STP2 (1 << 3)
#define PL011_LCRH_EPS (1 << 2)
#define PL011_LCRH_PEN (1 << 1)
#define PL011_LCRH_BRK (1 << 0)

#define PL011_CR_CTSEN (1 << 15)
#define PL011_CR_RTSEN (1 << 14)
#define PL011_CR_RTS (1 << 11)
#define PL011_CR_DTR (1 << 10)
#define PL011_CR_RXE (1 << 9)
#define PL011_CR_TXE (1 << 8)
#define PL011_CR_LBE (1 << 7)
#define PL011_CR_SIRLP (1 << 2)
#define PL011_CR_SIREN (1 << 1)
#define PL011_CR_UARTEN (1 << 0)

#define PL011_IMSC_TXIM (1 << 5)
#define PL011_IMSC_RXIM (1 << 4)

/****************************************************************************
 * Public Types
 ****************************************************************************/
typedef volatile struct {
  volatile uint32_t DR;
  volatile uint32_t RSRECR;
  volatile uint8_t RESERVED0[0x18 - 0x08];
  volatile uint32_t FR;
  volatile uint8_t RESERVED1[0x20 - 0x1c];
  volatile uint32_t ILPR;
  volatile uint32_t IBRD;
  volatile uint32_t FBRD;
  volatile uint32_t LCRH;
  volatile uint32_t CR;
  volatile uint32_t IFLS;
  volatile uint32_t IMSC;
  volatile uint32_t RIS;
  volatile uint32_t MIS;
  volatile uint32_t ICR;
  volatile uint32_t DMACR;
  volatile uint8_t RESERVED2[0x80 - 0x4c];
  volatile uint32_t ITCR;
  volatile uint32_t ITIP;
  volatile uint32_t ITOP;
  volatile uint32_t TDR;
} __attribute__((packed)) __attribute__((aligned(4))) pl011_uart_regs_t;

typedef enum {
  RECEIVE = (1 << 4),
  TRANSMIT = (1 << 5),
  RECEIVE_TIMEOUT = (1 << 6),
  FRAMING_ERROR = (1 << 7),
  PARITY_ERROR = (1 << 8),
  BREAK_ERROR = (1 << 9),
  OVERRUN_ERROR = (1 << 10),
} pl011_uart_irq_t;

/****************************************************************************
 * Public Global Variables
 ****************************************************************************/
static inline bool pl011_is_fifo_tx_empty(pl011_uart_regs_t* uart) {
  if ((uart->FR & PL011_FR_TXFE) != 0) {
    return true;
  }
  return false;
}

static inline bool pl011_is_fifo_rx_empty(pl011_uart_regs_t* uart) {
  if ((uart->FR & PL011_FR_RXFE) != 0) {
    return true;
  }
  return false;
}

static inline bool pl011_is_fifo_tx_full(pl011_uart_regs_t* uart) {
  if ((uart->FR & PL011_FR_TXFF) != 0) {
    return true;
  }
  return false;
}

static inline bool pl011_is_fifo_rx_full(pl011_uart_regs_t* uart) {
  if ((uart->FR & PL011_FR_RXFF) != 0) {
    return true;
  }
  return false;
}

static inline bool pl011_is_busy(pl011_uart_regs_t* uart) {
  if ((uart->FR & PL011_FR_BUSY) != 0) {
    return true;
  }
  return false;
}

/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/
void pl011_uart_enable(pl011_uart_regs_t* uart, uint64_t uart_clock,
                       uint64_t baudrate);
void pl011_uart_disable(pl011_uart_regs_t* uart);
size_t pl011_write(pl011_uart_regs_t* uart, uint8_t* buf, size_t size);
size_t pl011_read(pl011_uart_regs_t* uart, uint8_t* buf, size_t size);
int pl011_receive_interrupt_enable(pl011_uart_regs_t* uart);
int pl011_receive_interrupt_disable(pl011_uart_regs_t* uart);

extern inline bool pl011_is_fifo_tx_empty(pl011_uart_regs_t* uart);
extern inline bool pl011_is_fifo_rx_empty(pl011_uart_regs_t* uart);
extern inline bool pl011_is_fifo_tx_full(pl011_uart_regs_t* uart);
extern inline bool pl011_is_fifo_rx_full(pl011_uart_regs_t* uart);
extern inline bool pl011_is_busy(pl011_uart_regs_t* uart);

#endif  // BROWNIE_PLATFORMS_COMMON_PL011_UART_H_
