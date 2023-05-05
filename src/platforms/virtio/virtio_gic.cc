#include "platforms/virtio/virtio_gic.h"

#include "arch/arm64/irq/gic.h"
#include "common/logger.h"
#include "kernel/sched/sched.h"

namespace evisor {

uint32_t VirtioGic::Read(uint16_t addr) {
  uint32_t res = 0;
  if (addr == 0x000) {
    res = regs_.GICD_CTLR;
  } else if (addr == 0x004) {
    res = regs_.GICD_TYPER;
  } else if (addr == 0x008) {
    res = regs_.GICD_IIDR;
  } else if (0x80 <= addr && addr < 0x100) {
    res = regs_.GICD_IGROUPR[(addr & 0xff) >> 2];
  } else if (addr < 0x180) {
    res = regs_.GICD_ISENABLER[(addr & 0xff) >> 2];
  } else if (addr < 0x200) {
    res = regs_.GICD_ICENABLER[(addr & 0xff) >> 2];
  } else if (addr < 0x280) {
    res = regs_.GICD_ISPENDR[(addr & 0xff) >> 2];
  } else if (addr < 0x300) {
    res = regs_.GICD_ICPENDR[(addr & 0xff) >> 2];
  } else if (addr < 0x380) {
    res = regs_.GICD_ISACTIVER[(addr & 0xff) >> 2];
  } else if (addr < 0x400) {
    res = regs_.GICD_ICACTIVER[(addr & 0xff) >> 2];
  } else if (addr < 0x7fc) {
    res = regs_.GICD_IPRIORITYR[(addr & 0x3ff) >> 2];
  } else if (0x800 <= addr && addr < 0xbfc) {
    res = regs_.GICD_ITARGETSR[(addr & 0x3ff) >> 2];
  } else if (0xc00 <= addr && addr < 0xd00) {
    res = regs_.GICD_ICFGR[(addr & 0xff) >> 2];
  } else if (0xe00 <= addr && addr < 0xf00) {
    res = regs_.GICD_NSACR[(addr & 0xff) >> 2];
  } else if (addr == 0xf00) {
    res = regs_.GICD_SGIR;
  } else if (0xf10 <= addr && addr < 0xf20) {
    res = regs_.GICD_CPENDSGIR[(addr & 0xf) >> 2];
  } else if (0xf20 <= addr && addr < 0xf30) {
    res = regs_.GICD_SPENDSGIR[(addr & 0xf) >> 2];
  } else if (addr == 0xfe8) {  // ICPIDR2: Peripheral ID2 Register, (RO)
    res = 0x20;                // GICv2
  } else {
    LOG_ERROR("Unexpected read: addr = %04x", addr);
  }

  // LOG_TRACE("virtio_gic_read: addr = %04x, data = %04x", addr, res);
  return res;
}

void VirtioGic::Write(uint32_t addr, uint32_t data) {
  // LOG_TRACE("virtio_gic_write: addr = %04x, data = %04x", addr, data);
  if (addr == 0x000) {
    regs_.GICD_CTLR = data;
  } else if (addr == 0x004) {
    regs_.GICD_TYPER = data;
  } else if (addr == 0x008) {
    regs_.GICD_IIDR = data;
  } else if (0x80 <= addr && addr < 0x100) {
    regs_.GICD_IGROUPR[(addr & 0xff) >> 2] = data;
  } else if (addr < 0x180) {
    const uint8_t i = (addr & 0xff) >> 2;
    for (int j = 0; j < 32; j++) {
      const uint8_t id = i * 32 + j;
      if ((data >> j) & 0x1) {
        LOG_TRACE("Enable IRQ (id = %d) from EL1", id);
        if (id < 32) {
          // TODO if it's enabled, NuttX will not boot fine.
          // irq_register(id, 0, 0xca, virtio_gic_irq_callback);
        } else if (id == 33) {
          // LOG_TRACE("URAT0 IRQ was enabled");
          // irq_register(id, 0, 0x7f, virtio_gic_irq_callback);
        } else {
          LOG_TRACE("TODO: id = %d", id);
        }
      }
    }
    regs_.GICD_ISENABLER[i] = data;
  } else if (addr < 0x200) {
    regs_.GICD_ICENABLER[(addr & 0xff) >> 2] = data;
  } else if (addr < 0x280) {
    regs_.GICD_ISPENDR[(addr & 0xff) >> 2] = data;
  } else if (addr < 0x300) {
    regs_.GICD_ICPENDR[(addr & 0xff) >> 2] = data;
  } else if (addr < 0x380) {
    regs_.GICD_ISACTIVER[(addr & 0xff) >> 2] = data;
  } else if (addr < 0x400) {
    regs_.GICD_ICACTIVER[(addr & 0xff) >> 2] = data;
  } else if (addr < 0x7fc) {
    regs_.GICD_IPRIORITYR[(addr & 0x3ff) >> 2] = data;
  } else if (0x800 <= addr && addr < 0xbfc) {
    regs_.GICD_ITARGETSR[(addr & 0x3ff) >> 2] = data;
  } else if (0xc00 <= addr && addr < 0xd00) {
    regs_.GICD_ICFGR[(addr & 0xff) >> 2] = data;
  } else if (0xe00 <= addr && addr < 0xf00) {
    regs_.GICD_NSACR[(addr & 0xff) >> 2] = data;
  } else if (addr == 0xf00) {
    regs_.GICD_SGIR = data;
  } else if (0xf10 <= addr && addr < 0xf20) {
    regs_.GICD_CPENDSGIR[(addr & 0xf) >> 2] = data;
  } else if (0xf20 <= addr && addr < 0xf30) {
    regs_.GICD_SPENDSGIR[(addr & 0xf) >> 2] = data;
  } else {
    LOG_WARN("Unexpected write: addr = %04x, data = %04x", addr, data);
  }
}

#if 0
static void virtio_gic_irq_callback() {
  // LOG_INFO("virtio_gic_irq_callback()");
  // LOG_INFO("count = %d", arm_generic_timer_get_timer_count());
  // LOG_INFO("cntv_cval_el0 = %d", cpu_read_sysreg(cntv_cval_el0));
  // tcb_t* tsk = scheduler_get_current_task();
  // tsk->stat.irq_pending = true;
}
#endif

}  // namespace evisor
