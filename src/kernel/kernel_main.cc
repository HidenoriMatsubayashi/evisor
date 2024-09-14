#include "common/logger.h"
#include "platforms/platform.h"

// Hypervisor main entry point
extern "C" void KernelMain() {
  auto& serial = evisor::Serial::Get();
  serial.Init();

  LOG_INFO("Hello, eVisor.");
  while (true) {
    ;
  }
}
