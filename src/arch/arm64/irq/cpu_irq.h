#ifndef EVISOR_ARCH_ARM64_CPU_IRQ_H_
#define EVISOR_ARCH_ARM64_CPU_IRQ_H_

#include "arch/arm64/irq/gic.h"

extern "C" void CpuHandleIrq();

extern "C" void CpuCatchUnExpectedIrqs(uint32_t type,
                                       uint32_t esr_el2,
                                       uint32_t elr_el2,
                                       uint32_t far_el2);

namespace evisor {

// Enable IRQ for EL2.
void CpuEnableIrq();

// Disable IRQ for EL2.
void CpuDisableIrq();

// Set vector table for EL2.
void CpuInitIrqVectorTable();

// Route EL1's IRQs to EL2.
void CpuRouteIrqEl2();

void CpuEnableVFiqEl1();
void CpuDisableVFiqEl1();

void CpuEnableVIrqEl1();
void CpuDisableVirqEl1();

}  // namespace evisor

#endif  // EVISOR_ARCH_ARM64_CPU_IRQ_H_
