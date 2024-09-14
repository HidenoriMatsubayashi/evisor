#ifndef EVISOR_COMMON_QUEUE_H_
#define EVISOR_COMMON_QUEUE_H_

#include <cstdbool>
#include <cstdint>

namespace evisor {

class Queue {
 public:
  Queue();
  ~Queue();

  // Prevent copying.
  Queue(Queue const&) = delete;
  Queue& operator=(Queue const&) = delete;

  bool Empty();
  bool Full();
  bool Push(uint32_t data);
  uint32_t Pop();

 private:
  uint32_t head_;
  uint32_t tail_;
  uint32_t* buff_;
};

}  // namespace evisor

#endif  // EVISOR_COMMON_QUEUE_H_
