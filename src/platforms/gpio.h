#ifndef EVISOR_PLATFORMS_GPIO_H_
#define EVISOR_PLATFORMS_GPIO_H_

#include <cstdbool>
#include <cstdint>

namespace evisor {

class Gpio {
 public:
  enum GpioMode {
    kInput = 0b000,
    kOutput = 0b001,
    kAlt0 = 0b100,
    kAlt1 = 0b101,
    kAlt2 = 0b110,
    kAlt3 = 0b111,
    kAlt4 = 0b011,
    kAlt5 = 0b010
  };

  Gpio() = default;
  ~Gpio() = default;

  // Prevent copying.
  Gpio(Gpio const&) = delete;
  Gpio& operator=(Gpio const&) = delete;

  virtual bool SetMode(uint32_t port, GpioMode mode) = 0;
  virtual bool Output(uint32_t port, bool out) = 0;
};

}  // namespace evisor

#endif  // EVISOR_PLATFORMS_GPIO_H_
