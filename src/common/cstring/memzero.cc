#include "common/cstring/memzero.h"

#include <cstdint>

namespace evisor {

void memzero(void* buf1, size_t n) {
  auto dest = static_cast<uint8_t*>(buf1);
  for (size_t i = 0; i < n; i++) {
    dest[i] = 0;
  }
}

}  // namespace evisor
