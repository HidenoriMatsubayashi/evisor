#ifndef EVISOR_PLATFORMS_TIMER_H_
#define EVISOR_PLATFORMS_TIMER_H_

#include <cstdint>
#include <functional>

namespace evisor {

class Timer {
 public:
  using Handler = std::function<void()>;

  Timer();
  ~Timer();

  void Start(uint32_t interval_us, Handler handler);
  void Stop();
  static void Sleep(uint32_t msec);
  static void SleepUsec(uint32_t usec);
  static uint32_t GetSystemUsec();

 private:
  void HandleIrq();

  Handler handler_ = nullptr;
};

}  // namespace evisor

#endif  // EVISOR_PLATFORMS_TIMER_H_
