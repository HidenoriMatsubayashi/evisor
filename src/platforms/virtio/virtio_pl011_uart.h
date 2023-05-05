#ifndef EVISOR_PLATFORMS_VIRTIO_VIRTIO_PL011_UART_H_
#define EVISOR_PLATFORMS_VIRTIO_VIRTIO_PL011_UART_H_

#include <cstdint>

namespace evisor {

class VirtioPl011Uart {
 public:
  VirtioPl011Uart() = default;
  ~VirtioPl011Uart() = default;

  uint32_t Read(uint16_t addr);
  void Write(uint32_t addr, uint32_t data);

 private:
  struct Regs {
    uint32_t UARTDR;
    uint32_t UARTRSR;
    uint32_t UARTFR;
    uint32_t UARTILPR;
    uint32_t UARTIBRD;
    uint32_t UARTFBRD;
    uint32_t UARTLCR_H;
    uint32_t UARTCR;
    uint32_t UARTIFLS;
    uint32_t UARTIMSC;
    uint32_t UARTRIS;
    uint32_t UARTMIS;
    uint32_t UARTICR;
    uint32_t UARTDMACR;
  };
  Regs regs_ = {
      .UARTDR = 0,
      .UARTRSR = 0,
      .UARTFR = 0x90,
      .UARTILPR = 0,
      .UARTIBRD = 0,
      .UARTFBRD = 0,
      .UARTLCR_H = 0,
      .UARTCR = 0x300,
      .UARTIFLS = 0x12,
      .UARTIMSC = 0,
      .UARTRIS = 0,
      .UARTMIS = 0,
      .UARTICR = 0,
      .UARTDMACR = 0,
  };
};

}  // namespace evisor

#endif  // EVISOR_PLATFORMS_VIRTIO_VIRTIO_PL011_UART_H_
