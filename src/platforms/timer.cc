#include "platforms/timer.h"

#include "arch/arm64/irq/gic_v2.h"
#include "platforms/bcm2711/system_timer.h"
#include "platforms/platform.h"

namespace evisor {

Timer::Timer() {
  SystemTimer::Get().Init();

  const uint8_t interrupt_id = 0x61;
  GicV2::Get().RegisterIrq(interrupt_id, 0, 0xff, [this]() { HandleIrq(); });
}

Timer::~Timer() { Stop(); }

void Timer::Start(uint32_t interval_us, Handler handler) {
  handler_ = handler;
  SystemTimer::Get().Start(interval_us);
}

void Timer::Stop() {
  SystemTimer::Get().Stop();
  handler_ = nullptr;
}

uint32_t Timer::GetSystemUsec() { return SystemTimer::Get().GetTime(); }

void Timer::Sleep(uint32_t msec) {
  uint32_t target_time = GetSystemUsec() + (msec * 1000);
  while (GetSystemUsec() < target_time) {
    /* do nothing */
  }
}

void Timer::SleepUsec(uint32_t usec) {
  uint32_t target_time = GetSystemUsec() + usec;
  while (GetSystemUsec() < target_time) {
    /* do nothing */
  }
}

void Timer::HandleIrq() {
  SystemTimer::Get().HandleIrq();
  if (handler_) {
    handler_();
  }
}

}  // namespace evisor
