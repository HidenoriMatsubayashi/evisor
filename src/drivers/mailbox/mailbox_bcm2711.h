#ifndef EVISOR_DRIVERS_MAILBOX_BCM2711_H_
#define EVISOR_DRIVERS_MAILBOX_BCM2711_H_

#include <cstdbool>
#include <cstdint>

#include "drivers/common.h"
#include "drivers/mailbox/mailbox.h"
#include "platforms/platform.h"

namespace evisor {

class MailboxBcm2711 : public Mailbox {
 public:
  struct Tag {
    uint32_t id;
    uint32_t buff_size;
    uint32_t value_len;
  };

  // Unique clock IDs
  enum class ClockId : int {
    EMMC = 1,
    UART = 2,
    ARM = 3,
    CORE = 4,
    V3D = 5,
    H264 = 6,
    ISP = 7,
    SDRAM = 8,
    PIXEL = 9,
    PWM = 10,
    HEVC = 11,
    EMMC2 = 12,
    M2MC = 13,
    PIXEL_BVB = 14,
  };

  MailboxBcm2711();
  ~MailboxBcm2711() = default;

  // Prevent copying.
  MailboxBcm2711(MailboxBcm2711 const&) = delete;
  MailboxBcm2711& operator=(MailboxBcm2711 const&) = delete;

  static MailboxBcm2711& Get() {
    static MailboxBcm2711 instance;
    return instance;
  }

  bool Process(void* tag, uint32_t size) override;

 private:
  struct MailboxRegs {
    reg32_t Read0;       // 0x00         Read data from VC to ARM
    reg32_t Unused[3];   // 0x04-0x0F
    reg32_t Peek0;       // 0x10
    reg32_t Sender0;     // 0x14
    reg32_t Status0;     // 0x18         Status of VC to ARM
    reg32_t Config0;     // 0x1C
    reg32_t Write1;      // 0x20         Write data from ARM to VC
    reg32_t Unused2[3];  // 0x24-0x2F
    reg32_t Peek1;       // 0x30
    reg32_t Sender1;     // 0x34
    uint32_t Status1;    // 0x38         Status of ARM to VC
    uint32_t Config1;    // 0x3C
  } __attribute__((packed)) __attribute__((aligned(4)));

  void Write(uint8_t channel, uint32_t data);
  uint32_t Read(uint8_t channel);

  MailboxRegs* regs_;
};

}  // namespace evisor

#endif  // EVISOR_DRIVERS_MAILBOX_BCM2711_H_
