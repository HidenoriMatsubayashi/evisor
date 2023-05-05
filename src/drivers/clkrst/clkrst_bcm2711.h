#ifndef EVISOR_DRIVERS_CLKRST_CLKRST_BCM2711_H_
#define EVISOR_DRIVERS_CLKRST_CLKRST_BCM2711_H_

#include "drivers/clkrst/clkrst.h"

namespace evisor {

class ClkrstBcm2711 : public Clkrst {
 public:
  ClkrstBcm2711() = default;
  ~ClkrstBcm2711() = default;

  // Prevent copying.
  ClkrstBcm2711(ClkrstBcm2711 const&) = delete;
  ClkrstBcm2711& operator=(ClkrstBcm2711 const&) = delete;

  static ClkrstBcm2711& Get() {
    static ClkrstBcm2711 instance;
    return instance;
  }

  uint32_t GetClockRate(HwBlock hw) override;
};

}  // namespace evisor

#endif  // EVISOR_DRIVERS_CLKRST_CLKRST_BCM2711_H_
