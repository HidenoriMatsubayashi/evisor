#ifndef EVISOR_COMMON_LOGGER_H_
#define EVISOR_COMMON_LOGGER_H_

#include "common/cstdio/printf.h"

#if defined(NDEBUG)
#define _LOG_COMMON(level, format, ...)         \
  {                                             \
    evisor::printf("%s: ", (level));            \
    evisor::printf(format "\n", ##__VA_ARGS__); \
  }
#else
#define _LOG_COMMON(level, format, ...)                          \
  {                                                              \
    evisor::printf("%s[%s(%d)]: ", (level), __FILE__, __LINE__); \
    evisor::printf(format "\n", ##__VA_ARGS__);                  \
  }
#endif  // NDEBUG

#define PANIC(format, ...)                           \
  {                                                  \
    _LOG_COMMON("[PANIC]: ", format, ##__VA_ARGS__); \
    while (1)                                        \
      ;                                              \
  }
#define LOG_ERROR(format, ...) \
  _LOG_COMMON("[eVisor][ERROR]", format, ##__VA_ARGS__)
#define LOG_WARN(format, ...) \
  _LOG_COMMON("[eVisor][WARN]", format, ##__VA_ARGS__)
#define LOG_INFO(format, ...) \
  _LOG_COMMON("[eVisor][INFO]", format, ##__VA_ARGS__)
#define LOG_DEBUG(format, ...) \
  _LOG_COMMON("[eVisor][DEBUG]", format, ##__VA_ARGS__)
#define LOG_TRACE(format, ...) \
  _LOG_COMMON("[eVisor][TRACE]", format, ##__VA_ARGS__)

#endif  // EVISOR_COMMON_LOGGER_H_
