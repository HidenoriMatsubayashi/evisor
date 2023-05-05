#include "mm/uncached/kmm_uncached_zalloc.h"

#include "common/cstring.h"
#include "mm/uncached/kmm_uncached_malloc.h"

namespace evisor {

void* kmm_uncached_zalloc(size_t size) {
  void* alloc = kmm_uncached_malloc(size);
  if (alloc) {
    memzero(alloc, size);
  }
  return alloc;
}

}  // namespace evisor
