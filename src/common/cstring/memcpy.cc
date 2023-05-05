#include "common/cstring/memcpy.h"

#include <cstdint>

void memcpy(void* buf1, const void* buf2, size_t n) {
  auto dest = static_cast<uint8_t*>(buf1);
  auto src = static_cast<const uint8_t*>(buf2);
  for (size_t i = 0; i < n; i++) {
    dest[i] = src[i];
  }
}
