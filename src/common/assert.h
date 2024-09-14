#ifndef EVISOR_COMMON_ASSERT_H_
#define EVISOR_COMMON_ASSERT_H_

#include "common/logger.h"

#if defined(NDEBUG)
#define ASSERT(cond, format, ...)
#else
#define ASSERT(cond, format, ...)   \
  {                                 \
    if (!(cond)) {                  \
      PANIC(format, ##__VA_ARGS__); \
    }                               \
  }
#endif  // NDEBUG

#endif  // EVISOR_COMMON_ASSERT_H_
