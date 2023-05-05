#ifndef EVISOR_MM_USER_HEAP_UMM_MALLOC_H_
#define EVISOR_MM_USER_HEAP_UMM_MALLOC_H_

#include <cstddef>

namespace evisor {

void* umm_malloc(size_t size);
void umm_free(void* va);

}  // namespace evisor

#endif  // EVISOR_MM_USER_HEAP_UMM_MALLOC_H_
