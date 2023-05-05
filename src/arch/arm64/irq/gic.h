#ifndef EVISOR_ARCH_ARM64_GIC_H_
#define EVISOR_ARCH_ARM64_GIC_H_

#include <cstdint>
#include <functional>

namespace evisor {

class Gic {
 public:
  using IrqHandler = std::function<void()>;

  Gic() = default;
  ~Gic() = default;

  // Prevent copying.
  Gic(Gic const&) = delete;
  Gic& operator=(Gic const&) = delete;

  // Init GIC (General Interrupt Controller)
  virtual void Init() = 0;

  // Handle IRQs
  virtual void HandleIrq() = 0;

  void CatchUnexpectedIrqs(uint32_t type, uint32_t esr_el2, uint32_t elr_el2,
                           uint32_t far_el2);

  // Register IRQ to GIC
  virtual void RegisterIrq(uint16_t id, uint32_t target_processor,
                           uint8_t priority, IrqHandler handler) = 0;

  virtual void NotifyVirqSoftware() = 0;

  virtual void NotifyVirqHardware(uint16_t intid) = 0;

  virtual void NotifyIrqSoftware(uint32_t sgi_id, uint32_t cpu_id) = 0;

  virtual void PrintDumpRegisters() = 0;

 protected:
  IrqHandler irq_handler_[1024] = {nullptr};
};

}  // namespace evisor

#endif  // EVISOR_ARCH_ARM64_GIC_H_
