#ifndef EVISOR_ARCH_ARM64_CPU_REGS_DEF_H_
#define EVISOR_ARCH_ARM64_CPU_REGS_DEF_H_

#define STRINGIFY(x) #x

#define READ_CPU_REG(reg)                                                \
  ({                                                                     \
    uint64_t __val;                                                      \
    __asm__ volatile("mrs %0, " STRINGIFY(reg) : "=r"(__val)::"memory"); \
    __val;                                                               \
  })

#define WRITE_CPU_REG(reg, val) \
  ({ __asm__ volatile("msr " STRINGIFY(reg) ", %0" : : "r"(val) : "memory"); })

#endif  // EVISOR_ARCH_ARM64_CPU_REGS_DEF_H_
