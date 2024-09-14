#include "common/cstring/strlen.h"

namespace evisor {

size_t strlen(const char* s) {
  size_t count = 0;
  while (*s != '\0') {
    count++;
    s++;
  }
  return count;
}

}  // namespace evisor
