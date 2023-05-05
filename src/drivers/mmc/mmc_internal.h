#ifndef EVISOR_DRIVERS_MMC_MMC_INTERNAL_H_
#define EVISOR_DRIVERS_MMC_MMC_INTERNAL_H_

#include "drivers/common.h"

// clang-format off
// SD Clock Frequencies [Hz]
#define SD_CLOCK_ID                                 400000
#define SD_CLOCK_NORMAL                             25000000
#define SD_CLOCK_HIGH                               50000000
#define SD_CLOCK_100                                100000000
#define SD_CLOCK_208                                208000000

#define EMMC_CTRL0_ALT_BOOT_EN                      (1 << 22)
#define EMMC_CTRL0_BOOT_EN                          (1 << 21)
#define EMMC_CTRL0_SPI_MODE                         (1 << 20)

#define EMMC_CTRL1_RESET_DATA                       (1 << 26)
#define EMMC_CTRL1_RESET_CMD                        (1 << 25)
#define EMMC_CTRL1_RESET_HOST                       (1 << 24)
#define EMMC_CTRL1_CLK_EN                           (1 << 2)
#define EMMC_CTRL1_CLK_STABLE                       (1 << 1)
#define EMMC_CTRL1_CLK_INTLEN                       (1 << 0)

#define EMMC_STATUS_DAT_INHIBIT                     (1 << 1)
#define EMMC_STATUS_CMD_INHIBIT                     (1 << 0)
// clang-format on

#endif  // EVISOR_DRIVERS_MMC_MMC_INTERNAL_H_
