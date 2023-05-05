#ifndef EVISOR_ARCH_ARM64_KERNEL_TRAP_H_
#define EVISOR_ARCH_ARM64_KERNEL_TRAP_H_

#include <cstdint>

#ifdef __cplusplus
extern "C" {
#endif

void TrapHandleLowerElAarch64Sync(uint64_t esr, uint64_t elr, uint64_t far,
                                  uint64_t hvc_nr);

#ifdef __cplusplus
}
#endif

#endif  // EVISOR_ARCH_ARM64_KERNEL_TRAP_H_
