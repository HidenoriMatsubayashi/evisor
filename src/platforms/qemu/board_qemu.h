#ifndef EVISOR_PLATFORMS_QEMU_BOARD_QEMU_H_
#define EVISOR_PLATFORMS_QEMU_BOARD_QEMU_H_

#include "platforms/board.h"
#include "platforms/virtio/virtio_gic.h"
#include "platforms/virtio/virtio_pl011_uart.h"

namespace evisor {

class BoardQemu : public Board {
 public:
  BoardQemu() = default;
  ~BoardQemu() = default;

  // Prevent copying.
  BoardQemu(BoardQemu const&) = delete;
  BoardQemu& operator=(BoardQemu const&) = delete;

  static BoardQemu& Get() noexcept {
    static BoardQemu instance;
    return instance;
  }

  void Init(Tcb* tsk) override;
  uint64_t MmioRead(Tcb* tsk, uint64_t addr) override;
  void MmioWrite(Tcb* tsk, uint64_t addr, uint64_t val) override;

 private:
  VirtioGic virtio_gic_;
  VirtioPl011Uart virtio_uart_;
};

}  // namespace evisor

#endif  // EVISOR_PLATFORMS_QEMU_BOARD_QEMU_H_
