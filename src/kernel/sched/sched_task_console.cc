#include "common/logger.h"
#include "kernel/sched/sched.h"
#include "platforms/board.h"

namespace evisor {

void Sched::ConsoleSwitchTo(uint8_t pid) { console_forwarded_pid_ = pid; }

uint8_t Sched::GetCurrentPidUsingConsole() { return console_forwarded_pid_; }

void Sched::FlushConsole(Tcb* tsk) {
  const auto* console = tsk->board->GetConsole();
  while (!console->out->Empty()) {
    auto data = console->out->Pop();
    printf("%c", data & 0xff);
  }
}

}  // namespace evisor
