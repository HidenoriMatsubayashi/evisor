#ifndef EVISOR_ARCH_ARM64_MMU_H_
#define EVISOR_ARCH_ARM64_MMU_H_

#include <array>
#include <cstdint>

namespace evisor {

class Mmu {
 public:
  // MAIR_EL2 memory attributes
  enum PageMemoryAttribute {
    kDevicenGnRnE = 0,
    kDevicenGnRE = 1,
    kDeviceGRE = 2,
    kNormalNC = 3,
    kNormal = 4,
    kNormalWT = 5,
  };

  /* Page table entry descriptor for L3
   * attrs[3] : Access Permissions
   * attrs[4] : Memory access from secure/ns state
   * attrs[5] : Execute Permissions
   */
  enum PageTableEntryL3Desc {
    kRO = 0 << 3,
    kRW = 1 << 3,
    kSecure = 0 << 4,
    kNonSecure = 1 << 4,
    kExecute = 0 << 5,
    kExecuteNever = 1 << 5,
  };

  struct MmuMapRegion {
    // Region Name
    const char* name;
    // Region Base Physical Address
    uint64_t base_pa;
    // Region Base Virtual Address
    uint64_t base_va;
    // Region size
    uint64_t size;
    // Region Attributes
    uint64_t attrs;
  };

  Mmu() = default;
  ~Mmu() = default;

  // Prevent copying.
  Mmu(Mmu const&) = delete;
  Mmu& operator=(Mmu const&) = delete;

  static Mmu& Get() noexcept {
    static Mmu instance;
    return instance;
  }

  void Init(const std::array<MmuMapRegion, 7>& kernel_regions,
            const std::array<MmuMapRegion, 2>& guest_regions);

  // Enable MMU
  static void Enable();

  // Disable MMU
  static void Disable();

  static void SetStage2PageTable(uint64_t table, uint64_t pid);

  static uint64_t TranslateEl1IpaToEl2Va(uint64_t el1_ipa);

 private:
  std::array<MmuMapRegion, 7> mmu_regions_kernel_;
  std::array<MmuMapRegion, 2> mmu_regions_guest_;

  void CheckMmuConfigs();

  void CreatePageTables();

  // Create/Populate translation table(s) for given region
  void InitXlatTables(const MmuMapRegion& region);

  uint64_t* NewXlatTablesEntry();

  uint64_t* FindPteIndex(uint64_t va, uint32_t level);

  // Get index within given virtual address and translation table level
  inline uint64_t GetVaIndex(uint64_t va, uint32_t level);

  inline uint8_t PteDescType(uint64_t* pte);

  inline void SetPteTableDesc(uint64_t* pte, uint64_t* table);

  void SetPteBlockDesc(uint64_t* pte,
                       uint64_t pa,
                       uint32_t attrs,
                       uint32_t level);

  // Splits a block into table with entries spanning the old block
  void SplitPteBlockDesc(uint64_t* pte, uint32_t level);

  // Translation table control register settings
  uint64_t GetTcr(int el);
};

}  // namespace evisor

#endif  // EVISOR_ARCH_ARM64_MMU_H_
