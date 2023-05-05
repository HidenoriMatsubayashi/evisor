#ifndef EVISOR_DRIVERS_MAILBOX_H_
#define EVISOR_DRIVERS_MAILBOX_H_

#include <cstdint>

namespace evisor {

class Mailbox {
 public:
  Mailbox() = default;
  ~Mailbox() = default;

  virtual bool Process(void* tag, uint32_t size) = 0;
};

}  // namespace evisor

#endif  // EVISOR_DRIVERS_MAILBOX_H_
