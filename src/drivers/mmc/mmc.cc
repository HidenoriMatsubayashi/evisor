/// Reference source files:
/// https://github.com/rockytriton/LLD/blob/main/rpi_bm/part17/src/drivers/emmc/emmc.c
/// https://github.com/rsta2/circle/blob/master/addon/SDCard/emmc.cpp

#include "drivers/mmc/mmc.h"

#include "common/logger.h"
#include "drivers/clkrst/clkrst_bcm2711.h"
#include "drivers/mmc/mmc_internal.h"
#include "platforms/gpio.h"
#include "platforms/timer.h"

namespace evisor {

namespace {
constexpr int kRetryMax = 4;
}  // namespace

bool Mmc::Open() {
#ifdef BOARD_IS_RASPI4
  // Pin assignment: GPIO14 to TXD0, GPIO15 to RXD0.
  auto& gpio = GpioBcm2711::Get();
  gpio.SetMode(34, Gpio::GpioMode::kInput);
  gpio.SetMode(35, Gpio::GpioMode::kInput);
  gpio.SetMode(36, Gpio::GpioMode::kInput);
  gpio.SetMode(37, Gpio::GpioMode::kInput);
  gpio.SetMode(38, Gpio::GpioMode::kInput);
  gpio.SetMode(39, Gpio::GpioMode::kInput);

  gpio.SetMode(48, Gpio::GpioMode::kAlt3);
  gpio.SetMode(49, Gpio::GpioMode::kAlt3);
  gpio.SetMode(50, Gpio::GpioMode::kAlt3);
  gpio.SetMode(51, Gpio::GpioMode::kAlt3);
  gpio.SetMode(52, Gpio::GpioMode::kAlt3);
#endif

  device_.sdhc = false;
  device_.ocr = 0;
  device_.rca = 0;
  device_.base_clock = ClkrstBcm2711::Get().GetClockRate(Clkrst::HwBlock::EMMC);
  LOG_DEBUG("EMMC clock rate: %d [KHz]", device_.base_clock / 1000);

  {
    const uint32_t d = regs_->SLOTISR_VER;
    LOG_DEBUG("Vendor version %x, Host Controller version %x, slot status %x",
              (d >> 16) & 0xff, d >> 24, d & 0xff);
  }

  for (auto i = 0; i < kRetryMax; i++) {
    if (ResetCard()) {
      return true;
    }
    LOG_WARN("Failed to reset card, trying again...");
    Timer::Sleep(100);
  }

  return false;
}

bool Mmc::Seek(uint64_t offset) {
  if (offset % device_.block_size != 0) {
    LOG_ERROR("Offset (%d) needs to be %d byte aligned", offset,
              device_.block_size);
    return false;
  }
  device_.seek_offset = offset;
  return true;
}

int Mmc::Read(uint8_t* buf, uint32_t size) {
  uint32_t block = device_.seek_offset / device_.block_size;
  if (!HandleDataCommand(false, buf, size, block)) {
    return -1;
  }
  return size;
}

bool Mmc::HandleDataCommand(bool write, uint8_t* buf, uint32_t buf_size,
                            uint32_t block) {
  if (buf_size < device_.block_size || buf_size % device_.block_size != 0) {
    LOG_ERROR("Bad arguments: buffer size = %d, block size = %d", buf_size,
              device_.block_size);
    return false;
  }

  if (!device_.sdhc) {
    block *= 512;
  }

  CommandData d = {
      .cmd_type = CmdType::INVALID,
      .cmd_reg = kReserved_,
      .cmd_arg = 0,
      .cmd_timeout_usec = 0,
      .transfer_blocks = buf_size / device_.block_size,
      .offset = 0,
      .buffer = buf,
      .cmd_response = {0},
      .cmd_error = 0,
  };

  {
    auto command = CmdType::READ_SINGLE_BLOCK;
    if (write) {
      if (d.transfer_blocks > 1) {
        command = CmdType::WRITE_MULTIPLE_BLOCK;
      } else {
        command = CmdType::WRITE_BLOCK;
      }
    } else {
      if (d.transfer_blocks > 1) {
        command = CmdType::READ_MULTIPLE_BLOCK;
      }
    }
    d.cmd_type = command;
    d.cmd_reg = kCommands_[static_cast<int>(command)];
  }
  d.cmd_arg = block;
  d.cmd_timeout_usec = 5000;

  for (auto i = 0; i < kRetryMax; i++) {
    if (SendCommand(d)) {
      return true;
    }
    LOG_WARN("Retrying data command...");
  }
  LOG_ERROR("Data command failed");
  return false;
}

bool Mmc::AppCommand(CommandData& d) {
  if (d.cmd_reg.index >= 60) {
    LOG_ERROR("%d is the invalid app command", d.cmd_type);
    return false;
  }

  uint32_t rca = 0;
  if (device_.rca) {
    rca = device_.rca << 16;
  }

  auto command = d.cmd_reg;
  auto arg = d.cmd_arg;
  {
    d.cmd_reg = kCommands_[static_cast<int>(CmdType::APP_CMD)];
    d.cmd_arg = rca;
    if (!SendCommand(d)) {
      LOG_ERROR("APP_CMD failed");
      return false;
    }
  }

  d.cmd_reg = command;
  d.cmd_arg = arg;
  return SendCommand(d);
}

bool Mmc::SendCommand(CommandData& d) {
  if (d.transfer_blocks > 0xffff) {
    LOG_ERROR("TransferBlocks too large: %d", d.transfer_blocks);
    return false;
  }

  regs_->BLKSIZECNT = device_.block_size | (d.transfer_blocks << 16);
  regs_->ARG1 = d.cmd_arg;
  {
    reg32_t command_reg = *reinterpret_cast<reg32_t*>(&d.cmd_reg);
    regs_->CMDTM = command_reg;
  }

  // Wait command has finished.
  if (!WaitCommandDone(d)) {
    LOG_ERROR("SendCommand (%d) timed out", d.cmd_type);
    return false;
  }

  if (d.cmd_reg.is_data) {
    PerformDataTransfer(d);
  }

  if (d.cmd_reg.response_type == static_cast<int>(CmdRespType::RT48Busy) ||
      d.cmd_reg.is_data) {
    // Wait Data transfer has finished.
    RegWaitBitSet(&regs_->INTERRUPT, 0x2, 2000);
    uint32_t interrupts = regs_->INTERRUPT;
    regs_->INTERRUPT = 0xffff'0002;
    if ((interrupts & 0xffff'0002) != 2 &&
        (interrupts & 0xffff'0002) != 0x100002) {
      d.cmd_error = interrupts & 0xffff'0000;
      LOG_ERROR("Unexpected irqs: %x, status: %x", interrupts, regs_->STATUS);
      return false;
    }
    regs_->INTERRUPT = 0xffff'0002;
  }

  return true;
}

bool Mmc::WaitCommandDone(CommandData& d) {
  uint32_t times = 0;
  while (times++ < d.cmd_timeout_usec) {
    uint32_t reg = regs_->INTERRUPT;
    if (reg & 0x1) {
      break;
    } else if (reg != 0) {
      LOG_DEBUG("Error waiting for command complete: %d", d.cmd_reg.index);
      break;
    }
    Timer::SleepUsec(1);
  }
  if (times >= d.cmd_timeout_usec) {
    LOG_ERROR("WaitCommandDone timed out");
    return false;
  }

  const uint32_t interrupts = regs_->INTERRUPT;
  regs_->INTERRUPT = 0xffff'0001;
  if ((interrupts & 0xffff'0001) != 1) {
    d.cmd_error = interrupts & 0xffff'0000;
    LOG_ERROR("Unexpected irqs: %x, status: %x", interrupts, regs_->STATUS);
    return false;
  }

  switch (d.cmd_reg.response_type) {
    case static_cast<int>(CmdRespType::RT48):
    case static_cast<int>(CmdRespType::RT48Busy):
      d.cmd_response[0] = regs_->RESP0;
      break;
    case static_cast<int>(CmdRespType::RT136):
      d.cmd_response[0] = regs_->RESP0;
      d.cmd_response[1] = regs_->RESP1;
      d.cmd_response[2] = regs_->RESP2;
      d.cmd_response[3] = regs_->RESP3;
      break;
    default:
      break;
  }

  return true;
}

bool Mmc::PerformDataTransfer(CommandData& d) {
  uint32_t interrupts = 0;
  bool write = false;

  if (d.cmd_reg.direction) {
    interrupts = 1 << 5;
  } else {
    interrupts = 1 << 4;
    write = true;
  }

  auto* buf = static_cast<uint32_t*>(d.buffer);
  for (uint32_t block = 0; block < d.transfer_blocks; block++) {
    RegWaitBitSet(&regs_->INTERRUPT, interrupts, 2000);

    const uint32_t reg_val = regs_->INTERRUPT;
    regs_->INTERRUPT = interrupts;
    if ((reg_val & (0xffff'0000 | interrupts)) != interrupts) {
      d.cmd_error = reg_val & 0xffff'0000;
      LOG_ERROR("Unexpected irqs: %x, status: %x", reg_val, regs_->STATUS);
      return false;
    }

    auto length = device_.block_size;
    if (write) {
      while (length > 0) {
        regs_->DATA = *buf++;
        length -= 4;
      }
    } else {
      while (length > 0) {
        *buf++ = regs_->DATA;
        length -= 4;
      }
    }
  }

  return true;
}

}  // namespace evisor
