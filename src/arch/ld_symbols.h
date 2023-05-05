#ifndef EVISOR_ARCH_LD_SYMBOLS_H_
#define EVISOR_ARCH_LD_SYMBOLS_H_

#include <cstdint>

// See src/arch/*/boot/*/linker.ld
extern uint8_t __text_start[] __attribute__((visibility("hidden")));
extern uint8_t __text_end[] __attribute__((visibility("hidden")));
extern uint8_t __text_size[] __attribute__((visibility("hidden")));

extern uint8_t __rodata_start[] __attribute__((visibility("hidden")));
extern uint8_t __rodata_end[] __attribute__((visibility("hidden")));
extern uint8_t __rodata_size[] __attribute__((visibility("hidden")));

extern uint8_t __data_start[] __attribute__((visibility("hidden")));
extern uint8_t __data_end[] __attribute__((visibility("hidden")));
extern uint8_t __data_size[] __attribute__((visibility("hidden")));

extern uint8_t __bss_start[] __attribute__((visibility("hidden")));
extern uint8_t __bss_end[] __attribute__((visibility("hidden")));
extern uint8_t __bss_size[] __attribute__((visibility("hidden")));

extern uint8_t __heap_start[] __attribute__((visibility("hidden")));
extern uint8_t __heap_end[] __attribute__((visibility("hidden")));
extern uint8_t __heap_size[] __attribute__((visibility("hidden")));

extern uint8_t __uncached_space_start[] __attribute__((visibility("hidden")));
extern uint8_t __uncached_space_end[] __attribute__((visibility("hidden")));
extern uint8_t __uncached_space_size[] __attribute__((visibility("hidden")));

extern uint8_t __user_space_start[] __attribute__((visibility("hidden")));
extern uint8_t __user_space_end[] __attribute__((visibility("hidden")));
extern uint8_t __user_space_size[] __attribute__((visibility("hidden")));

extern uint8_t __stack_start[] __attribute__((visibility("hidden")));
extern uint8_t __stack_end[] __attribute__((visibility("hidden")));
extern uint8_t __stack_size[] __attribute__((visibility("hidden")));

namespace {
const uint64_t kTextStart = reinterpret_cast<uint64_t>(__text_start);
const uint64_t kTextEnd = reinterpret_cast<uint64_t>(__text_end);
const uint64_t kTextSize = reinterpret_cast<uint64_t>(__text_size);

const uint64_t kRoDataStart = reinterpret_cast<uint64_t>(__rodata_start);
const uint64_t kRoDataEnd = reinterpret_cast<uint64_t>(__rodata_end);
const uint64_t kRoDataSize = reinterpret_cast<uint64_t>(__rodata_size);

const uint64_t kDataStart = reinterpret_cast<uint64_t>(__data_start);
const uint64_t kDataEnd = reinterpret_cast<uint64_t>(__data_end);
const uint64_t kDataSize = reinterpret_cast<uint64_t>(__data_size);

const uint64_t kBssStart = reinterpret_cast<uint64_t>(__bss_start);
const uint64_t kBssEnd = reinterpret_cast<uint64_t>(__bss_end);
const uint64_t kBssSize = reinterpret_cast<uint64_t>(__bss_size);

const uint64_t kHeapStart = reinterpret_cast<uint64_t>(__heap_start);
const uint64_t kHeapEnd = reinterpret_cast<uint64_t>(__heap_end);
const uint64_t kHeapSize = reinterpret_cast<uint64_t>(__heap_size);

const uint64_t kUncachedStart =
    reinterpret_cast<uint64_t>(__uncached_space_start);
const uint64_t kUncachedEnd = reinterpret_cast<uint64_t>(__uncached_space_end);
const uint64_t kUncachedSize =
    reinterpret_cast<uint64_t>(__uncached_space_size);

const uint64_t kUserStart = reinterpret_cast<uint64_t>(__user_space_start);
const uint64_t kUserEnd = reinterpret_cast<uint64_t>(__user_space_end);
const uint64_t kUserSize = reinterpret_cast<uint64_t>(__user_space_size);

const uint64_t kStackStart = reinterpret_cast<uint64_t>(__stack_start);
const uint64_t kStackEnd = reinterpret_cast<uint64_t>(__stack_end);
const uint64_t kStackSize = reinterpret_cast<uint64_t>(__stack_size);
}  // namespace

#endif  // EVISOR_ARCH_LD_SYMBOLS_H_
