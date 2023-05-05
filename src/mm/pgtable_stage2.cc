#include "mm/pgtable_stage2.h"

#include "arch/arm64/mmu.h"
#include "common/logger.h"
#include "kernel/sched/sched.h"
#include "mm/heap/kmm_malloc.h"
#include "mm/heap/kmm_zalloc.h"
#include "platforms/platform.h"
#include "platforms/platform_config.h"

namespace evisor {

namespace {

constexpr uint64_t kStage2PteTypeBlock = 1;
constexpr uint64_t kStage2PteTypePage = 3;
constexpr uint64_t kStage2PteTypePageTable = 3;

// AF, bits[10]
constexpr uint64_t kStage2PteAf = (1 << 10);

/*
 * SH[1:0], bits[9:8]
 */
// Non-shaeable
constexpr uint64_t kStage2PteShNonShareable = (0 << 8);
// SH Inner Shareable
constexpr uint64_t kStage2PteShInnerShareable = (2 << 8);
// SH Outer Shareable
constexpr uint64_t kStage2PteShOuterShareable = (3 << 8);

/*
 * S2AP[1:0], bits[7:6]. Access Permissions bits
 * See: Memory access control on page D4-1704.
 */
// No access from EL1
constexpr uint64_t kStage2PteS2ApNone = (0 << 6);
// RO access from EL1
constexpr uint64_t kStage2PteS2ApRO = (1 << 6);
// WO access from EL1
constexpr uint64_t kStage2PteS2ApWO = (2 << 6);
// R/W Full access from EL1
constexpr uint64_t kStage2PteS2ApRW = (3 << 6);

/*
 * MemAttr[3:0] , bits[5:2]
 * See also MAIR_EL2 register setting in arch/arm64/mmu.cc
 */
// Normal inner write back
constexpr uint64_t kStage2PteMemAttrWb = (0xf << 2);
// DEVICE_nGnRnE
constexpr uint64_t kStage2PteMemAttrDevice_nGnRnE = (0x0 << 2);

// Stage2 page table entry for DRAM
constexpr uint64_t kStage2PteDram = kStage2PteTypePage | kStage2PteAf |
                                    kStage2PteShOuterShareable |
                                    kStage2PteS2ApRW | kStage2PteMemAttrWb;

// Stage2 page table entry for Devie I/O (Not accessible)
constexpr uint64_t kStage2PteDeviceNotAccessible =
    kStage2PteTypePage | kStage2PteAf | kStage2PteShNonShareable |
    kStage2PteS2ApNone | kStage2PteMemAttrDevice_nGnRnE;

// Stage2 page table entry for Devie I/O (accessible)
constexpr uint64_t kStage2PteDeviceAccessible =
    kStage2PteTypePage | kStage2PteAf | kStage2PteShNonShareable |
    kStage2PteS2ApRW | kStage2PteMemAttrDevice_nGnRnE;
}  // namespace

void PgTableStage2::MapPageAccessible(Tcb* task, ipa_t ipa, pa_t page) {
  MapPage(task, ipa, page, kStage2PteDram);
}

void PgTableStage2::MapNewPage(Tcb* tsk, ipa_t ipa, va_t page) {
  auto* pte_addr = MapPage(tsk, ipa & PAGE_MASK, page, kStage2PteDram);
  FlushDCache(pte_addr);
  FlushTlbVMID();
}

void PgTableStage2::MapNewDevicePage(Tcb* task,
                                     ipa_t ipa,
                                     pa_t page,
                                     bool accessable) {
  MapPage(
      task, ipa & PAGE_MASK, page,
      accessable ? kStage2PteDeviceAccessible : kStage2PteDeviceNotAccessible);
}

pa_t PgTableStage2::GetIpa(va_t va) {
  auto ipa = Mmu::TranslateEl1IpaToEl2Va(va);
  ipa &= 0xFFFFFFFFF000;
  ipa |= va & 0xFFF;
  return ipa;
}

// TODO: refactoring these definies.
#define TABLE_SHIFT 9
#define PTRS_PER_TABLE (1 << TABLE_SHIFT)
#define LV0_SHIFT PAGE_SHIFT + 3 * TABLE_SHIFT
#define LV1_SHIFT PAGE_SHIFT + 2 * TABLE_SHIFT
#define LV2_SHIFT PAGE_SHIFT + 1 * TABLE_SHIFT
#define LV3_SHIFT PAGE_SHIFT

void* PgTableStage2::MapPage(Tcb* task, ipa_t ipa, pa_t page, uint64_t flags) {
  if (!task->mm.page_table) {
    task->mm.page_table = reinterpret_cast<uint64_t>(kmm_zalloc(PAGE_SIZE));
  }

  pa_t lv1_table = task->mm.page_table;
  pa_t lv2_table = CreatePageTable(lv1_table, LV1_SHIFT, ipa);
  pa_t lv3_table = CreatePageTable(lv2_table, LV2_SHIFT, ipa);

  task->mm.pages++;
  return SetPageTableEntry(lv3_table, ipa, page, flags);
}

pa_t PgTableStage2::CreatePageTable(va_t table, uint64_t shift, ipa_t ipa) {
  uint64_t index = ipa >> shift;
  index = index & (PTRS_PER_TABLE - 1);

  if (!reinterpret_cast<uint64_t*>(table)[index]) {
    auto next_level_table = reinterpret_cast<va_t>(kmm_zalloc(PAGE_SIZE));
    uint64_t entry = next_level_table | kStage2PteTypePageTable;

    reinterpret_cast<uint64_t*>(table)[index] = entry;
    FlushDCache(&(reinterpret_cast<uint64_t*>(table)[index]));

    return next_level_table;
  }

  return reinterpret_cast<uint64_t*>(table)[index] & PAGE_MASK;
}

void* PgTableStage2::SetPageTableEntry(va_t pte,
                                       ipa_t ipa,
                                       pa_t pa,
                                       uint64_t flags) {
  uint64_t index = ipa >> PAGE_SHIFT;
  index = index & (PTRS_PER_TABLE - 1);

  uint64_t entry = pa | flags;
  reinterpret_cast<uint64_t*>(pte)[index] = entry;
  return &(reinterpret_cast<uint64_t*>(pte)[index]);
}

// TODO: move this coude to arch/arm64 directory
void PgTableStage2::FlushDCache(void* addr) {
  __asm__ volatile(
      "dc civac, %[addr]\n"
      "dsb sy"
      :
      : [addr] "r"(addr)
      : "memory");
}

// TODO: move this coude to arch/arm64 directory
void PgTableStage2::FlushTlbVMID() {
  __asm__ volatile(
      "dsb ish\n"
      "tlbi vmalls12e1is\n"
      "dsb ish\n"
      "isb");
  WRITE_CPU_REG(vttbr_el2, 0);
}

}  // namespace evisor
