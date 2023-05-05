#ifndef EVISOR_ARCH_ARM64_IRQ_GIC_V2_H_
#define EVISOR_ARCH_ARM64_IRQ_GIC_V2_H_

#include "arch/arm64/irq/gic.h"

namespace evisor {

class GicV2 : public Gic {
 public:
  GicV2();
  ~GicV2() = default;

  // Prevent copying.
  GicV2(GicV2 const &) = delete;
  GicV2 &operator=(GicV2 const &) = delete;

  static GicV2 &Get() noexcept {
    static GicV2 instance;
    return instance;
  }

  void Init() override;

  void HandleIrq() override;

  void RegisterIrq(uint16_t id, uint32_t target_processor, uint8_t priority,
                   IrqHandler handler) override;

  void NotifyVirqSoftware() override;

  void NotifyVirqHardware(uint16_t intid) override;

  void NotifyIrqSoftware(uint32_t sgi_id, uint32_t cpu_id) override;

  void PrintDumpRegisters() override;

 private:
  struct GicDistributorRegs {
    volatile uint32_t GICD_CTLR;
    volatile uint32_t GICD_TYPER;
    volatile uint32_t GICD_IIDR;
    volatile uint32_t __RESERVED_0[5];
    volatile uint32_t __IMPREMENTATION_DEFINED_0[8];
    volatile uint32_t __RESERVED_1[16];
    volatile uint32_t GICD_IGROUPR[32];
    volatile uint32_t GICD_ISENABLER[32];
    volatile uint32_t GICD_ICENABLER[32];
    volatile uint32_t GICD_ISPENDR[32];
    volatile uint32_t GICD_ICPENDR[32];
    volatile uint32_t GICD_ISACTIVER[32];
    volatile uint32_t GICD_ICACTIVER[32];
    volatile uint32_t GICD_IPRIORITYR[255];
    volatile uint32_t __RESERVED_2;
    volatile uint32_t GICD_ITARGETSR[255];
    volatile uint32_t __RESERVED_3;
    volatile uint32_t GICD_ICFGR[64];
    volatile uint32_t __IMPREMENTATION_DEFINED_1[64];
    volatile uint32_t GICD_NSACR[64];
    volatile uint32_t GICD_SGIR;
    volatile uint32_t __RESERVED4[3];
    volatile uint32_t GICD_CPENDSGIR[4];
    volatile uint32_t GICD_SPENDSGIR[4];
    volatile uint32_t __RESERVED5[40];
    volatile uint32_t __IMPREMENTATION_DEFINED_2[12];
  } __attribute__((packed)) __attribute__((aligned(4)));

  struct GicCpuInterfaceRegs {
    volatile uint32_t GICC_CTLR;
    volatile uint32_t GICC_PMR;
    volatile uint32_t GICC_BMR;
    volatile uint32_t GICC_IAR;
    volatile uint32_t GICC_EOIR;
    volatile uint32_t GICC_RPR;
    volatile uint32_t GICC_HPPIR;
    volatile uint32_t GICC_ABPR;
    volatile uint32_t GICC_AIAR;
    volatile uint32_t GICC_AEOIR;
    volatile uint32_t GICC_AHPPIR;
    volatile uint32_t __RESERVED_0[5];
    volatile uint32_t __IMPREMENTATION_DEFINED_0[36];
    volatile uint32_t GICC_APR[4];
    volatile uint32_t GICC_NSAPR[4];
    volatile uint32_t __RESERVED_1[2];
    volatile uint32_t GICC_IIDR;
    volatile uint32_t __RESERVED_2[960];
    volatile uint32_t GICC_DIR;
  } __attribute__((packed)) __attribute__((aligned(4)));

  struct GicVcpuInterfaceControlRegs {
    volatile uint32_t GICH_HCR;
    volatile uint32_t GICH_VTR;
    volatile uint32_t GICH_VMCR;
    volatile uint32_t __RESERVED_0;
    volatile uint32_t GICH_MISR;
    volatile uint32_t __RESERVED_1[3];
    volatile uint32_t GICH_EISR0;
    volatile uint32_t GICH_EISR1;
    volatile uint32_t __RESERVED_2[2];
    volatile uint32_t GICH_ELSR0;
    volatile uint32_t GICH_ELSR1;
    volatile uint32_t __RESERVED_3[46];
    volatile uint32_t GICH_APR;
    volatile uint32_t __RESERVED_4[3];
    volatile uint32_t GICH_LR[64];
  } __attribute__((packed)) __attribute__((aligned(4)));

  struct GicVcpuInterfaceRegs {
    volatile uint32_t GICV_CTLR;
    volatile uint32_t GICV_PMR;
    volatile uint32_t GICV_BMR;
    volatile uint32_t GICV_IAR;
    volatile uint32_t GICV_EOIR;
    volatile uint32_t GICV_RPR;
    volatile uint32_t GICV_HPPIR;
    volatile uint32_t GICV_ABPR;
    volatile uint32_t GICV_AIAR;
    volatile uint32_t GICV_AEOIR;
    volatile uint32_t GICV_AHPPIR;
    volatile uint32_t __RESERVED_0[5];
    volatile uint32_t __IMPREMENTATION_DEFINED_0[36];
    volatile uint32_t GICV_APR[4];
    volatile uint32_t __RESERVED_1[4];
    volatile uint32_t __RESERVED_2[2];
    volatile uint32_t GICV_IIDR;
    volatile uint32_t __RESERVED_3[960];
    volatile uint32_t GICV_DIR;
    volatile uint32_t __RESERVED_4[1024];
  } __attribute__((packed)) __attribute__((aligned(4)));

  struct GicRegs {
    volatile GicDistributorRegs *D;
    volatile GicCpuInterfaceRegs *C;
    volatile GicVcpuInterfaceControlRegs *H;
    volatile GicVcpuInterfaceRegs *V;
  };

  GicRegs regs_;
};

}  // namespace evisor

#endif  // EVISOR_ARCH_ARM64_IRQ_GIC_V2_H_
