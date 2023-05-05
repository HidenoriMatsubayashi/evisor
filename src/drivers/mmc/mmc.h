#ifndef EVISOR_DRIVERS_MMC_MMC_H_
#define EVISOR_DRIVERS_MMC_MMC_H_

#include <cstdbool>
#include <cstdint>

#include "drivers/common.h"
#include "platforms/platform.h"

namespace evisor {

class Mmc {
 public:
  struct CmdRegister {
    uint8_t resp_a : 1;
    uint8_t block_count : 1;
    uint8_t auto_command : 2;
    uint8_t direction : 1;
    uint8_t multiblock : 1;
    uint16_t resp_b : 10;
    uint8_t response_type : 2;
    uint8_t res0 : 1;
    uint8_t crc_enable : 1;
    uint8_t idx_enable : 1;
    uint8_t is_data : 1;
    uint8_t type : 2;
    uint8_t index : 6;
    uint8_t res1 : 2;
  };

  Mmc() = default;
  ~Mmc() = default;

  // Prevent copying.
  Mmc(Mmc const&) = delete;
  Mmc& operator=(Mmc const&) = delete;

  static Mmc& Get() {
    static Mmc instance;
    return instance;
  }

  bool Open();
  bool Seek(uint64_t offset);
  int Read(uint8_t* buf, uint32_t size);

 private:
  // SD card commands
  enum class CmdType : int {
    GO_IDLE_STATE = 0,
    SEND_OP_COND = 1,
    ALL_SEND_CID = 2,
    SEND_RELATIVE_ADDR = 3,
    SET_DSR = 4,
    IO_SET_OP_COND = 5,
    SWITCH_FUNC = 6,
    SELECT_CARD = 7,
    SEND_IF_COND = 8,
    // CMD9 - CMD15
    SET_BLOCKLEN = 16,
    READ_SINGLE_BLOCK = 17,
    READ_MULTIPLE_BLOCK = 18,
    // CMD19 - CMD23
    WRITE_BLOCK = 24,
    WRITE_MULTIPLE_BLOCK = 25,
    // CMD26 - CMD40
    CTOcrCheck = 41,
    // CMD42 - CMD50
    CTSendSCR = 51,
    //
    APP_CMD = 55,
    //
    INVALID = 0xffff,
  };

  struct ScrReg {
    uint32_t scr[2];
    uint32_t bus_widths;
    uint32_t version;
  };

  enum class SdError : int {
    SDECommandTimeout = 0,
    SDECommandCrc = 1,
    SDECommandEndBit = 2,
    SDECommandIndex = 3,
    SDEDataTimeout = 4,
    SDEDataCrc = 5,
    SDEDataEndBit,
    SDECurrentLimit,
    SDEAutoCmd12,
    SDEADma,
    SDETuning,
    SDERsvd
  };

  struct Device {
    bool sdhc;
    uint16_t ocr;
    uint32_t rca;
    ScrReg scr;
    uint32_t base_clock;
    uint32_t block_size;
    uint64_t seek_offset;
  };

  struct CommandData {
    CmdType cmd_type;
    CmdRegister cmd_reg;
    uint32_t cmd_arg;
    uint32_t cmd_timeout_usec;
    uint32_t transfer_blocks;
    uint64_t offset;
    void* buffer;
    uint32_t cmd_response[4];
    uint32_t cmd_error;
  };

  enum class CmdRespType : int {
    RTNone = 0,
    RT136 = 1,
    RT48 = 2,
    RT48Busy = 3
  };

