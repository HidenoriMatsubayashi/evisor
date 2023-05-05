#ifndef EVISOR_DRIVERS_GPIO_H_
#define EVISOR_DRIVERS_GPIO_H_

#include <cstdbool>
#include <cstdint>

namespace evisor {

class Gpio {
 public:
  enum class GpioMode {
    kInput,
    kOutput,
    kAlt0,
    kAlt1,
    kAlt2,
    kAlt3,
    kAlt4,
    kAlt5,
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

#endif  // EVISOR_DRIVERS_GPIO_H_
