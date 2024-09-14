#ifndef EVISOR_DRIVERS_GPIO_GPIO_BCM2711_H_
#define EVISOR_DRIVERS_GPIO_GPIO_BCM2711_H_

#include "drivers/common.h"
#include "drivers/gpio/gpio.h"

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
  struct Regs {
    reg32_t GPFSEL[6];  // GPFSEL0 - GPFSEL5
    reg32_t __RESERVED_0;
    reg32_t GPSET[2];  // GPSET0 - GPSET1
    reg32_t __RESERVED_1;
    reg32_t GPCLR[2];  // GPCLR0 - GPCLR1
    reg32_t __RESERVED_2;
    reg32_t GPLEV[2];  // GPLEV0 - GPLEV1
    reg32_t __RESERVED_3;
    reg32_t GPEDS[2];  // GPEDS0 - GPEDS1
    reg32_t __RESERVED_4;
    reg32_t GPREN[2];  // GPREN0 - GPREN1
    reg32_t __RESERVED_5;
    reg32_t GPFEN[2];  // GPFEN0 - GPFEN1
    reg32_t __RESERVED_6;
    reg32_t GPHEN[2];  // GPHEN0 - GPHEN1
    reg32_t __RESERVED_7;
    reg32_t GPLEN[2];  // GPLEN0 - GPLEN1
    reg32_t __RESERVED_8;
    reg32_t GPAREN[2];  // GPAREN0 - GPAREN1
    reg32_t __RESERVED_9;
    reg32_t GPAFEN[2];  // GPAFEN0 - GPAFEN1
  } __attribute__((packed)) __attribute__((aligned(4)));

  volatile Regs* regs_;
};

}  // namespace evisor

#endif  // EVISOR_DRIVERS_GPIO_GPIO_BCM2711_H_
