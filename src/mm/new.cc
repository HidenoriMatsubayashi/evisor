#include "mm/new.h"

#include "common/assert.h"
#include "mm/heap/kmm_malloc.h"

void* operator new(std::size_t n) {
  void* p = evisor::kmm_malloc(n);
  ASSERT(p != nullptr, "kmm_malloc failed");
  return p;
}

void operator delete(void* p) {
  if (!p) {
    return;
  }
  evisor::kmm_free(p);
}

void operator delete(void* p, std::size_t) {
  if (!p) {
    return;
  }
  evisor::kmm_free(p);
}
