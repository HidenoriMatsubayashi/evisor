#ifndef EVISOR_PLATFORMS_COMMON_PL011_UART_H_
#define EVISOR_PLATFORMS_COMMON_PL011_UART_H_

#include <cstdbool>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <array>

namespace evisor {

class Pl011Uart {
 public:
  using IrqHandler = std::function<void(uint8_t c)>;

  enum Irq {
    RECEIVE = (1 << 4),
    TRANSMIT = (1 << 5),
    RECEIVE_TIMEOUT = (1 << 6),
    FRAMING_ERROR = (1 << 7),
    PARITY_ERROR = (1 << 8),
    BREAK_ERROR = (1 << 9),
    OVERRUN_ERROR = (1 << 10),
  };

  Pl011Uart() = default;
  ~Pl011Uart() = default;

  void Init(uint64_t base_addr);
  void Enable(uint64_t uart_clock, uint64_t baudrate);
  void Disable();
  void EnableReceiveInterrupt(IrqHandler handler);
  void DisableReceiveInterrupt();
  size_t Write(uint8_t* buf, size_t size);
  size_t Read(uint8_t* buf, size_t size);
  inline bool IsFifoTxEmpty();
  inline bool IsFifoRxEmpty();
  inline bool IsFifoTxFull();
  inline bool IsFifoRxFull();
  inline bool IsBusy();

 private:
  void BaudrateSetup(uint64_t uart_clock, uint64_t baudrate);

  struct Regs {
    volatile uint32_t DR;
    volatile uint32_t RSRECR;
    volatile uint8_t __RESERVED_0[0x18 - 0x08];
    volatile uint32_t FR;
    volatile uint8_t __RESERVED_1[0x20 - 0x1c];
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
    volatile uint8_t __RESERVED_2[0x80 - 0x4c];
    volatile uint32_t ITCR;
    volatile uint32_t ITIP;
    volatile uint32_t ITOP;
    volatile uint32_t TDR;
  } __attribute__((packed)) __attribute__((aligned(4)));

  volatile Regs* regs_;
  IrqHandler irq_handler_;
};

}  // namespace evisor

#endif  // EVISOR_PLATFORMS_COMMON_PL011_UART_H_
