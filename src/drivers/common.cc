#include "drivers/common.h"

#include "platforms/timer.h"

namespace evisor {

bool RegWaitBitSet(reg32_t* reg, uint32_t mask, uint32_t timeout_ms) {
  const auto timeout_usec = timeout_ms * 1000;
  for (uint32_t i = 0; i <= timeout_usec; i++) {
    if ((*reg & mask) == mask) {
      return true;
    }
    Timer::SleepUsec(1);
  }
  return false;
}

bool RegWaitBitClear(reg32_t* reg, uint32_t mask, uint32_t timeout_ms) {
  const auto timeout_usec = timeout_ms * 1000;
  for (uint32_t i = 0; i <= timeout_usec; i++) {
    if ((*reg & mask) == 0) {
      return true;
    }
    Timer::SleepUsec(1);
  }
  return false;
}

}  // namespace evisor
