#include <cstdint>

#include "common/cstring/memcpy.h"

namespace evisor {

int isdigit(int c) {
  return (c >= '0' && c <= '9');
}

}  // namespace evisor