  const CmdRegister kReserved_ = { 1, 1, 3, 1, 1, 0xF, 3, 1, 1, 1, 1, 3, 0xF, 3 };
  const CmdRegister kCommands_[56] = {
      {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
      kReserved_,
      {0, 0, 0, 0, 0, 0, static_cast<int>(CmdRespType::RT136), 0, 1, 0, 0, 0, 2,
       0},
      {0, 0, 0, 0, 0, 0, static_cast<int>(CmdRespType::RT48), 0, 1, 0, 0, 0, 3,
       0},
      {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 0},
      {0, 0, 0, 0, 0, 0, static_cast<int>(CmdRespType::RT136), 0, 0, 0, 0, 0, 5,
       0},
      {0, 0, 0, 0, 0, 0, static_cast<int>(CmdRespType::RT48), 0, 1, 0, 0, 0, 6,
       0},
      {0, 0, 0, 0, 0, 0, static_cast<int>(CmdRespType::RT48Busy), 0, 1, 0, 0, 0,
       7, 0},
      {0, 0, 0, 0, 0, 0, static_cast<int>(CmdRespType::RT48), 0, 1, 0, 0, 0, 8,
       0},
      {0, 0, 0, 0, 0, 0, static_cast<int>(CmdRespType::RT136), 0, 1, 0, 0, 0, 9,
       0},
      kReserved_,
      kReserved_,
      kReserved_,
      kReserved_,
      kReserved_,
      kReserved_,
      {0, 0, 0, 0, 0, 0, static_cast<int>(CmdRespType::RT48), 0, 1, 0, 0, 0, 16,
       0},
      {0, 0, 0, 1, 0, 0, static_cast<int>(CmdRespType::RT48), 0, 1, 0, 1, 0, 17,
       0},
      {0, 1, 1, 1, 1, 0, static_cast<int>(CmdRespType::RT48), 0, 1, 0, 1, 0, 18,
       0},
      kReserved_,
      kReserved_,
      kReserved_,
      kReserved_,
      kReserved_,
      kReserved_,
      kReserved_,
      kReserved_,
      kReserved_,
      kReserved_,
      kReserved_,
      kReserved_,
      kReserved_,
      kReserved_,
      kReserved_,
      kReserved_,
      kReserved_,
      kReserved_,
      kReserved_,
      kReserved_,
      kReserved_,
      kReserved_,
      {0, 0, 0, 0, 0, 0, static_cast<int>(CmdRespType::RT48), 0, 0, 0, 0, 0, 41,
       0},
      kReserved_,
      kReserved_,
      kReserved_,
      kReserved_,
      kReserved_,
      kReserved_,
      kReserved_,
      kReserved_,
      kReserved_,
      {0, 0, 0, 1, 0, 0, static_cast<int>(CmdRespType::RT48), 0, 1, 0, 1, 0, 51,
       0},
      kReserved_,
      kReserved_,
      kReserved_,
      {0, 0, 0, 0, 0, 0, static_cast<int>(CmdRespType::RT48), 0, 1, 0, 0, 0, 55,
       0},
  };

  bool HandleDataCommand(bool write, uint8_t* buf, uint32_t buf_size,
                         uint32_t block);
  bool AppCommand(CommandData& d);
  bool SendCommand(CommandData& d);
  bool WaitCommandDone(CommandData& d);
  bool PerformDataTransfer(CommandData& d);

  bool ResetCard();
  bool IsCardInserted();
  bool ResetCommandHandlingCircuit();
  bool CheckV2Card();
  bool CheckUsableCard();
  bool CheckSdhcSupport(bool v2_card);
  bool CheckOcr();
  bool CheckRca();
  bool SelectCard();
  bool SetScr();
  uint32_t SdErrorMask(SdError err);

  void SetupHostIrqs();

  bool SetupInitialClockRate();
  bool ChangeClockRate(uint32_t base_clock, uint32_t target_rate);

  Device device_ = {
      .sdhc = 0,
      .ocr = 0,
      .rca = 0,
      .scr =
          {
              .scr = {0},
              .bus_widths = 0,
              .version = 0,
          },
      .base_clock = 0,
      .block_size = 0,
      .seek_offset = 0,
  };

  // EMMC2 registers
  // https://github.com/rsta2/circle/blob/master/addon/SDCard/emmc.cpp
  // https://www.raspberrypi.org/app/uploads/2012/02/BCM2835-ARM-Peripherals.pdf
  struct Emmc2Regs {
    reg32_t ARG2;
    reg32_t BLKSIZECNT;
    reg32_t ARG1;
    reg32_t CMDTM;
    reg32_t RESP0;
    reg32_t RESP1;
    reg32_t RESP2;
    reg32_t RESP3;
    reg32_t DATA;
    reg32_t STATUS;
    reg32_t CONTROL0;
    reg32_t CONTROL1;
    reg32_t INTERRUPT;
    reg32_t IRPT_MASK;
    reg32_t IRPT_EN;
    reg32_t CONTROL2;
    reg32_t CAP1;
    reg32_t CAP2;
    reg32_t RESERVED0[2];  // 0x48 - 0x4C
    reg32_t FORCE_IRPT;
    reg32_t RESERVED1[7];  // 0x54 - 0x6C
    reg32_t BOOT_TIMEOUT;
    reg32_t DBG_SEL;
    reg32_t RESERVED2[2];  // 0x78 - 0x7C
    reg32_t EXRDFIFO_CFG;
    reg32_t EXRDFIFO_EN;
    reg32_t TUNE_STEP;
    reg32_t TUNE_STEPS_STD;
    reg32_t TUNE_STEPS_DDR;
    reg32_t RESERVED3[23];  // 0x94 - 0xEC
    reg32_t SPI_INT_SPT;
    reg32_t RESERVED4[2];  // 0xF4 - 0xF8
    reg32_t SLOTISR_VER;
  } __attribute__((packed)) __attribute__((aligned(4)));

  // External Mass Media Controller 2 registers
  Emmc2Regs* regs_ = reinterpret_cast<Emmc2Regs*>(EMMC2_BASE);
};

}  // namespace evisor

#endif  // EVISOR_DRIVERS_MMC_MMC_H_
