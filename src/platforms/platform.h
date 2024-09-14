#ifndef EVISOR_PLATFORMS_PLATFORM_H_
#define EVISOR_PLATFORMS_PLATFORM_H_

#include "platforms/serial.h"

#ifdef BOARD_IS_RASPI4
#include "drivers/gpio/gpio_bcm2711.h"
#include "platforms/bcm2711/peripheral.h"
#else
#include "platforms/qemu/peripheral.h"
#endif  // BOARD_IS_RASPI4

#endif  // EVISOR_PLATFORM_SPLATFORM_H_
