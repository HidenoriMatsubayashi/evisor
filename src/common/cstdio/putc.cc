#include "common/cstdio/putc.h"

#include "platforms/serial.h"

namespace evisor {

size_t putc(uint8_t c) {
  auto& serial = Serial::Get();

  if (c == '\n') {
    uint8_t cc = '\r';
    serial.Send(&cc, 1);
  }
  return serial.Send(&c, 1);
}

}  // namespace evisor
