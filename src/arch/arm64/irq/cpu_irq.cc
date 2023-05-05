#include "arch/arm64/irq/cpu_irq.h"

#include "arch/arm64/irq/gic_v2.h"

void CpuHandleIrq() {
  evisor::GicV2::Get().HandleIrq();
}

void CpuCatchUnExpectedIrqs(uint32_t type,
                            uint32_t esr_el2,
                            uint32_t elr_el2,
                            uint32_t far_el2) {
  evisor::GicV2::Get().CatchUnexpectedIrqs(type, esr_el2, elr_el2, far_el2);
}

namespace evisor {

void CpuEnableIrq() {
  __asm__ volatile("msr daifclr, #2");
}

void CpuDisableIrq() {
  __asm__ volatile("msr daifset, #2");
}

void CpuInitIrqVectorTable() {
  __asm__ volatile(
      "adr x0, vector_table_el2\n"
      "msr vbar_el2, x0");
}

void CpuRouteIrqEl2() {
  __asm__ volatile(
      "mrs x0, hcr_el2\n"
      "orr x1, x0, #0x10\n"
      "msr hcr_el2, x1");
}

void CpuEnableVFiqEl1() {
  __asm__ volatile(
      "mrs x0, hcr_el2\n"
      "orr x1, x0, #0x40\n"
      "msr hcr_el2, x1");
}

void CpuEnableVIrqEl1() {
  __asm__ volatile(
      "mrs x0, hcr_el2\n"
      "orr x1, x0, #0x80\n"
      "msr hcr_el2, x1");
}

void CpuDisableVFiqEl1() {
  __asm__ volatile(
      "mrs x0, hcr_el2\n"
      "bic x1, x0, #0x40\n"
      "msr hcr_el2, x1");
}

void CpuDisableVirqEl1() {
  __asm__ volatile(
      "mrs x0, hcr_el2\n"
      "bic x1, x0, #0x80\n"
      "msr hcr_el2, x1");
}

}  // namespace evisor
