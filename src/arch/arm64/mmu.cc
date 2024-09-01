/***************************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.  The
 * ASF licenses this file to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance with the
 * License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 *
 ***************************************************************************/
#include "arch/arm64/mmu.h"

#include <algorithm>
#include <cstdint>

#include "arch/arm64/barriers.h"
#include "arch/arm64/cpu_regs.h"
#include "arch/mmu_config.h"
#include "common/assert.h"
#include "common/cstring.h"
#include "common/logger.h"
#include "common/macro.h"
#include "mm/pgtable.h"
#include "platforms/platform_config.h"

// clang-format off
#define XLAT_TABLE_ENTRIES_SHIFT                     (CONFIG_MMU_PAGE_SIZE_BITS - 3 /*Each table entry is 8 bytes*/)
#define XLAT_TABLE_ENTRIES                           (1 << XLAT_TABLE_ENTRIES_SHIFT)

/* Address size covered by each entry at given translation table level */
#define L3_XLAT_VA_SIZE_SHIFT                        CONFIG_MMU_PAGE_SIZE_BITS
#define L2_XLAT_VA_SIZE_SHIFT                        (L3_XLAT_VA_SIZE_SHIFT + XLAT_TABLE_ENTRIES_SHIFT)
#define L1_XLAT_VA_SIZE_SHIFT                        (L2_XLAT_VA_SIZE_SHIFT + XLAT_TABLE_ENTRIES_SHIFT)
#define L0_XLAT_VA_SIZE_SHIFT                        (L1_XLAT_VA_SIZE_SHIFT + XLAT_TABLE_ENTRIES_SHIFT)

// Get position (bit shift (position) in virtual address from translation table
// level
#define GET_VA_LEVEL_SHIFT(level) \
  (CONFIG_MMU_PAGE_SIZE_BITS +    \
   (XLAT_TABLE_ENTRIES_SHIFT * (CONFIG_MMU_TABLE_LEVEL_MAX - (level))))

/* Calculate the initial translation table level from CONFIG_MMU_VA_BITS
 * ------------------------------------
 * 4KB page size:
 * | L0[47:39] | L1 [38:30] | L2[29:21] | L3[20:12] | Offset[11:0] |
 * (va_bits <= 20)       - level 3
 * (21 <= va_bits <= 29) - level 2
 * (30 <= va_bits <= 38) - level 1
 * (39 <= va_bits <= 47) - level 0
 */
#define GET_PAGE_TABLE_BASE_LEVEL(va_bits)  \
  ((va_bits > L0_XLAT_VA_SIZE_SHIFT)   ? 0U \
   : (va_bits > L1_XLAT_VA_SIZE_SHIFT) ? 1U \
   : (va_bits > L2_XLAT_VA_SIZE_SHIFT) ? 2U \
                                       : 3U)
#define PAGE_TABLE_BASE_LEVEL                        GET_PAGE_TABLE_BASE_LEVEL(CONFIG_MMU_VA_BITS)

#define GET_NUM_BASE_LEVEL_ENTRIES(va_bits)          (1U << (va_bits - GET_VA_LEVEL_SHIFT(PAGE_TABLE_BASE_LEVEL)))

#define NUM_BASE_LEVEL_ENTRIES                       GET_NUM_BASE_LEVEL_ENTRIES(CONFIG_MMU_VA_BITS)


/* Page table entry descriptor
 * PTE[1]: 
 *  (L2 Entry)
 *   If 0, the descriptor gives the base address of a block of memory,
 *   and the attributes for that memory region
 *   If 1, the descriptor gives the address of the next level of translation
 *   table, and some attributes for that translation
 *  (L3 Entry)
 *   If 0, behaves identically to invalid bit[0] set to 0.
 *   If 1, gives the address of page of memory
 *
 *  PTE[0]: Identifies whether the descriptor is valid.
 *    0: Invalid / 1: Valid
 */
#define PTE_DESC_TYPE_MASK                           3
#define PTE_INVALID_DESC                             0
#define PTE_BLOCK_DESC                               1
#define PTE_TABLE_DESC                               3
#define PTE_PAGE_DESC                                3

