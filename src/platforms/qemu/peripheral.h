#ifndef EVISOR_PLATFORMS_QEMU_PERIPHERAL_H_
#define EVISOR_PLATFORMS_QEMU_PERIPHERAL_H_

/****************************************************************************
 * Memory Map for QEMU aarch64
 ****************************************************************************/
#define PERIPHERAL_BASE                   0x08000000
#define INTC_BASE                         PERIPHERAL_BASE
#define UART0_BASE                        (PERIPHERAL_BASE + 0x01000000)
#define GPIO_BASE                         (PERIPHERAL_BASE + 0x01030000)
#define EMMC2_BASE                        0xFFFFFFFF // dummy define
#define MAILBOX_BASE                      0xFFFFFFFF // dummy define

#define GIC_V2_DISTRIBUTOR_BASE           (INTC_BASE + 0x00000000)
#define GIC_V2_CPU_INTERFACE_BASE         (INTC_BASE + 0x00010000)
#define GIC_V2_HYPERVISOR_BASE            (INTC_BASE + 0x00030000)
#define GIC_V2_VIRTUAL_CPU_BASE           (INTC_BASE + 0x00040000)

#define UART_REFERENCE_CLOCK              24000000

#endif  // EVISOR_PLATFORMS_QEMU_PERIPHERAL_H_
