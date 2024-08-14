#include "arch/arm64/arm_generic_timer.h"

#include <cstdbool>

#include "arch/arm64/cpu_regs.h"
#include "arch/arm64/irq/gic.h"
#include "common/logger.h"
#include "common/macro.h"

namespace evisor {

namespace {

/* CNTHCTL_EL2 definitions */
[[maybe_unused]] constexpr uint64_t kCnthctlEl2Evntdir = BIT64(3);
[[maybe_unused]] constexpr uint64_t kCnthctlEl2Evnten = BIT64(2);
constexpr uint64_t kCnthctlEl2El1Pcen = BIT64(1);
constexpr uint64_t kCnthctlEl2El1Pcten = BIT64(0);

/* CNTHP_CTL_EL2 definitions */
[[maybe_unused]] constexpr uint64_t kCnthpCtlEl2IStatus = BIT64(2);
constexpr uint64_t kCnthpCtlEl2IMask = BIT64(1);
constexpr uint64_t kCnthpCtlEl2Enable = BIT64(0);

}  // namespace

void ArmGenericTimer::Init() {
  {
    // Enable EL0/EL1 access to timers
    // (Disable traps of timers by EL2)
    uint64_t val = READ_CPU_REG(cnthctl_el2);
    val |= (kCnthctlEl2El1Pcen | kCnthctlEl2El1Pcten);
    WRITE_CPU_REG(cnthctl_el2, val);

    // Set virtual timer offset to 0
    val = 0;
    WRITE_CPU_REG(cntvoff_el2, val);

    // Disable hypervisor physical timer.
    WRITE_CPU_REG(cnthp_ctl_el2, val);

    // Set compare value of hypervisor physical timer to max value
    SetCompare(~(uint64_t)0);
  }

  next_counter_value_ = 0;
  interval_us_ = 0;
}

void ArmGenericTimer::Start(uint32_t interval_us) {
  uint64_t cntfrq = GetCntfrq();

  interval_us_ = interval_us * cntfrq / 1000000L;
  {
    uint64_t cur = READ_CPU_REG(cntpct_el0);
    next_counter_value_ = cur + interval_us_;
    SetCompare(next_counter_value_);
  }
  Enable(true);
  SetIrqMask(false);
}

void ArmGenericTimer::Stop() {
  Enable(false);
  SetIrqMask(true);
  SetCompare(~(uint64_t)0);
}

void ArmGenericTimer::HandleIrq() {
  next_counter_value_ = next_counter_value_ + interval_us_;
  SetCompare(next_counter_value_);
}

uint64_t ArmGenericTimer::GetTimerCount() {
  return READ_CPU_REG(cntpct_el0);
}

uint8_t ArmGenericTimer::GetIStatus() {
  return (READ_CPU_REG(cnthp_ctl_el2) >> 2) & 0x1;
}

inline void ArmGenericTimer::Enable(bool enable) {
  uint64_t val = READ_CPU_REG(cnthp_ctl_el2);
  if (enable) {
    val |= kCnthpCtlEl2Enable;
  } else {
    val &= ~kCnthpCtlEl2Enable;
  }
  WRITE_CPU_REG(cnthp_ctl_el2, val);
}

inline void ArmGenericTimer::SetCompare(uint64_t val) {
  WRITE_CPU_REG(cnthp_cval_el2, val);
}

inline void ArmGenericTimer::SetIrqMask(bool mask) {
  uint64_t val = READ_CPU_REG(cnthp_ctl_el2);
  if (mask) {
    val |= kCnthpCtlEl2IMask;
  } else {
    val &= ~kCnthpCtlEl2IMask;
  }
  WRITE_CPU_REG(cnthp_ctl_el2, val);
}

// Get clock frequency [Hz]
inline uint32_t ArmGenericTimer::GetCntfrq() {
  return READ_CPU_REG(cntfrq_el0);
}

}  // namespace evisor