/* Block and Page descriptor attributes fields */
#define PTE_BLOCK_DESC_MEMTYPE(x)                    ((x) << 2)
#define PTE_BLOCK_DESC_NS                            (1ULL << 5)
#define PTE_BLOCK_DESC_AP_RO                         (1ULL << 7)
#define PTE_BLOCK_DESC_AP_RW                         (0ULL << 7)
#define PTE_BLOCK_DESC_NON_SHARE                     (0ULL << 8)
#define PTE_BLOCK_DESC_OUTER_SHARE                   (2ULL << 8)
#define PTE_BLOCK_DESC_INNER_SHARE                   (3ULL << 8)
#define PTE_BLOCK_DESC_AF                            (1ULL << 10)
#define PTE_BLOCK_DESC_NG                            (1ULL << 11)
#define PTE_BLOCK_DESC_PXN                           (1ULL << 53)
#define PTE_BLOCK_DESC_UXN                           (1ULL << 54)

/* TCR definitions */
#define TCR_EL1_IPS_SHIFT                            32U
#define TCR_EL2_PS_SHIFT                             16U
#define TCR_EL3_PS_SHIFT                             16U

#define TCR_T0SZ_SHIFT                               0U
#define TCR_T0SZ(x)                                  ((64 - (x)) << TCR_T0SZ_SHIFT)

#define TCR_IRGN_NC                                  (0ULL << 8)
#define TCR_IRGN_WBWA                                (1ULL << 8)
#define TCR_IRGN_WT                                  (2ULL << 8)
#define TCR_IRGN_WBNWA                               (3ULL << 8)
#define TCR_IRGN_MASK                                (3ULL << 8)
#define TCR_ORGN_NC                                  (0ULL << 10)
#define TCR_ORGN_WBWA                                (1ULL << 10)
#define TCR_ORGN_WT                                  (2ULL << 10)
#define TCR_ORGN_WBNWA                               (3ULL << 10)
#define TCR_ORGN_MASK                                (3ULL << 10)
#define TCR_SHARED_NON                               (0ULL << 12)
#define TCR_SHARED_OUTER                             (2ULL << 12)
#define TCR_SHARED_INNER                             (3ULL << 12)
#define TCR_TG0_4K                                   (0ULL << 14)
#define TCR_TG0_64K                                  (1ULL << 14)
#define TCR_TG0_16K                                  (2ULL << 14)
#define TCR_EPD1_DISABLE                             (1ULL << 23)

#define TCR_PS_BITS_4GB                              0x0ULL
#define TCR_PS_BITS_64GB                             0x1ULL
#define TCR_PS_BITS_1TB                              0x2ULL
#define TCR_PS_BITS_4TB                              0x3ULL
#define TCR_PS_BITS_16TB                             0x4ULL
#define TCR_PS_BITS_256TB                            0x5ULL

#if (CONFIG_MMU_PA_BITS == 48)
#define TCR_PS_BITS                                  TCR_PS_BITS_256TB
#elif (CONFIG_MMU_PA_BITS == 44)
#define TCR_PS_BITS                                  TCR_PS_BITS_16TB
#elif (CONFIG_MMU_PA_BITS == 42)
#define TCR_PS_BITS                                  TCR_PS_BITS_4TB
#elif (CONFIG_MMU_PA_BITS == 40)
#define TCR_PS_BITS                                  TCR_PS_BITS_1TB
#elif (CONFIG_MMU_PA_BITS == 36)
#define TCR_PS_BITS                                  TCR_PS_BITS_64GB
#else
#define TCR_PS_BITS                                  TCR_PS_BITS_4GB
#endif

// ***************************************
// VTCR_EL2,  Virtualization Translation Control Register (EL2)
// ***************************************
#define VTCR_NSA                                     (1 << 30)
#define VTCR_NSW                                     (1 << 29)
#define VTCR_VS                                      (0 << 19)
#define VTCR_PS                                      (TCR_PS_BITS << 16)
#define VTCR_TG0                                     TCR_TG0_4K
#define VTCR_SH0                                     TCR_SHARED_INNER
#define VTCR_ORGN0                                   TCR_ORGN_WBWA
#define VTCR_IRGN0                                   TCR_IRGN_WBWA
#define VTCR_SL0                                     (1 << 6)   // start at level1
#define VTCR_T0SZ                                    TCR_T0SZ(CONFIG_MMU_VA_BITS)

#define VTCR_VALUE                                                 \
  (VTCR_NSA | VTCR_NSW | VTCR_VS | VTCR_PS | VTCR_TG0 | VTCR_SH0 | \
   VTCR_ORGN0 | VTCR_IRGN0 | VTCR_SL0 | VTCR_T0SZ)

// clang-format on

