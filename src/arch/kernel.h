#ifndef EVISOR_ARCH_KERNEL_H_
#define EVISOR_ARCH_KERNEL_H_

#include <cstdint>

#ifdef __cplusplus
extern "C" {
#endif

void KernelSwitchFromKThread() __attribute__((visibility("hidden")));

#ifdef __cplusplus
}
#endif

#endif  // EVISOR_ARCH_KERNEL_H_
