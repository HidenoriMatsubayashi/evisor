#ifndef EVISOR_ARCH_ARM64_ARM_GENERIC_TIMER_H_
#define EVISOR_ARCH_ARM64_ARM_GENERIC_TIMER_H_

#include <cstdint>

namespace evisor {

class ArmGenericTimer {
 public:
  ArmGenericTimer() = default;
  ~ArmGenericTimer() = default;

  // Prevent copying.
  ArmGenericTimer(ArmGenericTimer const&) = delete;
  ArmGenericTimer& operator=(ArmGenericTimer const&) = delete;

  static ArmGenericTimer& Get() noexcept {
    static ArmGenericTimer instance;
    return instance;
  }

  void Init();
  void Start(uint32_t interval_us);
  void Stop();
  void HandleIrq();
  uint64_t GetTimerCount();
  uint8_t GetIStatus();

 private:
  inline void Enable(bool enable);
  inline void SetCompare(uint64_t val);
  inline void SetIrqMask(bool mask);
  inline uint32_t GetCntfrq();

  volatile uint64_t next_counter_value_ = 0;
  volatile uint64_t interval_us_ = 0;
};

}  // namespace evisor

#endif  // EVISOR_ARCH_ARM64_ARM_GENERIC_TIMER_H_
