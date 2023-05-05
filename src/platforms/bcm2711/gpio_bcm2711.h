#ifndef EVISOR_PLATFORMS_BCM2711_GPIO_BCM2711_H_
#define EVISOR_PLATFORMS_BCM2711_GPIO_BCM2711_H_

#include "platforms/gpio.h"

namespace evisor {

class GpioBcm2711 : public Gpio {
 public:
  GpioBcm2711();
  ~GpioBcm2711() = default;

  // Prevent copying.
  GpioBcm2711(GpioBcm2711 const&) = delete;
  GpioBcm2711& operator=(GpioBcm2711 const&) = delete;

  static GpioBcm2711& Get() noexcept {
    static GpioBcm2711 instance;
    return instance;
  }

  bool SetMode(uint32_t port, GpioMode mode) override;
  bool Output(uint32_t port, bool out) override;

 private:
  struct GpioRegs {
    volatile uint32_t GPFSEL[6];
    volatile uint32_t __RESERVED_0;
    volatile uint32_t GPSET[2];
    volatile uint32_t __RESERVED_1;
    volatile uint32_t GPCLR[2];
    volatile uint32_t __RESERVED_2;
    volatile uint32_t GPLEV[2];
    volatile uint32_t __RESERVED_3;
    volatile uint32_t GPEDS[2];
    volatile uint32_t __RESERVED_4;
    volatile uint32_t GPREN[2];
    volatile uint32_t __RESERVED_5;
    volatile uint32_t GPFEN[2];
    volatile uint32_t __RESERVED_6;
    volatile uint32_t GPHEN[2];
    volatile uint32_t __RESERVED_7;
    volatile uint32_t GPLEN[2];
    volatile uint32_t __RESERVED_8;
    volatile uint32_t GPAREN[2];
    volatile uint32_t __RESERVED_9;
    volatile uint32_t GPAFEN[2];
  } __attribute__((packed)) __attribute__((aligned(4)));

  bool CheckPort(uint32_t port);

  volatile GpioRegs* regs_;
};

}  // namespace evisor

#endif  // EVISOR_PLATFORMS_BCM2711_GPIO_BCM2711_H_
