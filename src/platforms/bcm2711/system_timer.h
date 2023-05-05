#ifndef EVISOR_PLATFORMS_BCM2711_SYSTEM_TIMER_H_
#define EVISOR_PLATFORMS_BCM2711_SYSTEM_TIMER_H_

#include <cstdint>

namespace evisor {

class SystemTimer {
 public:
  SystemTimer();
  ~SystemTimer() = default;

  // Prevent copying.
  SystemTimer(SystemTimer const&) = delete;
  SystemTimer& operator=(SystemTimer const&) = delete;

  static SystemTimer& Get() noexcept {
    static SystemTimer instance;
    return instance;
  }

  void Init();
  void Start(uint32_t interval_us);
  void Stop();
  void HandleIrq();
  uint32_t GetTime();

 private:
  struct SystemTimerRegs {
    volatile uint32_t CS;   // 0x00
    volatile uint32_t CLO;  // 0x04
    volatile uint32_t CHI;  // 0x08
    volatile uint32_t C0;   // 0x0c
    volatile uint32_t C1;   // 0x10
    volatile uint32_t C2;   // 0x14
    volatile uint32_t C3;   // 0x18
  } __attribute__((packed)) __attribute__((aligned(4)));

  volatile SystemTimerRegs* regs_;
  volatile uint32_t next_counter_value_ = 0;
  volatile uint32_t interval_us_ = 0;
};

}  // namespace evisor

#endif  // EVISOR_PLATFORMS_BCM2711_SYSTEM_TIMER_H_
