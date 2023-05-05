#include "common/cstring/memset.h"

#include <cstdint>

void* memset(void* buf, int ch, size_t n) {
  auto bbuf = static_cast<uint8_t*>(buf);
  for (size_t i = 0; i < n; i++) {
    *bbuf++ = static_cast<uint8_t>(ch);
  }
  return bbuf;
}
