#ifndef EVISOR_PLATFORMS_VIRTIO_VIRTIO_GIC_H_
#define EVISOR_PLATFORMS_VIRTIO_VIRTIO_GIC_H_

#include <cstdint>

namespace evisor {

class VirtioGic {
 public:
  VirtioGic() = default;
  ~VirtioGic() = default;

  uint32_t Read(uint16_t addr);
  void Write(uint32_t addr, uint32_t data);

 private:
  struct Regs {
    uint32_t GICD_CTLR;
    uint32_t GICD_TYPER;
    uint32_t GICD_IIDR;
    uint32_t GICD_IGROUPR[32];
    uint32_t GICD_ISENABLER[32];
    uint32_t GICD_ICENABLER[32];
    uint32_t GICD_ISPENDR[32];
    uint32_t GICD_ICPENDR[32];
    uint32_t GICD_ISACTIVER[32];
    uint32_t GICD_ICACTIVER[32];
    uint32_t GICD_IPRIORITYR[255];
    uint32_t GICD_ITARGETSR[255];
    uint32_t GICD_ICFGR[64];
    uint32_t GICD_NSACR[64];
    uint32_t GICD_SGIR;
    uint32_t GICD_CPENDSGIR[4];
    uint32_t GICD_SPENDSGIR[4];
  };
  Regs regs_ = {
      .GICD_CTLR = 0,
      .GICD_TYPER = 0,
      .GICD_IIDR = 0,
      .GICD_IGROUPR = {0},
      .GICD_ISENABLER = {0},
      .GICD_ICENABLER = {0},
      .GICD_ISPENDR = {0},
      .GICD_ICPENDR = {0},
      .GICD_ISACTIVER = {0},
      .GICD_ICACTIVER = {0},
      .GICD_IPRIORITYR = {0},
      .GICD_ITARGETSR = {0},
      .GICD_ICFGR = {0},
      .GICD_NSACR = {0},
      .GICD_SGIR = 0,
      .GICD_CPENDSGIR = {0},
      .GICD_SPENDSGIR = {0},
  };
};

}  // namespace evisor

#endif  // EVISOR_PLATFORMS_VIRTIO_VIRTIO_GIC_H_
