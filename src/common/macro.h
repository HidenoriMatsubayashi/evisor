#ifndef EVISOR_COMMON_MACRO_H_
#define EVISOR_COMMON_MACRO_H_

#define UNUSED(x) ((void)x)

#define BIT32(n) ((1UL) << (n))
#define BIT64(n) ((1ULL) << (n))

#define KB(x) ((x) << 10)
#define MB(x) (KB(x) << 10)
#define GB(x) ((MB(x##ul)) << 10)

#endif  // EVISOR_COMMON_MACRO_H_
