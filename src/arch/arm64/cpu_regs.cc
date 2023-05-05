#include "arch/arm64/cpu_regs.h"

uint8_t CpuRegGetCurrentEl() {
  /**
   * CurrentEL: Current Exception Level
   *  EL, bits [3:2]
   *   Possible values of this field are:
   *   00: EL0
   *   01: EL1
   *   10: EL2
   *   11: EL3
   **/
  uint64_t res;
  __asm__ volatile(
      "mrs %[res], CurrentEL\n"
      "lsr %[res], %[res], #2"
      : [res] "=r"(res)
      :
      :);

  return res;
}

void CpuRegLoadVCpuSysregs(VCpuSysregs* regs) {
  __asm__ volatile(
      "ldp x1, x2, [%[regs]], #16\n"
      "msr sctlr_el1, x1\n"
      "msr ttbr0_el1, x2\n"

      "ldp x1, x2, [%[regs]], #16\n"
      "msr ttbr1_el1, x1\n"
      "msr tcr_el1, x2\n"

      "ldp x1, x2, [%[regs]], #16\n"
      "msr esr_el1, x1\n"
      "msr far_el1, x2\n"

      "ldp x1, x2, [%[regs]], #16\n"
      "msr afsr0_el1, x1\n"
      "msr afsr1_el1, x2\n"

      "ldp x1, x2, [%[regs]], #16\n"
      "msr mair_el1, x1\n"
      "msr amair_el1, x2\n"

      "ldp x1, x2, [%[regs]], #16\n"
      "msr contextidr_el1, x1\n"
      "msr cpacr_el1, x2\n"

      "ldp x1, x2, [%[regs]], #16\n"
      "msr elr_el1, x1\n"
      "msr fpcr, x2\n"

      "ldp x1, x2, [%[regs]], #16\n"
      "msr fpsr, x1\n"
      "msr vpidr_el2, x2\n"  // TODO: check if this code needs.

      "ldp x1, x2, [%[regs]], #16\n"
      "msr vmpidr_el2, x1\n"  // TODO: check if this code needs.
      "msr par_el1, x2\n"

      "ldp x1, x2, [%[regs]], #16\n"
      "msr sp_el0, x1\n"
      "msr sp_el1, x2\n"

      "ldp x1, x2, [%[regs]], #16\n"
      "msr spsr_el1, x1\n"
      "msr tpidr_el0, x2\n"

      "ldp x1, x2, [%[regs]], #16\n"
      "msr tpidr_el1, x1\n"
      "msr tpidrro_el0, x2\n"

      "ldr x1, [%[regs]]\n"
      "msr vbar_el1, x1\n"

      "dsb ish\n"
      "isb"
      : [regs] "+r"(regs)
      :
      : "memory");
}

void CpuRegStoreVCpuSysregs(VCpuSysregs* regs) {
  __asm__ volatile(
      "dsb ish\n"
      "isb\n"

      "mrs x1, sctlr_el1\n"
      "mrs x2, ttbr0_el1\n"
      "stp x1, x2, [%[regs]], #16\n"

      "mrs x1, ttbr1_el1\n"
      "mrs x2, tcr_el1\n"
      "stp x1, x2, [%[regs]], #16\n"

      "mrs x1, esr_el1\n"
      "mrs x2, far_el1\n"
      "stp x1, x2, [%[regs]], #16\n"

      "mrs x1, afsr0_el1\n"
      "mrs x2, afsr1_el1\n"
      "stp x1, x2, [%[regs]], #16\n"

      "mrs x1, mair_el1\n"
      "mrs x2, amair_el1\n"
      "stp x1, x2, [%[regs]], #16\n"

      "mrs x1, contextidr_el1\n"
      "mrs x2, cpacr_el1\n"
      "stp x1, x2, [%[regs]], #16\n"

      "mrs x1, elr_el1\n"
      "mrs x2, fpcr\n"
      "stp x1, x2, [%[regs]], #16\n"

      "mrs x1, fpsr\n"
      "mrs x2, midr_el1\n"
      "stp x1, x2, [%[regs]], #16\n"

      "mrs x1, mpidr_el1\n"
      "mrs x2, par_el1\n"
      "stp x1, x2, [%[regs]], #16\n"

      "mrs x1, sp_el0\n"
      "mrs x2, sp_el1\n"
      "stp x1, x2, [%[regs]], #16\n"

      "mrs x1, spsr_el1\n"
      "mrs x2, tpidr_el0\n"
      "stp x1, x2, [%[regs]], #16\n"

      "mrs x1, tpidr_el1\n"
      "mrs x2, tpidrro_el0\n"
      "stp x1, x2, [%[regs]], #16\n"

      "mrs x1, vbar_el1\n"
      "str x1, [%[regs]]"
      : [regs] "+r"(regs)
      :
      : "memory");
}

