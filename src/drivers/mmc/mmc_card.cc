
#include "common/logger.h"
#include "drivers/mailbox/mailbox.h"
#include "drivers/mmc/mmc.h"
#include "drivers/mmc/mmc_internal.h"
#include "platforms/timer.h"

#define BSWAP32(x)                                      \
  (((x << 24) & 0xff000000) | ((x << 8) & 0x00ff0000) | \
   ((x >> 8) & 0x0000ff00) | ((x >> 24) & 0x000000ff))

namespace evisor {

bool Mmc::ResetCard() {
  // Reset EMMC controller
  {
    regs_->CONTROL1 = regs_->CONTROL1 & ~EMMC_CTRL1_CLK_EN;
    regs_->CONTROL1 = regs_->CONTROL1 & ~EMMC_CTRL1_CLK_INTLEN;
    regs_->CONTROL1 = regs_->CONTROL1 | EMMC_CTRL1_RESET_HOST;
    if (!RegWaitBitClear(&regs_->CONTROL1,
                         (EMMC_CTRL1_RESET_DATA | EMMC_CTRL1_RESET_CMD |
                          EMMC_CTRL1_RESET_HOST),
                         1000)) {
      LOG_ERROR("HOST controller reset timeout");
      return false;
    }
  }

  // Enable SD Bus Power VDD1 at 3.3V
  {
    regs_->CONTROL0 = regs_->CONTROL0 | (0x0F << 8);
    // regs_->CONTROL0 = regs_->CONTROL0 | (1 << 2); // HISPEED
    // regs_->CONTROL0 = regs_->CONTROL0 | (1 << 1); // 4DATA
    Timer::Sleep(3);
  }

  // Check for a valid card
  if (!IsCardInserted()) {
    LOG_ERROR("No card inserted");
    return false;
  }

  if (!SetupInitialClockRate()) {
    return false;
  }

  SetupHostIrqs();

  device_.block_size = 0;

  {
    CommandData d = {
        .cmd_type = CmdType::INVALID,
        .cmd_reg = kReserved_,
        .cmd_arg = 0,
        .cmd_timeout_usec = 0,
        .transfer_blocks = 0,
        .offset = 0,
        .buffer = nullptr,
        .cmd_response = {0},
        .cmd_error = 0,
    };
    d.cmd_type = CmdType::GO_IDLE_STATE;
    d.cmd_reg = kCommands_[static_cast<int>(d.cmd_type)];
    d.cmd_arg = 0;
    d.cmd_timeout_usec = 2000;

    if (!SendCommand(d)) {
      LOG_ERROR("GO_IDLE_STATE cmd failed");
      return false;
    }
  }

  bool v2_card = CheckV2Card();

  if (!CheckUsableCard()) {
    return false;
  }

  if (!CheckOcr()) {
    return false;
  }

  if (!CheckSdhcSupport(v2_card)) {
    return false;
  }

  ChangeClockRate(device_.base_clock, SD_CLOCK_HIGH);
  Timer::Sleep(10);

  if (!CheckRca()) {
    return false;
  }

  if (!SelectCard()) {
    return false;
  }

  if (!SetScr()) {
    return false;
  }

  // Clear all interrupts to be sure
  regs_->INTERRUPT = 0xFFFFFFFF;

  return true;
}

bool Mmc::CheckV2Card() {
  bool v2Card = false;

  CommandData d = {
      .cmd_type = CmdType::INVALID,
      .cmd_reg = kReserved_,
      .cmd_arg = 0,
      .cmd_timeout_usec = 0,
      .transfer_blocks = 0,
      .offset = 0,
      .buffer = nullptr,
      .cmd_response = {0},
      .cmd_error = 0,
  };
  d.cmd_type = CmdType::SEND_IF_COND;
  d.cmd_reg = kCommands_[static_cast<int>(d.cmd_type)];
  d.cmd_arg = 0x1AA;
  d.cmd_timeout_usec = 200;

  if (!SendCommand(d)) {
    if (d.cmd_error == 0) {
      LOG_ERROR("SEND_IF_COND timeout");
    } else if (d.cmd_error & (1 << 16)) {
      // timeout command error
      if (!ResetCommandHandlingCircuit()) {
        return false;
      }

      regs_->INTERRUPT = SdErrorMask(SdError::SDECommandTimeout);
      LOG_ERROR("SEND_IF_COND cmd timeout");
    } else {
      LOG_ERROR("Failure sending SEND_IF_COND");
      return false;
    }
  } else {
    if ((d.cmd_response[0] & 0xFFF) != 0x1AA) {
      LOG_ERROR("Unusable SD Card: %X", d.cmd_response[0]);
      return false;
    }

    v2Card = true;
  }

  return v2Card;
}

bool Mmc::CheckUsableCard() {
  CommandData d = {
      .cmd_type = CmdType::INVALID,
      .cmd_reg = kReserved_,
      .cmd_arg = 0,
      .cmd_timeout_usec = 0,
      .transfer_blocks = 0,
      .offset = 0,
      .buffer = nullptr,
      .cmd_response = {0},
      .cmd_error = 0,
  };
  d.cmd_type = CmdType::IO_SET_OP_COND;
  d.cmd_reg = kCommands_[static_cast<int>(d.cmd_type)];
  d.cmd_arg = 0;
  d.cmd_timeout_usec = 1000;

  if (!SendCommand(d)) {
    if (d.cmd_error == 0) {
      LOG_ERROR("IO_SET_OP_COND Timeout");
    } else if (d.cmd_error & (1 << 16)) {
      // timeout command error
      // this is a normal expected error and calling the reset command will fix
      // it.
      if (!ResetCommandHandlingCircuit()) {
        return false;
      }
      regs_->INTERRUPT = SdErrorMask(SdError::SDECommandTimeout);
    } else {
      LOG_ERROR("SDIO Card not supported");
      return false;
    }
  }

  return true;
}

bool Mmc::CheckSdhcSupport(bool v2_card) {
  bool card_busy = true;
  CommandData d = {
      .cmd_type = CmdType::INVALID,
      .cmd_reg = kReserved_,
      .cmd_arg = 0,
      .cmd_timeout_usec = 0,
      .transfer_blocks = 0,
      .offset = 0,
      .buffer = nullptr,
      .cmd_response = {0},
      .cmd_error = 0,
  };
  d.cmd_type = CmdType::CTOcrCheck;
  d.cmd_reg = kCommands_[static_cast<int>(d.cmd_type)];
  d.cmd_timeout_usec = 2000;

  while (card_busy) {
    uint32_t v2_flags = 0;

    if (v2_card) {
      v2_flags |= (1 << 30);  // SDHC Support
    }

    d.cmd_arg = 0x00FF8000 | v2_flags;
    if (!AppCommand(d)) {
      LOG_ERROR("APP CMD 41 FAILED 2nd");
      return false;
    }

    if (d.cmd_response[0] >> 31 & 1) {
      device_.ocr = (d.cmd_response[0] >> 8 & 0xFFFF);
      device_.sdhc = ((d.cmd_response[0] >> 30) & 1) != 0;
      card_busy = false;
    } else {
      LOG_DEBUG("SLEEPING: %X", d.cmd_response[0]);
      Timer::Sleep(500);
    }
  }

  return true;
}

bool Mmc::CheckOcr() {
  bool passed = false;
  CommandData d = {
      .cmd_type = CmdType::INVALID,
      .cmd_reg = kReserved_,
      .cmd_arg = 0,
      .cmd_timeout_usec = 0,
      .transfer_blocks = 0,
      .offset = 0,
      .buffer = nullptr,
      .cmd_response = {0},
      .cmd_error = 0,
  };
  d.cmd_type = CmdType::CTOcrCheck;
  d.cmd_reg = kCommands_[static_cast<int>(d.cmd_type)];
  d.cmd_arg = 0;
  d.cmd_timeout_usec = 2000;

  for (int i = 0; i < 5; i++) {
    if (AppCommand(d)) {
      passed = true;
      break;
    }
    LOG_WARN("Failed to CT0crCheck command send, trying again...(%d)", i);
  }

  if (!passed) {
    LOG_ERROR("check_ocr(41) failed");
    return false;
  }

  device_.ocr = (d.cmd_response[0] >> 8 & 0xFFFF);
  LOG_DEBUG("MEMORY OCR: %X", device_.ocr);

  return true;
}

bool Mmc::CheckRca() {
  CommandData d = {
      .cmd_type = CmdType::INVALID,
      .cmd_reg = kReserved_,
      .cmd_arg = 0,
      .cmd_timeout_usec = 0,
      .transfer_blocks = 0,
      .offset = 0,
      .buffer = nullptr,
      .cmd_response = {0},
      .cmd_error = 0,
  };
  d.cmd_type = CmdType::ALL_SEND_CID;
  d.cmd_reg = kCommands_[static_cast<int>(d.cmd_type)];
  d.cmd_arg = 0;
  d.cmd_timeout_usec = 2000;

  if (!SendCommand(d)) {
    LOG_ERROR("Failed to send CID");
    return false;
  }
  LOG_DEBUG("CARD ID: %X.%X.%X.%X", d.cmd_response[0], d.cmd_response[1],
            d.cmd_response[2], d.cmd_response[3]);

  d.cmd_type = CmdType::SEND_RELATIVE_ADDR;
  d.cmd_reg = kCommands_[static_cast<int>(d.cmd_type)];
  if (!SendCommand(d)) {
    LOG_ERROR("SEND_RELATIVE_ADDR cmd failed");
    return false;
  }

  device_.rca = (d.cmd_response[0] >> 16) & 0xFFFF;
  LOG_DEBUG("RCA: %X", device_.rca);
  LOG_DEBUG("CRC_ERR: %d", (d.cmd_response[0] >> 15) & 1);
  LOG_DEBUG("CMD_ERR: %d", (d.cmd_response[0] >> 14) & 1);
  LOG_DEBUG("GEN_ERR: %d", (d.cmd_response[0] >> 13) & 1);
  LOG_DEBUG("STS_ERR: %d", (d.cmd_response[0] >> 9) & 1);
  LOG_DEBUG("READY  : %d", (d.cmd_response[0] >> 8) & 1);

  if (!((d.cmd_response[0] >> 8) & 1)) {
    LOG_ERROR("Failed to read RCA");
    return false;
  }

  return true;
}

bool Mmc::SelectCard() {
  CommandData d = {
      .cmd_type = CmdType::INVALID,
      .cmd_reg = kReserved_,
      .cmd_arg = 0,
      .cmd_timeout_usec = 0,
      .transfer_blocks = 0,
      .offset = 0,
      .buffer = nullptr,
      .cmd_response = {0},
      .cmd_error = 0,
  };
  d.cmd_type = CmdType::SELECT_CARD;
  d.cmd_reg = kCommands_[static_cast<int>(d.cmd_type)];
  d.cmd_arg = device_.rca << 16;
  d.cmd_timeout_usec = 2000;

  if (!SendCommand(d)) {
    LOG_ERROR("Failed to select card");
    return false;
  }

  uint32_t status = (d.cmd_response[0] >> 9) & 0xF;
  if (status != 3 && status != 4) {
    LOG_ERROR("Invalid Status: %d", status);
    return false;
  }
  LOG_DEBUG("Status: %d", status);

  return true;
}

bool Mmc::SetScr() {
  CommandData d = {
      .cmd_type = CmdType::INVALID,
      .cmd_reg = kReserved_,
      .cmd_arg = 0,
      .cmd_timeout_usec = 0,
      .transfer_blocks = 0,
      .offset = 0,
      .buffer = nullptr,
      .cmd_response = {0},
      .cmd_error = 0,
  };

  if (!device_.sdhc) {
    d.cmd_type = CmdType::SET_BLOCKLEN;
    d.cmd_reg = kCommands_[static_cast<int>(d.cmd_type)];
    d.cmd_arg = 512;
    d.cmd_timeout_usec = 2000;
    if (!SendCommand(d)) {
      LOG_ERROR("Failed to set block len");
      return false;
    }
  }

  uint32_t bsc = regs_->BLKSIZECNT;
  bsc &= ~0xFFF;  // mask off bottom bits
  bsc |= 0x200;   // set bottom bits to 512
  regs_->BLKSIZECNT = bsc;

  d.buffer = &device_.scr.scr[0];
  d.transfer_blocks = 1;
  device_.block_size = 8;

  d.cmd_type = CmdType::CTSendSCR;
  d.cmd_reg = kCommands_[static_cast<int>(d.cmd_type)];
  d.cmd_arg = 0;
  d.cmd_timeout_usec = 30000;
  if (!AppCommand(d)) {
    LOG_ERROR("Failed to send SCR");
    return false;
  }

  LOG_DEBUG("GOT SRC: SCR0: %X SCR1: %X BWID: %X", device_.scr.scr[0],
            device_.scr.scr[1], device_.scr.bus_widths);

  device_.block_size = 512;

  uint32_t scr0 = BSWAP32(device_.scr.scr[0]);
  device_.scr.version = 0xFFFFFFFF;
  uint32_t spec = (scr0 >> (56 - 32)) & 0xf;
  uint32_t spec3 = (scr0 >> (47 - 32)) & 0x1;
  uint32_t spec4 = (scr0 >> (42 - 32)) & 0x1;

  if (spec == 0) {
    device_.scr.version = 1;
  } else if (spec == 1) {
    device_.scr.version = 11;
  } else if (spec == 2) {
    if (spec3 == 0) {
      device_.scr.version = 2;
    } else if (spec3 == 1) {
      if (spec4 == 0) {
        device_.scr.version = 3;
      }
      if (spec4 == 1) {
        device_.scr.version = 4;
      }
    }
  }
  LOG_DEBUG("SCR Version: %d", device_.scr.version);

  return true;
}

bool Mmc::IsCardInserted() {
  LOG_TRACE("checking for an inserted card");
  uint32_t status = regs_->STATUS;
  if ((status & (1 << 16)) == 0) {
    return false;
  }
  return true;
}

uint32_t Mmc::SdErrorMask(SdError err) {
  return 1 << (16 + static_cast<uint32_t>(err));
}

bool Mmc::ResetCommandHandlingCircuit() {
  regs_->CONTROL1 = regs_->CONTROL1 | EMMC_CTRL1_RESET_CMD;

  for (int i = 0; i < 10000; i++) {
    if (!(regs_->CONTROL1 & EMMC_CTRL1_RESET_CMD)) {
      return true;
    }
    Timer::Sleep(1);
  }
  LOG_ERROR("Command line failed to reset properly: %X", regs_->CONTROL1);
  return false;
}

}  // namespace evisor
