#ifndef EVISOR_ARCH_ARM64_BARRIERS_H_
#define EVISOR_ARCH_ARM64_BARRIERS_H_

namespace evisor {

static inline void Arm64Isb() { __asm__ volatile("isb"); }

static inline void Arm64Dsb() { __asm__ volatile("dsb"); }
static inline void Arm64DsbAllCore() { __asm__ volatile("dsb sy"); }

}  // namespace evisor

#endif  // EVISOR_ARCH_ARM64_BARRIERS_H_
