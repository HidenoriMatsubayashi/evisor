#ifndef EVISOR_MM_MM_LOCAL_H_
#define EVISOR_MM_MM_LOCAL_H_

#include <cstddef>

struct PageBlock {
  size_t size;
  uint8_t ref_count;
};

#endif  // EVISOR_MM_MM_LOCAL_H_