void CpuRegLoadVCpuAllSysregs(VCpuSysregs* regs) {
  __asm__ volatile(
      "mrs x1, sctlr_el1\n"
      "mrs x2, ttbr0_el1\n"
      "stp x1, x2, [%[regs]], #16\n"

      "mrs x1, ttbr1_el1\n"
      "mrs x2, tcr_el1\n"
      "stp x1, x2, [%[regs]], #16\n"

      "mrs x1, esr_el1\n"
      "mrs x2, far_el1\n"
      "stp x1, x2, [%[regs]], #16\n"

      "mrs x1, afsr0_el1\n"
      "mrs x2, afsr1_el1\n"
      "stp x1, x2, [%[regs]], #16\n"

      "mrs x1, mair_el1\n"
      "mrs x2, amair_el1\n"
      "stp x1, x2, [%[regs]], #16\n"

      "mrs x1, contextidr_el1\n"
      "mrs x2, cpacr_el1\n"
      "stp x1, x2, [%[regs]], #16\n"

      "mrs x1, elr_el1\n"
      "mrs x2, fpcr\n"
      "stp x1, x2, [%[regs]], #16\n"

      "mrs x1, fpsr\n"
      "mrs x2, midr_el1\n"
      "stp x1, x2, [%[regs]], #16\n"

      "mrs x1, mpidr_el1\n"
      "mrs x2, par_el1\n"
      "stp x1, x2, [%[regs]], #16\n"

      "mrs x1, sp_el0\n"
      "mrs x2, sp_el1\n"
      "stp x1, x2, [%[regs]], #16\n"

      "mrs x1, spsr_el1\n"
      "mrs x2, tpidr_el0\n"
      "stp x1, x2, [%[regs]], #16\n"

      "mrs x1, tpidr_el1\n"
      "mrs x2, tpidrro_el0\n"
      "stp x1, x2, [%[regs]], #16\n"

      "mrs x1, vbar_el1\n"
      "mrs x2, actlr_el1\n"
      "stp x1, x2, [%[regs]], #16\n"

      "mrs x1, id_pfr0_el1\n"
      "mrs x2, id_pfr1_el1\n"
      "stp x1, x2, [%[regs]], #16\n"

      "mrs x1, id_mmfr0_el1\n"
      "mrs x2, id_mmfr1_el1\n"
      "stp x1, x2, [%[regs]], #16\n"

      "mrs x1, id_mmfr2_el1\n"
      "mrs x2, id_mmfr3_el1\n"
      "stp x1, x2, [%[regs]], #16\n"

      "mrs x1, id_isar0_el1\n"
      "mrs x2, id_isar1_el1\n"
      "stp x1, x2, [%[regs]], #16\n"

      "mrs x1, id_isar2_el1\n"
      "mrs x2, id_isar3_el1\n"
      "stp x1, x2, [%[regs]], #16\n"

      "mrs x1, id_isar4_el1\n"
      "mrs x2, id_isar5_el1\n"
      "stp x1, x2, [%[regs]], #16\n"

      "mrs x1, mvfr0_el1\n"
      "mrs x2, mvfr1_el1\n"
      "stp x1, x2, [%[regs]], #16\n"

      "mrs x1, mvfr2_el1\n"
      "mrs x2, id_aa64pfr0_el1\n"
      "stp x1, x2, [%[regs]], #16\n"

      "mrs x1, id_aa64pfr1_el1\n"
      "mrs x2, id_aa64dfr0_el1\n"
      "stp x1, x2, [%[regs]], #16\n"

      "mrs x1, id_aa64dfr1_el1\n"
      "mrs x2, id_aa64isar0_el1\n"
      "stp x1, x2, [%[regs]], #16\n"

      "mrs x1, id_aa64isar1_el1\n"
      "mrs x2, id_aa64mmfr0_el1\n"
      "stp x1, x2, [%[regs]], #16\n"

      "mrs x1, id_aa64mmfr1_el1\n"
      "mrs x2, id_aa64afr0_el1\n"
      "stp x1, x2, [%[regs]], #16\n"

      "mrs x1, id_aa64afr1_el1\n"
      "mrs x2, ctr_el0\n"
      "stp x1, x2, [%[regs]], #16\n"

      "mrs x1, ccsidr_el1\n"
      "mrs x2, clidr_el1\n"
      "stp x1, x2, [%[regs]], #16\n"

      "mrs x1, csselr_el1\n"
      "mrs x2, aidr_el1\n"
      "stp x1, x2, [%[regs]], #16\n"

      "mrs x1, revidr_el1\n"
      "mrs x2, cntkctl_el1\n"
      "stp x1, x2, [%[regs]], #16\n"

      "mrs x1, cntp_ctl_el0\n"
      "mrs x2, cntp_cval_el0\n"
      "stp x1, x2, [%[regs]], #16\n"

      "mrs x1, cntp_tval_el0\n"
      "mrs x2, cntv_ctl_el0\n"
      "stp x1, x2, [%[regs]], #16\n"

      "mrs x1, cntv_cval_el0\n"
      "mrs x2, cntv_tval_el0\n"
      "stp x1, x2, [%[regs]], #16"
      : [regs] "+r"(regs)
      :
      : "memory");
}
