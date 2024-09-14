#ifndef EVISOR_PLATFORMS_QEMU_PERIPHERAL_H_
#define EVISOR_PLATFORMS_QEMU_PERIPHERAL_H_

/****************************************************************************
 * Memory Map for QEMU aarch64
 ****************************************************************************/
// clang-format off
#define PERIPHERAL_BASE            0x0800'0000
#define UART0_BASE                 (PERIPHERAL_BASE + 0x0100'0000)

#define UART_REFERENCE_CLOCK       24000000
// clang-format on

#endif  // EVISOR_PLATFORMS_QEMU_PERIPHERAL_H_
