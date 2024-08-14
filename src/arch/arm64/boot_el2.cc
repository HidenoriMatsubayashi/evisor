#include "arch/arm64/boot_el2.h"

#include "arch/arm64/arm_generic_timer.h"
#include "arch/arm64/barriers.h"
#include "arch/arm64/cpu_regs.h"
#include "arch/arm64/irq/cpu_irq.h"
#include "arch/arm64/irq/gic_v2.h"
#include "arch/arm64/mmu.h"
#include "arch/ld_symbols.h"
#include "common/macro.h"
#include "platforms/platform_config.h"
#include "platforms/serial.h"

namespace {

const std::array<evisor::Mmu::MmuMapRegion, 2> kMmuGuestRegions = {
    {{.name = "device_io",
      .base_pa = CONFIG_DEVICEIO_BASEADDR,
      .base_va = CONFIG_DEVICEIO_BASEADDR,
      .size = CONFIG_DEVICEIO_SIZE,
      .attrs =
          static_cast<uint64_t>(
              evisor::Mmu::PageMemoryAttribute::kDevicenGnRnE) |
          static_cast<uint64_t>(evisor::Mmu::PageTableEntryL3Desc::kRW) |
          static_cast<uint64_t>(evisor::Mmu::PageTableEntryL3Desc::kSecure)},
     {.name = "user_dram",
      .base_pa = reinterpret_cast<uint64_t>(__user_space_start),
      .base_va = reinterpret_cast<uint64_t>(__user_space_start),
      .size = reinterpret_cast<uint64_t>(__user_space_size),
      .attrs =
          static_cast<uint64_t>(
              evisor::Mmu::PageMemoryAttribute::kNormalNC) |  // TODO
          static_cast<uint64_t>(evisor::Mmu::PageTableEntryL3Desc::kRW) |
          static_cast<uint64_t>(evisor::Mmu::PageTableEntryL3Desc::kSecure)}}};

const std::array<evisor::Mmu::MmuMapRegion, 7> kMmuKernelRegions = {{
    // text segment: cacheable, read only, executable, secure
    {
        .name = "kernel_code",
        .base_pa = reinterpret_cast<uint64_t>(__text_start),
        .base_va = reinterpret_cast<uint64_t>(__text_start),
        .size = reinterpret_cast<uint64_t>(__text_size),
        .attrs =
            static_cast<uint64_t>(evisor::Mmu::PageMemoryAttribute::kNormal) |
            static_cast<uint64_t>(evisor::Mmu::PageTableEntryL3Desc::kRO) |
            static_cast<uint64_t>(evisor::Mmu::PageTableEntryL3Desc::kExecute) |
            static_cast<uint64_t>(evisor::Mmu::PageTableEntryL3Desc::kSecure),
    },
    // rodata segment: cacheable, read only, execute-never, secure
    {
        .name = "kernel_rdata",
        .base_pa = reinterpret_cast<uint64_t>(__rodata_start),
        .base_va = reinterpret_cast<uint64_t>(__rodata_start),
        .size = reinterpret_cast<uint64_t>(__rodata_size),
        .attrs =
            static_cast<uint64_t>(evisor::Mmu::PageMemoryAttribute::kNormal) |
            static_cast<uint64_t>(evisor::Mmu::PageTableEntryL3Desc::kRO) |
            static_cast<uint64_t>(
                evisor::Mmu::PageTableEntryL3Desc::kExecuteNever) |
            static_cast<uint64_t>(evisor::Mmu::PageTableEntryL3Desc::kSecure),
    },
    /* Mark rest of the mirtos execution regions (data, bss, noinit, etc.)
     * cacheable, read-write
     * Note: read-write region is marked execute-ever internally
     */
    {
        .name = "kernel_data",
        .base_pa = reinterpret_cast<uint64_t>(__data_start),
        .base_va = reinterpret_cast<uint64_t>(__data_start),
        .size = reinterpret_cast<uint64_t>(__data_size),
        .attrs =
            static_cast<uint64_t>(evisor::Mmu::PageMemoryAttribute::kNormal) |
            static_cast<uint64_t>(evisor::Mmu::PageTableEntryL3Desc::kRW) |
            static_cast<uint64_t>(evisor::Mmu::PageTableEntryL3Desc::kSecure),
    },
    {
        .name = "kernel_bss",
        .base_pa = reinterpret_cast<uint64_t>(__bss_start),
        .base_va = reinterpret_cast<uint64_t>(__bss_start),
        .size = reinterpret_cast<uint64_t>(__bss_size),
        .attrs =
            static_cast<uint64_t>(evisor::Mmu::PageMemoryAttribute::kNormal) |
            static_cast<uint64_t>(evisor::Mmu::PageTableEntryL3Desc::kRW) |
            static_cast<uint64_t>(evisor::Mmu::PageTableEntryL3Desc::kSecure),
    },
    {
        .name = "kernel_heap",
        .base_pa = reinterpret_cast<uint64_t>(__heap_start),
        .base_va = reinterpret_cast<uint64_t>(__heap_start),
        .size = reinterpret_cast<uint64_t>(__heap_size),
        .attrs =
            static_cast<uint64_t>(evisor::Mmu::PageMemoryAttribute::kNormal) |
            static_cast<uint64_t>(evisor::Mmu::PageTableEntryL3Desc::kRW) |
            static_cast<uint64_t>(evisor::Mmu::PageTableEntryL3Desc::kSecure),
    },
    {
        .name = "kernel_stack",
        .base_pa = reinterpret_cast<uint64_t>(__stack_start),
        .base_va = reinterpret_cast<uint64_t>(__stack_start),
        .size = reinterpret_cast<uint64_t>(__stack_size),
        .attrs =
            static_cast<uint64_t>(evisor::Mmu::PageMemoryAttribute::kNormal) |
            static_cast<uint64_t>(evisor::Mmu::PageTableEntryL3Desc::kRW) |
            static_cast<uint64_t>(evisor::Mmu::PageTableEntryL3Desc::kSecure),
    },
    {
        .name = "kernel_uncached",
        .base_pa = reinterpret_cast<uint64_t>(__uncached_space_start),
        .base_va = reinterpret_cast<uint64_t>(__uncached_space_start),
        .size = reinterpret_cast<uint64_t>(__uncached_space_size),
        .attrs =
            static_cast<uint64_t>(evisor::Mmu::PageMemoryAttribute::kNormalNC) |
            static_cast<uint64_t>(evisor::Mmu::PageTableEntryL3Desc::kRW) |
            static_cast<uint64_t>(evisor::Mmu::PageTableEntryL3Desc::kSecure),
    },
}};

/* SCTLR_EL2, System Control Register (EL2) */
constexpr uint64_t kSctlrEl2LSMAOE = BIT64(29);
constexpr uint64_t kSctlrEl2nTLSMD = BIT64(28);
constexpr uint64_t kSctlrEl2SPAN = BIT64(23);
constexpr uint64_t kSctlrEl2EIS = BIT64(22);
constexpr uint64_t kSctlrEl2WFE_TRAP = BIT64(18);
constexpr uint64_t kSctlrEl2WfiTrap = BIT64(16);
constexpr uint64_t kSctlrEl2ICache = BIT64(12);
constexpr uint64_t kSctlrEl2Eos = BIT64(11);
constexpr uint64_t kSctlrEl2SctlrEl1 = BIT64(5);
constexpr uint64_t kSctlrEl2SaEl1 = BIT64(4);
constexpr uint64_t kSctlrEl2SaEl2 = BIT64(3);
constexpr uint64_t kSctlrEl2InitVal =
    kSctlrEl2LSMAOE | kSctlrEl2nTLSMD | kSctlrEl2SPAN | kSctlrEl2EIS |
    kSctlrEl2WFE_TRAP | kSctlrEl2WfiTrap | kSctlrEl2ICache | kSctlrEl2Eos |
    kSctlrEl2SctlrEl1 | kSctlrEl2SaEl1 | kSctlrEl2SaEl2;

/* HCR_EL2, Hypervisor Configuration Register */
[[maybe_unused]] constexpr uint64_t kHcrEl2E2H = BIT64(34);
constexpr uint64_t kHcrEl2RW = BIT64(31);
[[maybe_unused]] constexpr uint64_t kHcrEl2TGE = BIT64(27);
constexpr uint64_t kHcrEl2TACR = BIT64(21);
constexpr uint64_t kHcrEl2TID3 = BIT64(18);
constexpr uint64_t kHcrEl2TID2 = BIT64(17);
constexpr uint64_t kHcrEl2TID1 = BIT64(16);
constexpr uint64_t kHcrEl2TWE = BIT64(14);
constexpr uint64_t kHcrEl2TWI = BIT64(13);
constexpr uint64_t kHcrEl2AMO = BIT64(5);
constexpr uint64_t kHcrEl2IMO = BIT64(4);
constexpr uint64_t kHcrEl2FMO = BIT64(3);
constexpr uint64_t kHcrEl2SWIO = BIT64(1);
constexpr uint64_t kHcrEl2VM = BIT64(0);  // Enable Stage-2 MMU
constexpr uint64_t kHcrEl2InitVal =
    (kHcrEl2RW | kHcrEl2TACR | kHcrEl2TID3 | kHcrEl2TID2 | kHcrEl2TID1 |
     kHcrEl2TWE | kHcrEl2TWI | kHcrEl2AMO | kHcrEl2IMO | kHcrEl2FMO |
     kHcrEl2SWIO | kHcrEl2VM);

void InitHypervisorRegisters() {
  WRITE_CPU_REG(sctlr_el2, kSctlrEl2InitVal);
  evisor::Arm64Isb();

  WRITE_CPU_REG(hcr_el2, kHcrEl2InitVal);
}

}  // namespace

void BootFromEl2() {
  // Set up the MMU for EL2
  evisor::Mmu::Disable();
  InitHypervisorRegisters();
#ifdef CONFIG_EARLY_SERIAL_INIT
  evisor::Serial::Get().Init();
#endif
  evisor::Mmu::Get().Init(kMmuKernelRegions, kMmuGuestRegions);

  // Set up the interrupt controller for EL2
  {
    evisor::CpuInitIrqVectorTable();
    evisor::CpuRouteIrqEl2();
    evisor::Arm64Isb();
    evisor::Arm64DsbAllCore();
    evisor::GicV2::Get().Init();
  }

  // Set up the timer for EL2
  evisor::ArmGenericTimer::Get().Init();
}
