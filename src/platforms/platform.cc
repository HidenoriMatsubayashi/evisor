#include "platforms/platform.h"

#if defined(BOARD_IS_RASPI4)
#include "platforms/bcm2711/board_bcm2711.h"
#else
#include "platforms/qemu/board_qemu.h"
#endif

namespace evisor {

Board& GetPlatformBoard() {
#ifdef BOARD_IS_RASPI4
  return evisor::BoardBcm2711::Get();
#else
  return evisor::BoardQemu::Get();
#endif
}

}  // namespace evisor
