#ifndef EVISOR_ARCH_ARM64_CPU_REGS_H_
#define EVISOR_ARCH_ARM64_CPU_REGS_H_

#include <cstdint>

#include "arch/arm64/cpu_regs_def.h"

struct VCpuSysregs {
  // Not trapped registers
  uint64_t sctlr_el1;
  uint64_t ttbr0_el1;
  uint64_t ttbr1_el1;
  uint64_t tcr_el1;
  uint64_t esr_el1;
  uint64_t far_el1;
  uint64_t afsr0_el1;
  uint64_t afsr1_el1;
  uint64_t mair_el1;
  uint64_t amair_el1;
  uint64_t contextidr_el1;
  uint64_t cpacr_el1;
  uint64_t elr_el1;
  uint64_t fpcr;
  uint64_t fpsr;
  uint64_t midr_el1;   // RO register
  uint64_t mpidr_el1;  // RO register
  uint64_t par_el1;
  uint64_t sp_el0;
  uint64_t sp_el1;
  uint64_t spsr_el1;
  uint64_t tpidr_el0;
  uint64_t tpidr_el1;
  uint64_t tpidrro_el0;
  uint64_t vbar_el1;

  // Reads of the following registers are trapped to EL2 (See HCR_EL2.TACR)
  uint64_t actlr_el1;  // rw

  // Reads of the following registers are trapped to EL2 (See HCR_EL2.TID3)
  uint64_t id_pfr0_el1;       // RO register
  uint64_t id_pfr1_el1;       // RO register
  uint64_t id_mmfr0_el1;      // RO register
  uint64_t id_mmfr1_el1;      // RO register
  uint64_t id_mmfr2_el1;      // RO register
  uint64_t id_mmfr3_el1;      // RO register
  uint64_t id_isar0_el1;      // RO register
  uint64_t id_isar1_el1;      // RO register
  uint64_t id_isar2_el1;      // RO register
  uint64_t id_isar3_el1;      // RO register
  uint64_t id_isar4_el1;      // RO register
  uint64_t id_isar5_el1;      // RO register
  uint64_t mvfr0_el1;         // RO register
  uint64_t mvfr1_el1;         // RO register
  uint64_t mvfr2_el1;         // RO register
  uint64_t id_aa64pfr0_el1;   // RO register
  uint64_t id_aa64pfr1_el1;   // RO register
  uint64_t id_aa64dfr0_el1;   // RO register
  uint64_t id_aa64dfr1_el1;   // RO register
  uint64_t id_aa64isar0_el1;  // RO register
  uint64_t id_aa64isar1_el1;  // RO register
  uint64_t id_aa64mmfr0_el1;  // RO register
  uint64_t id_aa64mmfr1_el1;  // RO register
  uint64_t id_aa64afr0_el1;   // RO register
  uint64_t id_aa64afr1_el1;   // RO register

  // Reads of the following registers are trapped to EL2 (See HCR_EL2.TID2)
  uint64_t ctr_el0;     // RO register
  uint64_t ccsidr_el1;  // RO register
  uint64_t clidr_el1;   // RO register
  uint64_t csselr_el1;  // R/W register

  // Reads of the following registers are trapped to EL2 (See HCR_EL2.TID1)
  uint64_t aidr_el1;    // RO register
  uint64_t revidr_el1;  // RO register

  // Arm generic timer
  uint64_t cntkctl_el1;
  uint64_t cntp_ctl_el0;
  uint64_t cntp_cval_el0;
  uint64_t cntp_tval_el0;
  uint64_t cntv_ctl_el0;
  uint64_t cntv_cval_el0;
  uint64_t cntv_tval_el0;
};

#ifdef __cplusplus
extern "C" {
#endif

uint8_t CpuRegGetCurrentEl();
void CpuRegLoadVCpuSysregs(VCpuSysregs* regs);
void CpuRegStoreVCpuSysregs(VCpuSysregs* regs);
void CpuRegLoadVCpuAllSysregs(VCpuSysregs* regs);

#ifdef __cplusplus
}
#endif

#endif  // EVISOR_ARCH_ARM64_CPU_REGS_H_
