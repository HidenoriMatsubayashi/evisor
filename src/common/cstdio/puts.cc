#include "common/cstdio/puts.h"

#include "common/cstdio/putc.h"
#include "common/cstring/strlen.h"

namespace evisor {

size_t puts(const char* s) {
  auto size = strlen(s);
  while (size--) {
    putc(*(s++));
  }
  return size;
}

}  // namespace evisor
