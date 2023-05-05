#ifndef EVISOR_PLATFORMS_BCM2711_BOARD_BCM2711_H_
#define EVISOR_PLATFORMS_BCM2711_BOARD_BCM2711_H_

#include "platforms/board.h"
#include "platforms/virtio/virtio_gic.h"
#include "platforms/virtio/virtio_pl011_uart.h"

namespace evisor {

class BoardBcm2711 : public Board {
 public:
  BoardBcm2711() = default;
  ~BoardBcm2711() = default;

  // Prevent copying.
  BoardBcm2711(BoardBcm2711 const&) = delete;
  BoardBcm2711& operator=(BoardBcm2711 const&) = delete;

  static BoardBcm2711& Get() noexcept {
    static BoardBcm2711 instance;
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

#endif  // EVISOR_PLATFORMS_BCM2711_BOARD_BCM2711_H_
