#include <array>

#include "arch/arm64/cpu_regs.h"
#include "arch/arm64/irq/cpu_irq.h"
#include "arch/ld_symbols.h"
#include "common/logger.h"
#include "fs/loader.h"
#include "kernel/sched/sched.h"
#include "kernel/task/task.h"
#include "platforms/platform.h"
#include "platforms/platform_config.h"

namespace {

void PrintDebugInfo() {
  LOG_TRACE("Current EL: %d", CpuRegGetCurrentEl());
  LOG_TRACE("sctlr_el2: 0x%08x", READ_CPU_REG(sctlr_el2));
  LOG_TRACE("ttbr0_el2: 0x%08x", READ_CPU_REG(ttbr0_el2));
  LOG_TRACE("hcr_el2: 0x%08x", READ_CPU_REG(hcr_el2));
  LOG_TRACE("elr_el2: 0x%08x", READ_CPU_REG(elr_el2));
  LOG_TRACE("spsr_el2: 0x%08x", READ_CPU_REG(spsr_el2));

  LOG_TRACE("--------------- Memory Map ---------------");
  LOG_TRACE("text:     %08X - %08X (%d KB)", kTextStart, kTextEnd,
            kTextSize / 1000);
  LOG_TRACE("rodata:   %08X - %08X (%d KB)", kRoDataStart, kRoDataEnd,
            kRoDataSize / 1000);
  LOG_TRACE("data:     %08X - %08X (%d KB)", kDataStart, kDataEnd,
            kDataSize / 1000);
  LOG_TRACE("bss:      %08X - %08X (%d KB)", kBssStart, kBssEnd,
            kBssSize / 1000);
  LOG_TRACE("stack:    %08X - %08X (%d KB)", kStackStart, kStackEnd,
            kStackSize / 1000);
  LOG_TRACE("heap:     %08X - %08X (%d KB)", kHeapStart, kHeapEnd,
            kHeapSize / 1000);
  LOG_TRACE("uncached: %08X - %08X (%d KB)", kUncachedStart, kUncachedEnd,
            kUncachedSize / 1000);
  LOG_TRACE("user:     %08X - %08X (%d KB)", kUserStart, kUserEnd,
            kUserSize / 1000);
  LOG_TRACE("------------------------------------------");
}

std::array<evisor::LoaderVcpuConfig, 1> kConfigVCPUs = {{
#if defined(TEST_GUEST_IS_TEST_APP)
    {
        .filename = "test_app.bin",
        .file_load_va = 0,
        .pc = 0,
        .sp = 0x1000,
    },
#elif defined(TEST_GUEST_IS_SERIAL)
    {
        .filename = "serial.bin",
        .file_load_va = 0,
        .pc = 0,
        .sp = 0x10000,
    },
#elif defined(TEST_GUEST_IS_NUTTX)
    {
        .filename = "nuttx.bin",
        .file_load_va = 0x40280000,
        .pc = 0x40280000,
        .sp = 0x41280000,
    },
#else
    // Linux
    {
        .filename = "Image",
        .file_load_va = 0x40000000,
        .pc = 0x40000000,
        .sp = 0x50000000,
    },
#endif
}};

}  // namespace

// Hypervisor main entry point
extern "C" void KernelMain() {
#ifndef CONFIG_EARLY_SERIAL_INIT
  auto& serial = evisor::Serial::Get();
  serial.Init();
#endif

  LOG_INFO("Hello, eVisor.");
  PrintDebugInfo();

  auto& sched = evisor::Sched::Get();
  sched.Init();

  for (auto& vcpu : kConfigVCPUs) {
    if (sched.CreateTask(evisor::LoaderLoadVcpu, &vcpu) < 0) {
      LOG_ERROR("Failed to create %s", vcpu.filename);
    }
  }

  while (true) {
    evisor::CpuDisableIrq();
    sched.Schedule();
    evisor::CpuEnableIrq();
  }
}
