#ifndef EVISOR_DRIVERS_CLKRST_CLKRST_H_
#define EVISOR_DRIVERS_CLKRST_CLKRST_H_

#include <cstdint>

namespace evisor {

class Clkrst {
 public:
  enum class HwBlock { CPU, GPU, DRAM, EMMC };

  Clkrst() = default;
  ~Clkrst() = default;

  virtual uint32_t GetClockRate(HwBlock hw) = 0;
};

}  // namespace evisor

#endif  // EVISOR_DRIVERS_CLKRST_CLKRST_H_
