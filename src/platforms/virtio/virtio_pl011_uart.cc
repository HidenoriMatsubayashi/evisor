#include "platforms/virtio/virtio_pl011_uart.h"

#include "common/cstdio.h"
#include "common/logger.h"
#include "kernel/sched/sched.h"
#include "platforms/board.h"

namespace evisor {

uint32_t VirtioPl011Uart::Read(uint16_t addr) {
  uint32_t res = 0;
  switch (addr) {
    case 0x000: {
      res = regs_.UARTDR;
      regs_.UARTFR = regs_.UARTFR | 0x10;
      break;
    }
    case 0x004:
      res = regs_.UARTRSR;
      break;
    case 0x018: {
      auto* tsk = Sched::Get().GetCurrentTask();
      const auto* console = tsk->board->GetConsole();
      if (!console->in->Empty()) {
        auto data = console->in->Pop();
        regs_.UARTDR = regs_.UARTDR & 0xf00;
        regs_.UARTDR = regs_.UARTDR | (data & 0xff);
        regs_.UARTFR = regs_.UARTFR & ~0x10;
      }
      res = regs_.UARTFR;
      break;
    }
    case 0x020:
      res = regs_.UARTILPR;
      break;
    case 0x024:
      res = regs_.UARTIBRD;
      break;
    case 0x028:
      res = regs_.UARTFBRD;
      break;
    case 0x02C:
      res = regs_.UARTLCR_H;
      break;
    case 0x030:
      res = regs_.UARTCR;
      break;
    case 0x034:
      res = regs_.UARTIFLS;
      break;
    case 0x038:
      res = regs_.UARTIMSC;
      break;
    case 0x03C:
      res = regs_.UARTRIS;
      break;
    case 0x040:
      res = regs_.UARTMIS;
      break;
    case 0x044:
      res = regs_.UARTICR;
      break;
    case 0x048:
      res = regs_.UARTDMACR;
      break;
    case 0xFE0: /* UARTPeriphID0 */
      res = 0x11;
      break;
    case 0xFE4: /* UARTPeriphID1 */
      res = 0x10;
      break;
    case 0xFE8: /* UARTPeriphID2 */
      res = 0x34;
      break;
    case 0xFEC: /* UARTPeriphID3 */
      res = 0x0;
      break;
    case 0xFF0: /* UARTCellID0 */
      res = 0xD;
      break;
    case 0xFF4: /* UARTCellID1 */
      res = 0xF0;
      break;
    case 0xFF8: /* UARTCellID2 */
      res = 0x05;
      break;
    case 0xFFC: /* UARTCellID3 */
      res = 0xB1;
      break;
    default:
      LOG_ERROR("Unexpected read: addr = %04x", addr);
      break;
  }
  return res;
}

void VirtioPl011Uart::Write(uint32_t addr, uint32_t data) {
  switch (addr) {
    case 0x000: {
      regs_.UARTDR = data & 0xfff;

      auto* tsk = Sched::Get().GetCurrentTask();
      const auto* console = tsk->board->GetConsole();
      if (!console->out->Full()) {
        console->out->Push(data & 0xFF);
      }
      break;
    }
    case 0x004:
      regs_.UARTRSR = data & 0xf;
      break;
    case 0x018:
      regs_.UARTFR = data & 0x1ff;
      break;
    case 0x020:
      regs_.UARTILPR = data & 0xff;
      break;
    case 0x024:
      regs_.UARTIBRD = data & 0xffff;
      break;
    case 0x028:
      regs_.UARTFBRD = data & 0x3f;
      break;
    case 0x02C:
      regs_.UARTLCR_H = data & 0xff;
      break;
    case 0x030:
      regs_.UARTCR = data & 0xffff;
      break;
    case 0x034:
      regs_.UARTIFLS = data & 0x3f;
      break;
    case 0x038:
      regs_.UARTIMSC = data & 0x7ff;
      break;
    case 0x03C:
      regs_.UARTRIS = data & 0x7ff;
      break;
    case 0x040:
      regs_.UARTMIS = data & 0x7ff;
      break;
    case 0x044:
      regs_.UARTICR = data & 0x7ff;
      break;
    case 0x048:
      regs_.UARTDMACR = data & 0x7;
      break;
    default:
      LOG_WARN("Unexpected write: addr = %04x, data = %04x", addr, data);
      break;
  }
}

}  // namespace evisor
