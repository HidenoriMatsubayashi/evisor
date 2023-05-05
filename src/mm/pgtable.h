#ifndef EVISOR_MM_PGTABLE_H_
#define EVISOR_MM_PGTABLE_H_

#include "arch/mmu_config.h"

// clang-format off
#define PAGE_SIZE           (1U << CONFIG_MMU_PAGE_SIZE_BITS)
#define PAGE_MASK           0xfffffffffffff000
#define PAGE_SHIFT          CONFIG_MMU_PAGE_SIZE_BITS
// clang-format on

#ifndef __ASSEMBLER__

#include <cstdint>

typedef uint64_t pa_t;
typedef uint64_t va_t;
typedef uint64_t ipa_t;

#endif

#endif  // EVISOR_MM_PGTABLE_H_
