#ifndef EVISOR_PLATFORMS_PLATFORM_QEMU_CONFIG_H_
#define EVISOR_PLATFORMS_PLATFORM_QEMU_CONFIG_H_

#include "common/macro.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/
#define CONFIG_DEVICEIO_BASEADDR  0x08000000
#define CONFIG_DEVICEIO_SIZE      MB(512)

//#define CONFIG_MMU_DEBUG

#endif  // EVISOR_PLATFORMS_PLATFORM_QEMU_CONFIG_H_
