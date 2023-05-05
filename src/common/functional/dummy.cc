#include "common/logger.h"

namespace std {

// # Fix std::function: undefined reference to `std::__throw_bad_function_call()
void __throw_bad_function_call() { PANIC(); }

}  // namespace std
