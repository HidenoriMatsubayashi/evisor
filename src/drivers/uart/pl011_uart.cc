#include "drivers/uart/pl011_uart.h"

#include "arch/arm64/irq/gic_v2.h"
#include "common/macro.h"
#include "platforms/platform.h"

#ifdef BOARD_IS_RASPI4
#include "platforms/bcm2711/peripheral.h"
#else
#include "platforms/qemu/peripheral.h"
#endif

namespace evisor {

namespace {

/*
 * FR Register
 */
// Transmit FIFO empty.
constexpr uint32_t kFrTxfe = BIT32(7);
// Receive FIFO full.
constexpr uint32_t kFrRxff = BIT32(6);
// Transmit FIFO full.
constexpr uint32_t kFrTxff = BIT32(5);
// Receive FIFO empty.
constexpr uint32_t kFrRxfe = BIT32(4);
// UART busy.
constexpr uint32_t kFrBusy = BIT32(3);

/*
 * LCRH Register
 */
// Word length.
constexpr uint32_t kLcrhWlen_8bits = 0b11 << 5;

/*
 * CR Register
 */
// Receive enable.
constexpr uint32_t kCrRxe = BIT32(9);
// Transmit enable.
constexpr uint32_t kCrTxe = BIT32(8);
// UART enable.
constexpr uint32_t kCrUarten = BIT32(0);

/*
 * IMSC Register
 */
// Receive interrupt mask.
constexpr uint32_t kImscRxim = BIT32(4);
}  // namespace

void Pl011Uart::Init(uint64_t base_addr) {
  regs_ = reinterpret_cast<Regs*>(base_addr);

#ifdef BOARD_IS_RASPI4
  // Pin assignment: GPIO14 to TXD0, GPIO15 to RXD0.
  auto& gpio = evisor::GpioBcm2711::Get();
  gpio.SetMode(14, evisor::Gpio::GpioMode::kAlt0);
  gpio.SetMode(15, evisor::Gpio::GpioMode::kAlt0);
#endif
}

void Pl011Uart::Enable(uint64_t base_clock, uint64_t baudrate) {
  SetBaudrate(base_clock, baudrate);
  regs_->LCRH = kLcrhWlen_8bits;
  regs_->CR = kCrUarten | kCrTxe | kCrRxe;
}

void Pl011Uart::Disable() {
  regs_->CR = 0;
}

void Pl011Uart::EnableReceiveIrq(IrqHandler handler) {
  irq_handler_ = handler;

  uint8_t irq_id;
#ifdef BOARD_IS_RASPI4
  irq_id = 121 + 32;
#else
  // UART1 in QEMU
  irq_id = 33;
#endif

  GicV2::Get().RegisterIrq(irq_id, 0, 0x7f, [this]() {
    if (regs_->MIS & kImscRxim) {
      while (IsBusy()) {
        ;
      }
      while (!IsFifoRxEmpty()) {
        uint8_t c = 0;
        Read(&c, 1);
        irq_handler_(c);
      }
    }
    regs_->ICR = kImscRxim;
  });
  regs_->IMSC = regs_->IMSC | kImscRxim;
}

void Pl011Uart::DisableReceiveIrq() {
  regs_->IMSC = regs_->IMSC & (~kImscRxim);
  irq_handler_ = nullptr;
}

size_t Pl011Uart::Write(uint8_t* buf, size_t size) {
  size_t writes = 0;
  for (; writes < size; writes++) {
    while (IsFifoTxFull()) {
      ;
    }
    regs_->DR = buf[writes];
  }
  return writes;
}

size_t Pl011Uart::Read(uint8_t* buf, size_t size) {
  size_t reads = 0;
  for (; reads < size; reads++) {
    while (IsFifoRxEmpty()) {
      ;
    }
    buf[reads] = regs_->DR;
  }
  return reads;
}

void Pl011Uart::SetBaudrate(uint64_t base_clock, uint64_t baudrate) {
  const uint32_t bauddiv = (1000 * base_clock) / (16 * baudrate);
  const uint32_t ibrd = bauddiv / 1000;
  const uint32_t fbrd = ((bauddiv - ibrd * 1000) * 64 + 500) / 1000;
  regs_->IBRD = ibrd;
  regs_->FBRD = fbrd;
}

inline bool Pl011Uart::IsFifoTxEmpty() {
  return (regs_->FR & kFrTxfe) != 0;
}

inline bool Pl011Uart::IsFifoRxEmpty() {
  return (regs_->FR & kFrRxfe) != 0;
}

inline bool Pl011Uart::IsFifoTxFull() {
  return (regs_->FR & kFrTxff) != 0;
}

inline bool Pl011Uart::IsFifoRxFull() {
  return (regs_->FR & kFrRxff) != 0;
}

inline bool Pl011Uart::IsBusy() {
  return (regs_->FR & kFrBusy) != 0;
}

}  // namespace evisor
