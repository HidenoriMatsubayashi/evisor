#ifndef EVISOR_PLATFORMS_BOARD_H_
#define EVISOR_PLATFORMS_BOARD_H_

#include <cstdint>

#include "common/macro.h"
#include "common/queue.h"
#include "kernel/task/task.h"

namespace evisor {

class Board {
 public:
  struct Console {
    Queue* in;
    Queue* out;
  };

  Board() = default;
  ~Board() = default;

  // Prevent copying.
  Board(Board const&) = delete;
  Board& operator=(Board const&) = delete;

  virtual void Init(Tcb* tsk) {
    UNUSED(tsk);
    console_ = new Console();
    console_->in = new Queue();
    console_->out = new Queue();
  };

  const Console* GetConsole() { return console_; }

  virtual uint64_t MmioRead(Tcb* tsk, uint64_t addr) = 0;

  virtual void MmioWrite(Tcb* tsk, uint64_t addr, uint64_t val) = 0;

  void VmEnter(Tcb* tsk) {
    if (!console_->in->Empty()) {
      // TODO: Check if GIC corresponding IRQ is enabled
      tsk->stat.irq_pending = true;
    } else {
      tsk->stat.irq_pending = false;
    }
  }

  void VmLeave(Tcb* tsk) { UNUSED(tsk); }

  int IsIrqAsserted(Tcb* tsk) { return tsk->stat.irq_pending; }

  int IsFiqAsserted(Tcb* tsk) { return tsk->stat.fiq_pending; }

  void debug(Tcb* tsk) { UNUSED(tsk); }

 private:
  Console* console_ = nullptr;
};

}  // namespace evisor

#endif  // EVISOR_PLATFORMS_BOARD_H_
