#include "mm/user_heap/umm_zalloc.h"

#include "common/cstring.h"
#include "mm/user_heap/umm_malloc.h"

namespace evisor {

void* umm_zalloc(size_t size) {
  void* alloc = evisor::umm_malloc(size);
  if (alloc) {
    memzero(alloc, size);
  }
  return alloc;
}

}  // namespace evisor
