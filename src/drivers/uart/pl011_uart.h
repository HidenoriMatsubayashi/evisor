#ifndef EVISOR_DRIVERS_UART_PL011_UART_H_
#define EVISOR_DRIVERS_UART_PL011_UART_H_

#include <cstdbool>
#include <cstdint>
#include <functional>

#include "drivers/common.h"

namespace evisor {

class Pl011Uart {
 public:
  using IrqHandler = std::function<void(uint8_t c)>;

  Pl011Uart() = default;
  ~Pl011Uart() = default;

  void Init(uint64_t base_addr);

  void Enable(uint64_t base_clock, uint64_t baudrate);
  void Disable();

  void EnableReceiveIrq(IrqHandler handler);
  void DisableReceiveIrq();

  size_t Write(uint8_t* buf, size_t size);
  size_t Read(uint8_t* buf, size_t size);

 private:
  void SetBaudrate(uint64_t base_clock, uint64_t baudrate);
  inline bool IsFifoTxEmpty();
  inline bool IsFifoRxEmpty();
  inline bool IsFifoTxFull();
  inline bool IsFifoRxFull();
  inline bool IsBusy();

  struct Regs {
    reg32_t DR;
    reg32_t RSRECR;
    reg32_t __RESERVED_0[4];
    reg32_t FR;
    reg32_t __RESERVED_1[1];
    reg32_t ILPR;
    reg32_t IBRD;
    reg32_t FBRD;
    reg32_t LCRH;
    reg32_t CR;
    reg32_t IFLS;
    reg32_t IMSC;
    reg32_t RIS;
    reg32_t MIS;
    reg32_t ICR;
    reg32_t DMACR;
    reg32_t __RESERVED_2[13];
    reg32_t ITCR;
    reg32_t ITIP;
    reg32_t ITOP;
    reg32_t TDR;
  } __attribute__((packed)) __attribute__((aligned(4)));

  volatile Regs* regs_;
  IrqHandler irq_handler_;
};

}  // namespace evisor

#endif  // EVISOR_DRIVERS_UART_PL011_UART_H_
