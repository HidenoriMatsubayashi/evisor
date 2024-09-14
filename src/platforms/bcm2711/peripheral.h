#ifndef EVISOR_PLATFORMS_BCM2711_PERIPHERAL_H_
#define EVISOR_PLATFORMS_BCM2711_PERIPHERAL_H_

/****************************************************************************
 * Memory Map for BCM2711 (Raspberry Pi 4)
 ****************************************************************************/
// clang-format off
#define PERIPHERAL_BASE            0xFE000000
#define GPIO_BASE                  (PERIPHERAL_BASE + 0x00200000)
#define UART0_BASE                 (PERIPHERAL_BASE + 0x00201000)

#define UART_REFERENCE_CLOCK       48000000
// clang-format on

#endif  // EVISOR_PLATFORMS_BCM2711_PERIPHERAL_H_
