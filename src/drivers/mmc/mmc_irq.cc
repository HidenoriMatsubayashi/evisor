#include "drivers/mmc/mmc.h"

namespace evisor {

void Mmc::SetupHostIrqs() {
  // disable interrupts
  regs_->IRPT_EN = 0;
  // mask all interrupts
  regs_->IRPT_MASK = 0xFFFFFFFF;
  // clear all interrupts
  regs_->INTERRUPT = 0xFFFFFFFF;
}

}  // namespace evisor
