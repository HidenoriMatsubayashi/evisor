#ifndef EVISOR_MM_UNCACHED_KMM_UNCACHED_MALLOC_H_
#define EVISOR_MM_UNCACHED_KMM_UNCACHED_MALLOC_H_

#include <cstddef>

namespace evisor {

// Allocate kernel memory in the uncached memory space.
void* kmm_uncached_malloc(size_t size);
// Free kernel memory in the uncached memory space.
void kmm_uncached_free(void* va);

}  // namespace evisor

#endif  // EVISOR_MM_UNCACHED_KMM_UNCACHED_MALLOC_H_
