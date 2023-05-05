#ifndef EVISOR_PLATFORMS_SERIAL_H_
#define EVISOR_PLATFORMS_SERIAL_H_

#include <cstddef>
#include <cstdint>

#include "platforms/common/pl011_uart.h"

namespace evisor {

class Serial {
 public:
  Serial() = default;
  ~Serial();

  // Prevent copying.
  Serial(Serial const&) = delete;
  Serial& operator=(Serial const&) = delete;

  static Serial& Get() {
    static Serial instance;
    return instance;
  }

  void Init();
  size_t Send(uint8_t* buf, size_t size);

 private:
  Pl011Uart uart_;
};

}  // namespace evisor

#endif  // EVISOR_PLATFORMS_SERIAL_H_
