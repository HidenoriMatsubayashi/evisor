#ifndef EVISOR_PLATFORMS_PLATFORM_CONFIG_H_
#define EVISOR_PLATFORMS_PLATFORM_CONFIG_H_

#ifdef BOARD_IS_RASPI4
#include "platforms/bcm2711/config.h"
#else
#include "platforms/qemu/config.h"
#endif

#define CONFIG_EARLY_SERIAL_INIT

#endif  // EVISOR_PLATFORMS_PLATFORM_CONFIG_H_
