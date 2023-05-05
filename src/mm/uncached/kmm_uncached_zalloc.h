#ifndef EVISOR_MM_HEAP_KMM_UNCACHED_ZALLOC_H_
#define EVISOR_MM_HEAP_KMM_UNCACHED_ZALLOC_H_

#include <cstddef>

namespace evisor {

void* kmm_uncached_zalloc(size_t size);

}  // namespace evisor

#endif  // EVISOR_MM_HEAP_KMM_UNCACHED_ZALLOC_H_
