#include "platforms/serial.h"

#include "arch/arm64/irq/gic_v2.h"
#include "common/cctype.h"
#include "common/logger.h"
#include "kernel/sched/sched.h"
#include "platforms/platform.h"

namespace evisor {

namespace {
constexpr char kHypervisorCommandStart = '?';
constexpr char kHypervisorCommandShowTaskList = 'l';
constexpr char kHypervisorCommandSwitchTaskConsole = 's';
}  // namespace

Serial::~Serial() {
  uart_.DisableReceiveIrq();
  uart_.Disable();
}

void Serial::Init() {
  uart_.Init(UART0_BASE);
  uart_.EnableReceiveIrq([](uint8_t c) {
    static bool hypervisor_command_comming = false;
    static bool hypervisor_command_switch_req = false;
    auto& sched = Sched::Get();

    if (hypervisor_command_comming) {
      if (hypervisor_command_switch_req) {
        if (isdigit(c)) {
          auto pid = c - '0';
          auto tsk = sched.GetTask(pid);
          if (tsk) {
            sched.ConsoleSwitchTo(pid);
            LOG_INFO("Console is assigned to %s (PID: %d)", tsk->name, pid);
            if (tsk->state == RUNNING) {
              sched.FlushConsole(tsk);
            }
          } else {
            LOG_ERROR("PID %d is invalid or not running.", pid);
          }
        }
        hypervisor_command_switch_req = false;
        hypervisor_command_comming = false;
      } else if (c == kHypervisorCommandSwitchTaskConsole) {
        hypervisor_command_switch_req = true;
      } else if (c == kHypervisorCommandShowTaskList) {
        sched.PrintTasks();
        hypervisor_command_comming = false;
      } else {
        // do nothing
      }
    } else if (c == kHypervisorCommandStart) {
      hypervisor_command_comming = true;
    } else {
      auto console_forwarded_pid = sched.GetCurrentPidUsingConsole();
      auto* tsk = sched.GetTask(console_forwarded_pid);
      if (tsk->state == RUNNING) {
        const auto* console = tsk->board->GetConsole();
        console->in->Push(c);
      }
      // TODO: fix IRQ magic number
      GicV2::Get().NotifyVirqHardware(33);
    }
  });

  // Set baudrate 115200Hz
  uart_.Enable(UART_REFERENCE_CLOCK, 115200);
}

size_t Serial::Send(uint8_t* buf, size_t size) {
  return uart_.Write(buf, size);
}

}  // namespace evisor
