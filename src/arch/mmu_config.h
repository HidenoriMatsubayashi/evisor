#ifndef EVISOR_ARCH_MMU_CONFIG_H_
#define EVISOR_ARCH_MMU_CONFIG_H_

/// Enable MMU log messages for debug
// #define CONFIG_MMU_DEBUG

/// The bit numbers of page table (= page size)
/// Currently support only 4KB page table
#define CONFIG_MMU_PAGE_SIZE_BITS 12

/// Virtual address space size
/// Allows choosing one of multiple possible virtual address
/// space sizes. The level of translation table is determined by
/// a combination of page size and virtual address space size.
///
/// The choice could be: 32, 36, 42, 48
#define CONFIG_MMU_VA_BITS 36

/// Physical address space size
/// Choose the maximum physical address range that the kernel will support.
///
/// The choice could be: 32, 36, 42, 48
#define CONFIG_MMU_PA_BITS 36

/// maximum level of page table
#define CONFIG_MMU_TABLE_LEVEL_MAX 3

/// maximum page table array size
#define CONFIG_MMU_MAX_XLAT_TABLES 14

#endif  // EVISOR_ARCH_MMU_CONFIG_H_
