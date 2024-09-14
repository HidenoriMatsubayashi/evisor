#include "platforms/serial.h"

#include "common/cctype.h"
#include "common/logger.h"
#include "platforms/platform.h"

namespace evisor {

Serial::~Serial() {
  uart_.Disable();
}

void Serial::Init() {
  uart_.Init(UART0_BASE);
  // Set baudrate 115200Hz
  uart_.Enable(UART_REFERENCE_CLOCK, 115200);
}

size_t Serial::Send(uint8_t* buf, size_t size) {
  return uart_.Write(buf, size);
}

}  // namespace evisor