namespace evisor {

namespace {

/*
 * Default MAIR
 *                  index   attribute
 * DEVICE_nGnRnE      0     0000:0000
 * DEVICE_nGnRE       1     0000:0100
 * DEVICE_GRE         2     0000:1100
 * NORMAL_NC          3     0100:0100
 * NORMAL             4     1111:1111
 * NORMAL_WT          5     1011:1011
 */
constexpr uint64_t xDefaultMairEl2 =
    (0x00UL << (evisor::Mmu::PageMemoryAttribute::kDevicenGnRnE * 8)) |
    (0x04UL << (evisor::Mmu::PageMemoryAttribute::kDevicenGnRE * 8)) |
    (0x0cUL << (evisor::Mmu::PageMemoryAttribute::kDeviceGRE * 8)) |
    (0x44UL << (evisor::Mmu::PageMemoryAttribute::kNormalNC * 8)) |
    (0xffUL << (evisor::Mmu::PageMemoryAttribute::kNormal * 8));

uint64_t s_base_xlat_table[NUM_BASE_LEVEL_ENTRIES]
    __attribute__((aligned(NUM_BASE_LEVEL_ENTRIES * sizeof(uint64_t))));

uint64_t s_xlat_tables[CONFIG_MMU_MAX_XLAT_TABLES][XLAT_TABLE_ENTRIES]
    __attribute__((aligned(XLAT_TABLE_ENTRIES * sizeof(uint64_t))));

}  // namespace

void Mmu::Init(const std::array<MmuMapRegion, 7>& kernel_regions,
               const std::array<MmuMapRegion, 2>& guest_regions) {
  mmu_regions_kernel_ = kernel_regions;
  mmu_regions_guest_ = guest_regions;

  CheckMmuConfigs();
  CreatePageTables();

  /* Set MAIR, TCR and TBBR registers */
  {
    WRITE_CPU_REG(mair_el2, xDefaultMairEl2);  // Cache policies
    WRITE_CPU_REG(tcr_el2, GetTcr(2));
    WRITE_CPU_REG(ttbr0_el2, (uint64_t)s_base_xlat_table);
    WRITE_CPU_REG(vtcr_el2, VTCR_VALUE);
    evisor::Arm64Dsb();
    evisor::Arm64Isb();
  }

  Enable();
}

// static
void Mmu::Enable() {
  __asm__ volatile(
      "mrs x0, sctlr_el2\n"
      "orr x0, x0, #1 << 2\n"  // set SCTLR.C
      "orr x0, x0, #1 << 0\n"  // set SCTLR.M
      "dsb ish\n"
      "isb\n"
      "msr sctlr_el2, x0\n"
      "isb");
}

// static
void Mmu::Disable() {
  __asm__ volatile(
      "mrs x0, sctlr_el2\n"
      "bic x0, x0, #1 << 0\n"  // clear SCTLR.M
      "bic x0, x0, #1 << 2\n"  // clear SCTLR.C
      "msr sctlr_el2, x0\n"
      "isb");
}

// static
void Mmu::SetStage2PageTable(uint64_t table, uint64_t pid) {
  __asm__ volatile(
      // VMID
      "and %[pid], %[pid], #0xff\n"
      "lsl %[pid], %[pid], #48\n"
      "orr %[table], %[table], %[pid]\n"

      "msr vttbr_el2, %[table]\n"
      "dsb ish\n"
      "isb"
      :
      : [table] "r"(table), [pid] "r"(pid));
}

// static
uint64_t Mmu::TranslateEl1IpaToEl2Va(uint64_t el1_ipa) {
  uint64_t result = 0;
  __asm__ volatile(
      "at s12e1r, x0\n"  // Translation stage 1 by stage 2 translation (EL1)
      "mrs %[result], par_el1"
      : [result] "=r"(result)
      : [el1_ipa] "r"(el1_ipa));
  return result;
}

void Mmu::CheckMmuConfigs() {
  uint64_t max_va = 0;
  uint64_t max_pa = 0;

  // check virtual/physical address space size
  for (uint32_t i = 0; i < mmu_regions_guest_.size(); i++) {
    auto* region = &mmu_regions_guest_[i];
    max_pa = std::max(max_pa, region->base_pa + region->size);
    max_va = std::max(max_va, region->base_va + region->size);
  }
  ASSERT(max_pa <= (1ULL << CONFIG_MMU_PA_BITS),
         "Maximum physical address is over a supported range");
  ASSERT(max_va <= (1ULL << CONFIG_MMU_VA_BITS),
         "Maximum virtual address is over a supported range");
}

void Mmu::CreatePageTables() {
  // create translation tables for user platform regions
  for (uint8_t i = 0; i < mmu_regions_guest_.size(); i++) {
    auto& region = mmu_regions_guest_[i];
    InitXlatTables(region);
  }

  // create translation tables for kernel regions
  for (uint8_t index = 0; index < mmu_regions_kernel_.size(); index++) {
    auto& region = mmu_regions_kernel_[index];
    InitXlatTables(region);
  }
}

void Mmu::InitXlatTables(const MmuMapRegion& region) {
  uint64_t va = region.base_va;
  uint64_t pa = region.base_pa;
  uint64_t size = region.size;
  uint64_t attrs = region.attrs;
  uint32_t level = PAGE_TABLE_BASE_LEVEL;

#if defined(CONFIG_MMU_DEBUG)
  LOG_DEBUG("TLB: name = %s, va = %x, pa = %x, size = %x", region.name, va, pa,
            size);
#endif

  // check memory alignment
  ASSERT((va & (PAGE_SIZE - 1)) == 0, "virtual address is not page aligned");
  ASSERT((size & (PAGE_SIZE - 1)) == 0, "memory size is not page aligned");

  while (size > 0) {
    ASSERT(level <= CONFIG_MMU_TABLE_LEVEL_MAX,
           "max translation table level exceeded");

    // Locate PTE for given virtual address and page table level
    uint64_t* pte = FindPteIndex(va, level);
    ASSERT(pte != NULL, "Couln't find PTE");

    const uint64_t level_size = 1ULL << GET_VA_LEVEL_SHIFT(level);
    if (size >= level_size && !(va & (level_size - 1))) {
      /* Given range fits into level size,
       * create block/page descriptor
       */
      SetPteBlockDesc(pte, pa, attrs, level);
      va += level_size;
      pa += level_size;
      size -= level_size;

      // Block is mapped, start again for next range
      level = PAGE_TABLE_BASE_LEVEL;
    } else if (PteDescType(pte) == PTE_INVALID_DESC) {
      uint64_t* new_table = NewXlatTablesEntry();
      SetPteTableDesc(pte, new_table);
      level++;
    } else if (PteDescType(pte) == PTE_BLOCK_DESC) {
      SplitPteBlockDesc(pte, level);
      level++;
    } else if (PteDescType(pte) == PTE_TABLE_DESC) {
      level++;
    } else {
      PANIC("Never reach here!");
    }
  }
}

uint64_t* Mmu::FindPteIndex(uint64_t va, uint32_t level) {
  uint64_t* pte = NULL;

  // Traverse all translation tables
  pte = (uint64_t*)s_base_xlat_table;
  for (uint32_t i = PAGE_TABLE_BASE_LEVEL; i <= CONFIG_MMU_TABLE_LEVEL_MAX;
       i++) {
    pte += GetVaIndex(va, i);
    if (i == level) {
      return pte;
    }

    if (PteDescType(pte) != PTE_TABLE_DESC) {
      return NULL;
    }

    // Move on to the next translation table level
    // TODO: Currently only support only 4KB page size (3 level)
    pte = (uint64_t*)(*pte & 0x0000fffffffff000);
  }

  return NULL;
}

inline uint64_t Mmu::GetVaIndex(uint64_t va, uint32_t level) {
  return (va >> GET_VA_LEVEL_SHIFT(level)) & (XLAT_TABLE_ENTRIES - 1);
}

uint64_t* Mmu::NewXlatTablesEntry() {
  static uint32_t idx = 0;

  ASSERT(idx < CONFIG_MMU_MAX_XLAT_TABLES,
         "don't have enough xlat tables anymore");
  return (uint64_t*)(s_xlat_tables[idx++]);
}

inline uint8_t Mmu::PteDescType(uint64_t* pte) {
  return *pte & PTE_DESC_TYPE_MASK;
}

inline void Mmu::SetPteTableDesc(uint64_t* pte, uint64_t* table) {
#if defined(CONFIG_MMU_DEBUG)
  LOG_DEBUG("Set new table: pte = %x, page table = %x", pte, table);
#endif
  *pte = PTE_TABLE_DESC | (uint64_t)table;
}

void Mmu::SetPteBlockDesc(uint64_t* pte,
                          uint64_t pa,
                          uint32_t attrs,
                          uint32_t level) {
/* MAIR_ELx memory attributes */
#define MAIR_ATTR_MASK 0x7U
#define MAIR_ATTR(attr) ((attr)&MAIR_ATTR_MASK)

  auto desc = pa;

  desc |= (level == 3) ? PTE_PAGE_DESC : PTE_BLOCK_DESC;  // Type/Valid
  desc |= (attrs & PageTableEntryL3Desc::kNonSecure) ? PTE_BLOCK_DESC_NS : 0;
  desc |= (attrs & PageTableEntryL3Desc::kRW)
              ? PTE_BLOCK_DESC_AP_RW
              : PTE_BLOCK_DESC_AP_RO;  // Data access permission
  desc |= PTE_BLOCK_DESC_AF;           // Access flag

  const uint32_t mem_type = MAIR_ATTR(attrs);
  desc |= PTE_BLOCK_DESC_MEMTYPE(mem_type);  // Memory region attributes
  switch (mem_type) {
    case PageMemoryAttribute::kDevicenGnRnE:
    case PageMemoryAttribute::kDevicenGnRE:
    case PageMemoryAttribute::kDeviceGRE: {
      /* Access to Device memory and non-cacheable memory are coherent
       * for all observers in the system and are treated as
       * Outer shareable, so, for these 2 types of memory,
       * it is not strictly needed to set shareability field
       */
      desc |= PTE_BLOCK_DESC_OUTER_SHARE;

      /* Map device memory as execute-never */
      desc |= PTE_BLOCK_DESC_PXN;
      desc |= PTE_BLOCK_DESC_UXN;
      break;
    }
    case PageMemoryAttribute::kNormalNC:
    case PageMemoryAttribute::kNormal:
    case PageMemoryAttribute::kNormalWT: {
      if (mem_type == PageMemoryAttribute::kNormal) {
        desc |= PTE_BLOCK_DESC_INNER_SHARE;
      } else {
        desc |= PTE_BLOCK_DESC_OUTER_SHARE;
      }

      /* Make Normal RW memory as execute never */
      if (attrs & PageTableEntryL3Desc::kExecuteNever) {
        desc |= PTE_BLOCK_DESC_PXN;
      }
      break;
    }
    default:
      break;
  }

  *pte = desc;

#if defined(CONFIG_MMU_DEBUG)
  printf("%x: ", pte);
  printf("%s ",
         (mem_type == PageMemoryAttribute::kNormal)
             ? "MEM"
             : ((mem_type == PageMemoryAttribute::kNormalNC) ? "NC" : "DEV"));
  printf("%s ", (attrs & PageTableEntryL3Desc::kRW) ? "-RW" : "-RO");
  printf("%s ", (attrs & PageTableEntryL3Desc::kNonSecure) ? "-NS" : "-S");
  printf("%s\n",
         (attrs & PageTableEntryL3Desc::kExecuteNever) ? "-XN" : "-EXEC");
#endif
}

void Mmu::SplitPteBlockDesc(uint64_t* pte, uint32_t level) {
  const uint64_t old_block_desc = *pte;

  /* get address size shift bits for next level */
  const int levelshift = GET_VA_LEVEL_SHIFT(level + 1);

#if defined(CONFIG_MMU_DEBUG)
  // sinfo("Splitting existing PTE %p(L%d)\n", pte, level);
#endif

  uint64_t* new_table = NewXlatTablesEntry();
  for (uint32_t i = 0; i < XLAT_TABLE_ENTRIES; i++) {
    new_table[i] = old_block_desc | (i << levelshift);
    if (level == 2) {
      new_table[i] |= PTE_PAGE_DESC;
    }
  }

  /* Overwrite existing PTE set the new table into effect */
  SetPteTableDesc(pte, new_table);
}

uint64_t Mmu::GetTcr(int el) {
  const uint64_t va_bits = CONFIG_MMU_VA_BITS;
  const uint64_t tcr_ps_bits = TCR_PS_BITS;

  uint64_t tcr;
  if (el == 1) {
    tcr = (tcr_ps_bits << TCR_EL1_IPS_SHIFT);

    /* TCR_EL1.EPD1: Disable translation table walk for addresses
     * that are translated using TTBR1_EL1.
     */
    tcr |= TCR_EPD1_DISABLE;
  } else {
    tcr = (tcr_ps_bits << TCR_EL3_PS_SHIFT);
  }

  tcr |= TCR_T0SZ(va_bits);

  /* Translation table walk is cacheable, inner/outer WBWA and
   * inner shareable
   */
  tcr |= TCR_TG0_4K | TCR_SHARED_INNER | TCR_ORGN_WBWA | TCR_IRGN_WBWA;

  return tcr;
}

}  // namespace evisor
