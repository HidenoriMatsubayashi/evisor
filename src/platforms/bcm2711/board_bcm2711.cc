#include "platforms/bcm2711/board_bcm2711.h"

#include "arch/arm64/cpu_regs.h"
#include "common/logger.h"
#include "mm/pgtable_stage2.h"
#include "platforms/bcm2711/config.h"
#include "platforms/bcm2711/peripheral.h"
#include "platforms/board.h"
#include "platforms/platform.h"
#include "platforms/virtio/virtio_gic.h"
#include "platforms/virtio/virtio_pl011_uart.h"

namespace evisor {

void BoardBcm2711::Init(Tcb* tsk) {
  Board::Init(tsk);

  // Need to virtualize GICD registers.
  uint64_t start = GIC_V2_DISTRIBUTOR_BASE;
  uint64_t end = GIC_V2_CPU_INTERFACE_BASE;
  for (uint64_t pa = start; pa < end; pa += PAGE_SIZE) {
#if defined(TEST_GUEST_IS_NUTTX)
    uint64_t va = 0x08000000 | (pa & 0xfff);
    PgTableStage2::MapNewDevicePage(tsk, va, pa, false);
#else
    PgTableStage2::MapNewDevicePage(tsk, pa, pa, false);
#endif
  }

  // Map CPU interface to Virual CPU interface.
  start = GIC_V2_VIRTUAL_CPU_BASE;
  end = GIC_V2_VIRTUAL_CPU_BASE + 0x2000;
  for (uint64_t pa = start; pa < end; pa += PAGE_SIZE) {
#if defined(TEST_GUEST_IS_NUTTX)
    uint64_t va = (0x08010000 | (pa & 0xffff)) - 0x6000;
    PgTableStage2::MapNewDevicePage(tsk, va, pa, true);
#else
    PgTableStage2::MapNewDevicePage(tsk, pa, pa, true);
#endif
  }

  start = UART0_BASE;
  end = start + 0x1000;
  for (uint64_t pa = start; pa < end; pa += PAGE_SIZE) {
#if defined(TEST_GUEST_IS_NUTTX)
    uint64_t va = 0x09000000 | (pa & 0x00000fff);
    PgTableStage2::MapNewDevicePage(tsk, va, pa, false);
#else
    PgTableStage2::MapNewDevicePage(tsk, pa, pa, false);
#endif
  }
}

uint64_t BoardBcm2711::MmioRead(Tcb* tsk, uint64_t addr) {
  UNUSED(tsk);

  const uint32_t base_addr = addr & 0xFFFFF000;

#if defined(TEST_GUEST_IS_NUTTX)
  if (base_addr == 0x09000000) {
    return virtio_uart_.Read(addr & 0xFFF);
  } else if (base_addr == 0x08000000) {
    return virtio_gic_.Read(addr & 0xFFF);
  } else {
    LOG_ERROR("Unexpected read: addr = %08x", addr);
  }
#else
  if (base_addr == UART0_BASE) {
    return virtio_uart_.Read(addr & 0xFFF);
  } else if (base_addr == GIC_V2_DISTRIBUTOR_BASE) {
    return virtio_gic_.Read(addr & 0xFFF);
  } else {
    LOG_ERROR("Unexpected read: addr = %08x", addr);
  }
#endif

  return 0;
}

void BoardBcm2711::MmioWrite(Tcb* tsk, uint64_t addr, uint64_t val) {
  UNUSED(tsk);

  const uint32_t base_addr = addr & 0xFFFFF000;

#if defined(TEST_GUEST_IS_NUTTX)
  if (base_addr == 0x09000000) {
    virtio_uart_.Write(addr & 0xFFF, val);
  } else if (base_addr == 0x08000000) {
    virtio_gic_.Write(addr & 0xFFF, val);
  } else {
    PANIC("Unexpected write: addr = %08x, data = %08x", addr, val);
  }
#else
  if (base_addr == UART0_BASE) {
    virtio_uart_.Write(addr & 0xFFF, val);
  } else if (base_addr == GIC_V2_DISTRIBUTOR_BASE) {
    virtio_gic_.Write(addr & 0xFFF, val);
  } else {
    PANIC("Unexpected write: addr = %08x, data = %08x", addr, val);
  }
#endif
}

}  // namespace evisor
