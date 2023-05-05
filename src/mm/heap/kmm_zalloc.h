#ifndef EVISOR_MM_HEAP_KMM_ZALLOC_H_
#define EVISOR_MM_HEAP_KMM_ZALLOC_H_

#include <cstddef>

namespace evisor {

extern void* kmm_zalloc(size_t size);

}  // namespace evisor

#endif  // EVISOR_MM_HEAP_KMM_ZALLOC_H_
