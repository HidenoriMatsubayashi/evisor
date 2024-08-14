// Reference:
// https://github.com/raspberrypi/firmware/wiki/Mailbox-property-interface#clocks

#include "drivers/mailbox/mailbox_bcm2711.h"

#include "common/cstring.h"
#include "common/logger.h"
#include "mm/uncached/kmm_uncached_malloc.h"
#include "mm/uncached/kmm_uncached_zalloc.h"

namespace evisor {

namespace {

struct MailboxProperty {
  uint32_t size;
  uint32_t code;
  uint8_t tags[0];
};

constexpr uint32_t kStatus0MessageEmpty = 0x4000'0000;
constexpr uint32_t kStatus0MessageFull = 0x8000'0000;

/// Channels 8 and 9 are used:
/// Channel 8: Request from ARM for response by VC
/// Channel 9: Request from VC for response by ARM (none currently defined)

// Mailbox Channel 0: Power Management Interface
[[maybe_unused]] constexpr uint8_t kChannelPower = 0;
// Mailbox Channel 1: Frame Buffer
[[maybe_unused]] constexpr uint8_t kChannelFb = 1;
// Mailbox Channel 2: Virtual UART
[[maybe_unused]] constexpr uint8_t kChannelUart = 2;
// Mailbox Channel 3: VCHIQ Interface
[[maybe_unused]] constexpr uint8_t kChannelVChiq = 3;
// Mailbox Channel 4: LEDs Interface
[[maybe_unused]] constexpr uint8_t kChannelLeds = 4;
// Mailbox Channel 5: Buttons Interface
[[maybe_unused]] constexpr uint8_t kChannelButtons = 5;
// Mailbox Channel 6: Touchscreen Interface
[[maybe_unused]] constexpr uint8_t kChannelTouch = 6;
// Mailbox Channel 7: Counter
[[maybe_unused]] constexpr uint8_t kChannelCount = 7;
// Mailbox Channel 8: Tags (ARM to VC)
constexpr uint8_t kChannelTags = 8;

enum class RequestResponseCode : uint32_t {
  REQUEST_PROCESS_REQUEST = 0,
  RESPONSE_SUCCESS = 0x80000000,
  RESPONSE_ERROR = 0x80000001,
};

constexpr uint32_t kTagIdEndOfProperty = 0;

}  // namespace

MailboxBcm2711::MailboxBcm2711() {
  regs_ = reinterpret_cast<MailboxRegs*>(MAILBOX_BASE);
}

bool MailboxBcm2711::Process(void* tag, uint32_t size) {
  auto* property_data =
      reinterpret_cast<uint32_t*>(kmm_uncached_zalloc(8 * 1024));
  {
    memcpy(&property_data[2], tag, size);

    auto* buff = reinterpret_cast<MailboxProperty*>(property_data);
    const int buffer_size = size + 12;
    buff->size = buffer_size;
    buff->code =
        static_cast<uint32_t>(RequestResponseCode::REQUEST_PROCESS_REQUEST);
    property_data[buffer_size / 4 - 1] = kTagIdEndOfProperty;

    Write(kChannelTags,
          static_cast<uint32_t>(reinterpret_cast<uintptr_t>(property_data)));
    Read(kChannelTags);

    memcpy(tag, property_data + 2, size);
  }
  kmm_uncached_free(property_data);

  return true;
}

void MailboxBcm2711::Write(uint8_t channel, uint32_t data) {
  while (regs_->Status0 & kStatus0MessageFull) {
    ;
  }

  // The mailbox interface has 28 bits (MSB) available for the value and 4 bits
  // (LSB) for the channel:
  // Request message: 28 bits (MSB) buffer address
  // Response message: 28 bits (MSB) buffer address
  regs_->Write1 = ((data & 0xffff'fff0) | (channel & 0xf));
}

uint32_t MailboxBcm2711::Read(uint8_t channel) {
  while (true) {
    while (regs_->Status0 & kStatus0MessageEmpty) {
      ;
    }

    auto data = regs_->Read0;
    uint8_t read_channel = data & 0xf;
    if (read_channel == channel) {
      return data & 0xffff'fff0;
    }
  }
}

}  // namespace evisor
