#ifndef EVISOR_PLATFORMS_BCM2711_PERIPHERAL_H_
#define EVISOR_PLATFORMS_BCM2711_PERIPHERAL_H_

/****************************************************************************
 * Memory Map for BCM2711 (Raspberry Pi 4)
 ****************************************************************************/
// clang-format off
#define PERIPHERAL_BASE            0xFE000000
#define SYSTEM_TIMER_BASE          (PERIPHERAL_BASE + 0x00003000)
#define DMA_BASE                   (PERIPHERAL_BASE + 0x00007000)
#define MAILBOX_BASE               (PERIPHERAL_BASE + 0x0000b880)
#define GPIO_BASE                  (PERIPHERAL_BASE + 0x00200000)
#define UART0_BASE                 (PERIPHERAL_BASE + 0x00201000)
#define EMMC2_BASE                 (PERIPHERAL_BASE + 0x00340000)
#define INTC_BASE                  0xFF800000
#define VIRTIO_BASE                0xFFFF'FFFF  // dummy define

#define GIC_V2_DISTRIBUTOR_BASE    (INTC_BASE + 0x00041000)
#define GIC_V2_CPU_INTERFACE_BASE  (INTC_BASE + 0x00042000)
#define GIC_V2_HYPERVISOR_BASE     (INTC_BASE + 0x00044000)
#define GIC_V2_VIRTUAL_CPU_BASE    (INTC_BASE + 0x00046000)

#define UART_REFERENCE_CLOCK       48000000
// clang-format on

#endif  // EVISOR_PLATFORMS_BCM2711_PERIPHERAL_H_
