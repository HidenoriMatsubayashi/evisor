#include "platforms/bcm2711/gpio_bcm2711.h"

#include "common/logger.h"
#include "platforms/bcm2711/peripheral.h"

namespace evisor {

namespace {
constexpr uint8_t kMaxPortNum = 57;
}

GpioBcm2711::GpioBcm2711() { regs_ = reinterpret_cast<GpioRegs*>(GPIO_BASE); }

bool GpioBcm2711::SetMode(uint32_t port, GpioMode mode) {
  if (!CheckPort(port)) {
    LOG_ERROR("GPIO port number must be %d or less (%d)", kMaxPortNum, port);
    return false;
  }

  const uint32_t fselx = port / 10;
  const uint32_t fselx_offset = port % 10;

  /* FSELx register/field:
   * 000 = GPIO Pin x is an input
   * 001 = GPIO Pin x is an output
   * 100 = GPIO Pin x takes alternate function 0
   * 101 = GPIO Pin x takes alternate function 1
   * 110 = GPIO Pin x takes alternate function 2
   * 111 = GPIO Pin x takes alternate function 3
   * 011 = GPIO Pin x takes alternate function 4
   * 010 = GPIO Pin x takes alternate function 5
   */
  uint32_t gpfsel = regs_->GPFSEL[fselx];
  gpfsel &= ~(static_cast<uint32_t>(0x7 << (fselx_offset * 3)));
  gpfsel |= static_cast<uint32_t>(mode << (fselx_offset * 3));
  regs_->GPFSEL[fselx] = gpfsel;

  return true;
}

bool GpioBcm2711::Output(uint32_t port, bool out) {
  if (!CheckPort(port)) {
    return false;
  }

  volatile uint32_t* reg;
  if (out) {
    reg = regs_->GPSET;
  } else {
    reg = regs_->GPCLR;
  }

  uint32_t num = port / 32;
  reg[num] = 1 << (port % 32);  // TODO: check if it's correct.

  return true;
}

bool GpioBcm2711::CheckPort(uint32_t port) { return (port <= kMaxPortNum); }

}  // namespace evisor
