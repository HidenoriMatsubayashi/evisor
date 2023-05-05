#ifndef EVISOR_ARCH_COMMON_ASM_MACRO_H_
#define EVISOR_ARCH_COMMON_ASM_MACRO_H_

#define FUNCTION(sym) \
  sym:

#define GLOBAL_FUNCTION(sym) \
  .global sym;               \
  sym:

#endif  // EVISOR_ARCH_COMMON_ASM_MACRO_H_
