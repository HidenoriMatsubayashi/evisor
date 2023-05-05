#ifndef EVISOR_COMMON_CSTRING_MEMSET_H_
#define EVISOR_COMMON_CSTRING_MEMSET_H_

#include <cstddef>

#ifdef __cplusplus
extern "C" {
#endif

void* memset(void* buf, int ch, size_t n);

#ifdef __cplusplus
}
#endif

#endif  // EVISOR_COMMON_CSTRING_MEMSET_H_
