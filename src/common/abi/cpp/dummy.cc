#include "common/logger.h"

// Dummy definition to fix "undefined reference to `__cxa_pure_virtual" build failure.
extern "C" void __cxa_pure_virtual()
{
  PANIC();
}
