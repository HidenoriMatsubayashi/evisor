#ifndef EVISOR_MM_KMM_TRAP_H_
#define EVISOR_MM_KMM_TRAP_H_

#include <cstdbool>
#include <cstdint>

#include "mm/pgtable.h"

namespace evisor {

// Handle memory access trap.
bool HandleMmTrapMemoryAccessFault(va_t addr);

// Handle register access trap.
bool HandleMmTrapRegisterAccess(va_t addr, uint8_t srt, bool read);

}  // namespace evisor

#endif  // EVISOR_MM_KMM_TRAP_H_
