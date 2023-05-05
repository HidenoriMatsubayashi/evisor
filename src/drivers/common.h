#ifndef EVISOR_DRIVERS_COMMON_H_
#define EVISOR_DRIVERS_COMMON_H_

#include <cstdbool>
#include <cstdint>

typedef volatile uint32_t reg32_t;

namespace evisor {

bool RegWaitBitSet(reg32_t* reg, uint32_t mask, uint32_t timeout_ms);
bool RegWaitBitClear(reg32_t* reg, uint32_t mask, uint32_t timeout_ms);

}  // namespace evisor

#endif  // EVISOR_DRIVERS_COMMON_H_
