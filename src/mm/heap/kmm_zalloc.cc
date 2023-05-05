#include "mm/heap/kmm_zalloc.h"

#include "common/cstring.h"
#include "mm/heap/kmm_malloc.h"

namespace evisor {

void* kmm_zalloc(size_t size) {
  void* alloc = evisor::kmm_malloc(size);
  if (alloc) {
    memzero(alloc, size);
  }
  return alloc;
}

}  // namespace evisor
