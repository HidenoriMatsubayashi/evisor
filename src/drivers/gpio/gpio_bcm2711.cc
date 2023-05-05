#include "drivers/gpio/gpio_bcm2711.h"

#include "common/logger.h"
#include "platforms/bcm2711/peripheral.h"

namespace evisor {

namespace {
constexpr uint8_t kMaxPortNum = 57;
}

GpioBcm2711::GpioBcm2711() {
  regs_ = reinterpret_cast<Regs*>(GPIO_BASE);
}

bool GpioBcm2711::SetMode(uint32_t port, GpioMode mode) {
  if (port > kMaxPortNum) {
    LOG_ERROR("GPIO port number must be %d or less (%d)", kMaxPortNum, port);
    return false;
  }

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
  uint32_t function_mode = 0;
  switch (mode) {
    case GpioMode::kInput:
      function_mode = 0b000;
      break;
    case GpioMode::kOutput:
      function_mode = 0b001;
      break;
    case GpioMode::kAlt0:
      function_mode = 0b100;
      break;
    case GpioMode::kAlt1:
      function_mode = 0b101;
      break;
    case GpioMode::kAlt2:
      function_mode = 0b110;
      break;
    case GpioMode::kAlt3:
      function_mode = 0b111;
      break;
    case GpioMode::kAlt4:
      function_mode = 0b011;
      break;
    case GpioMode::kAlt5:
      function_mode = 0b010;
      break;
    default:
      break;
  }
  const uint32_t fselx = port / 10;
  const uint32_t fselx_offset = port % 10;
  uint32_t gpfsel = regs_->GPFSEL[fselx];
  {
    gpfsel &= ~(static_cast<uint32_t>(0x7 << (fselx_offset * 3)));
    gpfsel |= static_cast<uint32_t>(function_mode << (fselx_offset * 3));
    regs_->GPFSEL[fselx] = gpfsel;
  }

  return true;
}

bool GpioBcm2711::Output(uint32_t port, bool out) {
  if (port > kMaxPortNum) {
    LOG_ERROR("GPIO port number must be %d or less (%d)", kMaxPortNum, port);
    return false;
  }

  // GPSETx and GPCLRx are WO type registers
  const uint32_t reg_num = port / 32;
  const uint32_t bit_offset = port % 32;
  if (out) {
    regs_->GPSET[reg_num] = 1 << (bit_offset);
  } else {
    regs_->GPCLR[reg_num] = 1 << (bit_offset);
  }

  return true;
}

}  // namespace evisor
