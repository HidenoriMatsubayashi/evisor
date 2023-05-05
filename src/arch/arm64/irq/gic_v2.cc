#include "arch/arm64/irq/gic_v2.h"

#include "common/cstdio.h"
#include "common/logger.h"
#include "platforms/platform.h"

namespace evisor {

namespace {

/* GICC_CTLR */
constexpr uint32_t kGiccCtlrEnableGrp1 = (1 << 1);
constexpr uint32_t kGiccCtlrEnableGrp0 = (1 << 0);

/* GICD_CTLR */
constexpr uint32_t kGicdCtlrEnableGrp1 = (1 << 1);
constexpr uint32_t kGicdCtlrEnableGrp0 = (1 << 0);

/* GICH_HCR */
constexpr uint32_t kGichHcrEn = (1 << 0);

/* GICH_LR */
constexpr uint32_t kGichLrHw = (1 << 31);
constexpr uint32_t kGichLrVirtualIrqGrp0 = (0 << 30);
constexpr uint32_t kGichLrVirtualIrqGrp1 = (1 << 30);
constexpr uint32_t kGichLrStatePending = (1 << 28);
constexpr uint32_t GICH_LR_PRIORITY(uint32_t x) { return x << 23; }
constexpr uint32_t GICH_LR_PHYSICAL_ID(uint32_t x) { return x << 10; }
constexpr uint32_t GICH_LR_VIRTUAL_ID(uint32_t x) { return x << 0; }

}  // namespace

GicV2::GicV2() {
  regs_ = {
      .D = reinterpret_cast<GicDistributorRegs *>(GIC_V2_DISTRIBUTOR_BASE),
      .C = reinterpret_cast<GicCpuInterfaceRegs *>(GIC_V2_CPU_INTERFACE_BASE),
      .H = reinterpret_cast<GicVcpuInterfaceControlRegs *>(
          GIC_V2_HYPERVISOR_BASE),
      .V = reinterpret_cast<GicVcpuInterfaceRegs *>(GIC_V2_VIRTUAL_CPU_BASE),
  };
}

void GicV2::Init() {
  // disable distribute
  regs_.D->GICD_CTLR = 0;

  // set interrupt priority mask (RPI4 sets 0xf0)
  regs_.C->GICC_PMR = 0xf0;

  // enable CPU interface
  regs_.C->GICC_CTLR = kGiccCtlrEnableGrp1 | kGiccCtlrEnableGrp0;

  // enable distribute
  regs_.D->GICD_CTLR = kGicdCtlrEnableGrp1 | kGicdCtlrEnableGrp0;

  // enable Virtual CPU interface operation.
  regs_.H->GICH_HCR = kGichHcrEn;
}

void GicV2::RegisterIrq(uint16_t id, uint32_t target_processor,
                        uint8_t priority, IrqHandler handler) {
  irq_handler_[id] = handler;

  // enable interrupt
  regs_.D->GICD_ISENABLER[id / 32] =
      regs_.D->GICD_ISENABLER[id / 32] | (1 << (id % 32));

  // set the priority (lower numbers have higher priority)
  const uint32_t priority_mask = ~((uint32_t)priority << ((id % 4) * 8));
  regs_.D->GICD_IPRIORITYR[id / 4] =
      regs_.D->GICD_IPRIORITYR[id / 4] & priority_mask;

  // set target processor
  const uint32_t itargetsr_shift = ((id % 4) * 8);
  uint32_t itargetsr_tmp = regs_.D->GICD_ITARGETSR[id / 4];
  itargetsr_tmp &= ~((uint32_t)0xff << itargetsr_shift);
  itargetsr_tmp |= ((uint32_t)0x01 << target_processor) << itargetsr_shift;
  regs_.D->GICD_ITARGETSR[id / 4] = itargetsr_tmp;
}

void GicV2::HandleIrq() {
  const uint32_t iar = regs_.C->GICC_IAR & 0x0fff;
  const uint32_t core = iar >> 10;
  const uint32_t id = iar & 0x3ff;

  if (irq_handler_[id] != NULL) {
    irq_handler_[id]();
  } else {
    LOG_ERROR("Unexpected IRQ core: %d, id: %d", core, id);
  }

  regs_.C->GICC_EOIR = iar;
}

void GicV2::NotifyVirqSoftware() {}

void GicV2::NotifyVirqHardware(uint16_t intid) {
  regs_.H->GICH_LR[0] = kGichLrHw | kGichLrVirtualIrqGrp0 |
                        kGichLrStatePending | GICH_LR_PRIORITY(0) |
                        GICH_LR_PHYSICAL_ID(intid) | GICH_LR_VIRTUAL_ID(intid);
}

void GicV2::NotifyIrqSoftware(uint32_t sgi_id, uint32_t cpu_id) {
  const uint32_t val = (sgi_id & 0xf) | ((cpu_id & 0xff) << 16);
  regs_.D->GICD_SGIR = val;
}

void GicV2::PrintDumpRegisters() {
  LOG_TRACE("GICD_CTLR: %x", regs_.D->GICD_CTLR);
  LOG_TRACE("GICD_TYPER: %x", regs_.D->GICD_TYPER);
  LOG_TRACE("GICD_IIDR: %x", regs_.D->GICD_IIDR);
  LOG_TRACE("GICD_IGROUPR[0]: %x", regs_.D->GICD_IGROUPR[0]);
  LOG_TRACE("GICD_ISENABLER[0]: %x", regs_.D->GICD_ISENABLER[0]);
  LOG_TRACE("GICD_ICENABLER[0]: %x", regs_.D->GICD_ICENABLER[0]);
  LOG_TRACE("GICD_ISPENDR[0]: %x", regs_.D->GICD_ISPENDR[0]);
  LOG_TRACE("GICD_ICPENDR[0]: %x", regs_.D->GICD_ICPENDR[0]);
  LOG_TRACE("GICD_ISACTIVER[0]: %x", regs_.D->GICD_ISACTIVER[0]);
  LOG_TRACE("GICD_ICACTIVER[0]: %x", regs_.D->GICD_ICACTIVER[0]);
  LOG_TRACE("GICD_IPRIORITYR[5]: %x", regs_.D->GICD_IPRIORITYR[5]);
  LOG_TRACE("GICD_IPRIORITYR[6]: %x", regs_.D->GICD_IPRIORITYR[6]);
  LOG_TRACE("GICD_IPRIORITYR[7]: %x", regs_.D->GICD_IPRIORITYR[7]);
  LOG_TRACE("GICD_ITARGETSR[6]: %x", regs_.D->GICD_ITARGETSR[6]);
  // GICD_ICFGR[64];
  // GICD_NSACR[64];
  LOG_TRACE("GICD_SGIR: %x", regs_.D->GICD_SGIR);
  LOG_TRACE("GICD_CPENDSGIR[0]: %x", regs_.D->GICD_CPENDSGIR[0]);
  LOG_TRACE("GICD_CPENDSGIR[1]: %x", regs_.D->GICD_CPENDSGIR[1]);
  LOG_TRACE("GICD_CPENDSGIR[2]: %x", regs_.D->GICD_CPENDSGIR[2]);
  LOG_TRACE("GICD_CPENDSGIR[3]: %x", regs_.D->GICD_CPENDSGIR[3]);
  LOG_TRACE("GICD_SPENDSGIR[0]: %x", regs_.D->GICD_SPENDSGIR[0]);
  LOG_TRACE("GICD_SPENDSGIR[1]: %x", regs_.D->GICD_SPENDSGIR[1]);
  LOG_TRACE("GICD_SPENDSGIR[2]: %x", regs_.D->GICD_SPENDSGIR[2]);
  LOG_TRACE("GICD_SPENDSGIR[3]: %x", regs_.D->GICD_SPENDSGIR[3]);

  LOG_TRACE("GICC_CTLR: %x", regs_.C->GICC_CTLR);
  LOG_TRACE("GICC_PMR: %x", regs_.C->GICC_PMR);
  LOG_TRACE("GICC_BMR: %x", regs_.C->GICC_BMR);
  LOG_TRACE("GICC_IAR: %x", regs_.C->GICC_IAR);
  LOG_TRACE("GICC_EOIR: %x", regs_.C->GICC_EOIR);
  LOG_TRACE("GICC_RPR: %x", regs_.C->GICC_RPR);
  LOG_TRACE("GICC_HPPIR: %x", regs_.C->GICC_HPPIR);
  LOG_TRACE("GICC_ABPR: %x", regs_.C->GICC_ABPR);
  LOG_TRACE("GICC_AIAR: %x", regs_.C->GICC_AIAR);
  LOG_TRACE("GICC_AEOIR: %x", regs_.C->GICC_AEOIR);
  LOG_TRACE("GICC_AHPPIR: %x", regs_.C->GICC_AHPPIR);
  LOG_TRACE("GICC_APR[0]: %x", regs_.C->GICC_APR[0]);
  LOG_TRACE("GICC_APR[1]: %x", regs_.C->GICC_APR[1]);
  LOG_TRACE("GICC_APR[2]: %x", regs_.C->GICC_APR[2]);
  LOG_TRACE("GICC_APR[3]: %x", regs_.C->GICC_APR[3]);
  LOG_TRACE("GICC_NSAPR[0]: %x", regs_.C->GICC_NSAPR[0]);
  LOG_TRACE("GICC_NSAPR[1]: %x", regs_.C->GICC_NSAPR[1]);
  LOG_TRACE("GICC_NSAPR[2]: %x", regs_.C->GICC_NSAPR[2]);
  LOG_TRACE("GICC_NSAPR[3]: %x", regs_.C->GICC_NSAPR[3]);
  LOG_TRACE("GICC_IIDR: %x", regs_.C->GICC_IIDR);
  LOG_TRACE("GICC_DIR: %x", regs_.C->GICC_DIR);

  LOG_TRACE("GICH_HCR: %x", regs_.H->GICH_HCR);
  LOG_TRACE("GICH_VTR: %x", regs_.H->GICH_VTR);
  LOG_TRACE("GICH_VMCR: %x", regs_.H->GICH_VMCR);
  LOG_TRACE("GICH_MISR: %x", regs_.H->GICH_MISR);
  LOG_TRACE("GICH_EISR0: %x", regs_.H->GICH_EISR0);
  LOG_TRACE("GICH_EISR1: %x", regs_.H->GICH_EISR1);
  LOG_TRACE("GICH_ELSR0: %x", regs_.H->GICH_ELSR0);
  LOG_TRACE("GICH_ELSR1: %x", regs_.H->GICH_ELSR1);
  LOG_TRACE("GICH_APR: %x", regs_.H->GICH_APR);
  for (int i = 0; i < 64; i++) {
    LOG_TRACE("GICH_LR[%d]: %x", i, regs_.H->GICH_LR[i]);
  }

  LOG_TRACE("GICV_CTLR: %x", regs_.V->GICV_CTLR);
  LOG_TRACE("GICV_PMR: %x", regs_.V->GICV_PMR);
  LOG_TRACE("GICV_BMR: %x", regs_.V->GICV_BMR);
  LOG_TRACE("GICV_IAR: %x", regs_.V->GICV_IAR);
  LOG_TRACE("GICV_EOIR: %x", regs_.V->GICV_EOIR);
  LOG_TRACE("GICV_RPR: %x", regs_.V->GICV_RPR);
  LOG_TRACE("GICV_HPPIR: %x", regs_.V->GICV_HPPIR);
  LOG_TRACE("GICV_ABPR: %x", regs_.V->GICV_ABPR);
  LOG_TRACE("GICV_AIAR: %x", regs_.V->GICV_AIAR);
  LOG_TRACE("GICV_AEOIR: %x", regs_.V->GICV_AEOIR);
  LOG_TRACE("GICV_AHPPIR: %x", regs_.V->GICV_AHPPIR);
  LOG_TRACE("GICV_APR[0]: %x", regs_.V->GICV_APR[0]);
  LOG_TRACE("GICV_APR[1]: %x", regs_.V->GICV_APR[1]);
  LOG_TRACE("GICV_APR[2]: %x", regs_.V->GICV_APR[2]);
  LOG_TRACE("GICV_APR[3]: %x", regs_.V->GICV_APR[3]);
  LOG_TRACE("GICV_IIDR: %x", regs_.V->GICV_IIDR);
  LOG_TRACE("GICV_DIR: %x", regs_.V->GICV_DIR);
}

}  // namespace evisor
