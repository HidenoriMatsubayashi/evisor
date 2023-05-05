#include "platforms/bcm2711/system_timer.h"

#include "platforms/bcm2711/peripheral.h"

namespace evisor {

namespace {
constexpr uint32_t kCsM0 = (1 << 0);
constexpr uint32_t kCsM1 = (1 << 1);
constexpr uint32_t kCsM2 = (1 << 2);
constexpr uint32_t kCsM3 = (1 << 3);
}  // namespace

SystemTimer::SystemTimer() {
  Init();
}

void SystemTimer::Init() {
  next_counter_value_ = 0;
  interval_us_ = 0;
  regs_ = reinterpret_cast<SystemTimerRegs*>(SYSTEM_TIMER_BASE);
}

void SystemTimer::Start(uint32_t interval_us) {
  interval_us_ = interval_us;
  next_counter_value_ = regs_->CLO + interval_us;
  regs_->C1 = next_counter_value_;
}

void SystemTimer::Stop() {
  regs_->C1 = 0;
}

void SystemTimer::HandleIrq() {
  next_counter_value_ = next_counter_value_ + interval_us_;
  regs_->C1 = next_counter_value_;
  regs_->CS = regs_->CS | kCsM1;
}

uint32_t SystemTimer::GetTime() {
  return regs_->CLO;
}

}  // namespace evisor
