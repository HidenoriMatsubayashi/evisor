#ifndef EVISOR_MM_NEW_H_
#define EVISOR_MM_NEW_H_

#include <cstdint>
#include <cstdlib>

void* operator new(std::size_t n);
void operator delete(void* p);
void operator delete(void* p, std::size_t);

#endif  // EVISOR_MM_NEW_H_
