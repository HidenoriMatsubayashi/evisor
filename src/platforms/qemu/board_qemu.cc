#include "platforms/qemu/board_qemu.h"

#include <cstdint>

#include "arch/arm64/cpu_regs.h"
#include "common/logger.h"
#include "mm/pgtable_stage2.h"
#include "platforms/platform.h"
#include "platforms/qemu/config.h"
#include "platforms/qemu/peripheral.h"
#include "platforms/virtio/virtio_gic.h"
#include "platforms/virtio/virtio_pl011_uart.h"

namespace evisor {

void BoardQemu::Init(Tcb* tsk) {
  Board::Init(tsk);

  // Need to virtualize GICD registers.
  uint64_t start = GIC_V2_DISTRIBUTOR_BASE;
  uint64_t end = GIC_V2_CPU_INTERFACE_BASE;
  for (uint64_t pa = start; pa < end; pa += PAGE_SIZE) {
    PgTableStage2::MapNewDevicePage(tsk, pa, pa, false);
  }

  // Map CPU interface to Virual CPU interface.
  start = GIC_V2_VIRTUAL_CPU_BASE;
  end = GIC_V2_VIRTUAL_CPU_BASE + 0x2000;
  for (uint64_t pa = start; pa < end; pa += PAGE_SIZE) {
    uint64_t va = pa - 0x30000;
    PgTableStage2::MapNewDevicePage(tsk, va, pa, true);
  }

  start = UART0_BASE;
  end = start + 0x1000;
  for (uint64_t pa = start; pa < end; pa += PAGE_SIZE) {
    PgTableStage2::MapNewDevicePage(tsk, pa, pa, false);
  }
}

uint64_t BoardQemu::MmioRead(Tcb* tsk, uint64_t addr) {
  UNUSED(tsk);

  const uint32_t base_addr = addr & 0xFFFFF000;
  if (base_addr == UART0_BASE) {
    return virtio_uart_.Read(addr & 0xFFF);
  } else if (base_addr == GIC_V2_DISTRIBUTOR_BASE) {
    return virtio_gic_.Read(addr & 0xFFF);
  } else {
    LOG_ERROR("Unexpected read: addr = %08x", addr);
  }
  return 0;
}

void BoardQemu::MmioWrite(Tcb* tsk, uint64_t addr, uint64_t val) {
  UNUSED(tsk);

  const uint32_t base_addr = addr & 0xFFFFF000;
  if (base_addr == UART0_BASE) {
    virtio_uart_.Write(addr & 0xFFF, val);
  } else if (base_addr == GIC_V2_DISTRIBUTOR_BASE) {
    virtio_gic_.Write(addr & 0xFFF, val);
  } else {
    LOG_ERROR("Unexpected write: addr = %08x, data = %08x", addr, val);
  }
}

}  // namespace evisor
