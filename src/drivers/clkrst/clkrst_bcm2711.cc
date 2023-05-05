#include "drivers/clkrst/clkrst_bcm2711.h"

#include "common/logger.h"
#include "drivers/mailbox/mailbox_bcm2711.h"

namespace evisor {

namespace {

struct MailboxGetClockRate {
  MailboxBcm2711::Tag tag;
  uint32_t tag_id;
  uint32_t rate;
};

constexpr uint32_t kTagIdGetClockRate = 0x00030002;

}  // namespace

uint32_t ClkrstBcm2711::GetClockRate(HwBlock hw) {
  auto& mailbox = MailboxBcm2711::Get();

  MailboxGetClockRate d;
  d.tag.id = kTagIdGetClockRate;
  d.tag.buff_size = sizeof(d) - sizeof(d.tag);
  d.tag.value_len = 0;
  switch (hw) {
    case HwBlock::CPU:
      d.tag_id = static_cast<uint32_t>(MailboxBcm2711::ClockId::ARM);
      break;
    case HwBlock::DRAM:
      d.tag_id = static_cast<uint32_t>(MailboxBcm2711::ClockId::SDRAM);
      break;
    case HwBlock::GPU:
      d.tag_id = static_cast<uint32_t>(MailboxBcm2711::ClockId::V3D);
      break;
    case HwBlock::EMMC:
      d.tag_id = static_cast<uint32_t>(MailboxBcm2711::ClockId::EMMC);
      break;
    default:
      LOG_ERROR("Unexpected tag id (%d) came", hw);
      return 0;
  }
  mailbox.Process(&d, sizeof(d));
  return d.rate;
}

}  // namespace evisor
