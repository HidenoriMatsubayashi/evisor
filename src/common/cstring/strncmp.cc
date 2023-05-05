#include "common/cstring/memset.h"

namespace evisor {

int strncmp(const char* s1, const char* s2, size_t n) {
  size_t i;
  for (i = 0; i < n && *s1 && (*s1 == *s2); i++, s1++, s2++) {
    ;
  }
  return (i != n) ? (*s1 - *s2) : 0;
}

}  // namespace evisor
