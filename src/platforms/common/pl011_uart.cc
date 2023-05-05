#include "platforms/common/pl011_uart.h"

#include "arch/arm64/irq/gic_v2.h"
#include "platforms/platform.h"

#ifdef BOARD_IS_RASPI4
#include "platforms/bcm2711/peripheral.h"
#else
#include "platforms/qemu/peripheral.h"
#endif

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

namespace evisor {

void Pl011Uart::Init(uint64_t base_addr) {
  regs_ = reinterpret_cast<Regs*>(base_addr);

#ifdef BOARD_IS_RASPI4
  // Pin assignment: GPIO14 to TXD0, GPIO15 to RXD0.
  auto& gpio = evisor::GpioBcm2711::Get();
  gpio.SetMode(14, evisor::Gpio::GpioMode::kAlt0);
  gpio.SetMode(15, evisor::Gpio::GpioMode::kAlt0);
#endif
}

void Pl011Uart::Enable(uint64_t uart_clock, uint64_t baudrate) {
  Disable();
  BaudrateSetup(uart_clock, baudrate);

  // 8bit, FIFO
  regs_->LCRH = PL011_LCRH_WLEN_8;
  // uart enable, TX/RX enable
  regs_->CR = PL011_CR_UARTEN | PL011_CR_TXE | PL011_CR_RXE;
}

void Pl011Uart::Disable() {
  // Clear UART setting
  regs_->CR = 0;

  // disable FIFO
  regs_->LCRH = 0;
}

void Pl011Uart::EnableReceiveInterrupt(IrqHandler handler) {
  irq_handler_ = handler;

  uint8_t irq_id;
#ifdef BOARD_IS_RASPI4
  irq_id = 121 + 32;
#else
  irq_id = 33;  // UART1 in QEMU
#endif

  GicV2::Get().RegisterIrq(irq_id, 0, 0x7f, [this]() {
    // echo back reveice charactor
    if (regs_->MIS & Irq::RECEIVE) {
      while (IsBusy()) {
        ;
      }
      while (!IsFifoRxEmpty()) {
        uint8_t c = 0;
        Read(&c, 1);
        irq_handler_(c);
      }
    }

    // clear interrupt flag
    regs_->ICR = RECEIVE;
  });
  regs_->IMSC = regs_->IMSC | PL011_IMSC_RXIM;
}

void Pl011Uart::DisableReceiveInterrupt() {
  regs_->IMSC = regs_->IMSC & (~((uint32_t)PL011_IMSC_RXIM));
  irq_handler_ = nullptr;
}

size_t Pl011Uart::Write(uint8_t* buf, size_t size) {
  size_t i;
  for (i = 0; i < size; i++) {
    while (IsFifoTxFull()) {
      ;
    }
    regs_->DR = (uint8_t)buf[i];
  }
  return i;
}

size_t Pl011Uart::Read(uint8_t* buf, size_t size) {
  size_t i;
  for (i = 0; i < size; i++) {
    while (IsFifoRxEmpty()) {
      ;
    }
    buf[i] = (uint8_t)regs_->DR;
  }
  return i;
}

inline bool Pl011Uart::IsFifoTxEmpty() {
  if ((regs_->FR & PL011_FR_TXFE) != 0) {
    return true;
  }
  return false;
}

inline bool Pl011Uart::IsFifoRxEmpty() {
  if ((regs_->FR & PL011_FR_RXFE) != 0) {
    return true;
  }
  return false;
}

inline bool Pl011Uart::IsFifoTxFull() {
  if ((regs_->FR & PL011_FR_TXFF) != 0) {
    return true;
  }
  return false;
}

inline bool Pl011Uart::IsFifoRxFull() {
  if ((regs_->FR & PL011_FR_RXFF) != 0) {
    return true;
  }
  return false;
}

inline bool Pl011Uart::IsBusy() {
  if ((regs_->FR & PL011_FR_BUSY) != 0) {
    return true;
  }
  return false;
}

void Pl011Uart::BaudrateSetup(uint64_t uart_clock, uint64_t baudrate) {
  const uint32_t bauddiv = (1000 * uart_clock) / (16 * baudrate);
  const uint32_t ibrd = bauddiv / 1000;
  const uint32_t fbrd = ((bauddiv - ibrd * 1000) * 64 + 500) / 1000;
  regs_->IBRD = ibrd;
  regs_->FBRD = fbrd;
}

}  // namespace evisor
