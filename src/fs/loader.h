#ifndef EVISOR_FS_LOADER_H_
#define EVISOR_FS_LOADER_H_

#include <cstdbool>
#include <cstdint>

namespace evisor {

struct LoaderVcpuConfig {
  const char* filename;   // Guest OS binary filename
  uint64_t file_load_va;  // load address (virtual address) of the file
  uint64_t pc;            // Entry Point
  uint64_t sp;            // Stack Pointer
};

// Load VCPU with an user specified binary file and config
bool LoaderLoadVcpu(void* config, uint64_t* pc, uint64_t* sp)
    __attribute__((visibility("hidden")));

}  // namespace evisor

#endif  // EVISOR_FS_LOADER_H_
