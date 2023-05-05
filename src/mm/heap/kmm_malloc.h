#ifndef EVISOR_MM_HEAP_KMM_MALLOC_H_
#define EVISOR_MM_HEAP_KMM_MALLOC_H_

#include <cstddef>

namespace evisor {

void* kmm_malloc(size_t size);
void kmm_free(void* va);

}  // namespace evisor

#endif  // EVISOR_MM_HEAP_KMM_MALLOC_H_
