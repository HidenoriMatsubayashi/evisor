#include "common/logger.h"
#include "drivers/common.h"
#include "drivers/mmc/mmc.h"
#include "drivers/mmc/mmc_internal.h"
#include "platforms/timer.h"

namespace evisor {

namespace {

// Set the clock dividers to generate a target value
// The EMMC module contains its own internal clock divider to generate the
// card’s clock from clk_emmc
uint32_t GetClockDivider(uint32_t base_clock, uint32_t target_clock) {
  uint32_t target_div = 1;
  if (target_clock <= base_clock) {
    target_div = base_clock / target_clock;
    if (base_clock % target_clock) {
      target_div = 0;
    }
  }

  // Find the first bit set
  int div = -1;
  for (int first_bit = 31; first_bit >= 0; first_bit--) {
    uint32_t bt = (1 << first_bit);
    if (target_div & bt) {
      div = first_bit;
      target_div &= ~(bt);
      if (target_div) {
        div++;
      }
      break;
    }
  }

  if (div == -1) {
    div = 31;
  }
  if (div >= 32) {
    div = 31;
  }

  if (div != 0) {
    div = (1 << (div - 1));
  }
  if (div >= 0x400) {
    div = 0x3ff;
  }

  const uint32_t clk_freq8 = div & 0xff;
  const uint32_t clk_freq_ms2 = (div >> 8) & 0x3;
  return (clk_freq8 << 8) | (clk_freq_ms2 << 6);
}

}  // namespace

bool Mmc::ChangeClockRate(uint32_t base_clock, uint32_t target_rate) {
  uint32_t divider = GetClockDivider(base_clock, target_rate);

  while (
      (regs_->STATUS & (EMMC_STATUS_CMD_INHIBIT | EMMC_STATUS_DAT_INHIBIT))) {
    Timer::Sleep(1);
  }

  uint32_t c1 = regs_->CONTROL1 & ~EMMC_CTRL1_CLK_EN;
  regs_->CONTROL1 = c1;
  Timer::Sleep(3);
  regs_->CONTROL1 = (c1 | divider) & ~0xFFE0;
  Timer::Sleep(3);
  regs_->CONTROL1 = c1 | EMMC_CTRL1_CLK_EN;
  Timer::Sleep(3);

  return true;
}

bool Mmc::SetupInitialClockRate() {
  regs_->CONTROL2 = 0;

  {
    uint32_t n = regs_->CONTROL1;

    // Clock enable for internal EMMC clocks for power saving
    n |= EMMC_CTRL1_CLK_INTLEN;

    // Set to identification frequency
    n |= GetClockDivider(device_.base_clock, SD_CLOCK_HIGH);

    // was not masked out and or'd with (7 << 16) in original driver
    n &= ~(0xf << 16);
    n |= (11 << 16);  // data timeout = TMCLK * 2^24

    regs_->CONTROL1 = n;
  }

  if (!RegWaitBitSet(&regs_->CONTROL1, EMMC_CTRL1_CLK_STABLE, 1000)) {
    LOG_ERROR("Clock didn't stabilise within 1 second");
    return false;
  }

  // Enable clock
  Timer::Sleep(2);
  regs_->CONTROL1 = regs_->CONTROL1 | EMMC_CTRL1_CLK_EN;
  Timer::Sleep(2);

  return true;
}

}  // namespace evisor
