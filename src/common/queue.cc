#include "common/queue.h"

#include "common/logger.h"
#include "mm/heap/kmm_malloc.h"

namespace evisor {

namespace {
constexpr int kMaxSize = 1024;
}  // namespace

Queue::Queue() {
  buff_ = static_cast<uint32_t *>(kmm_malloc(kMaxSize));
  head_ = 0;
  tail_ = 0;
}

Queue::~Queue() { kmm_free(buff_); }

bool Queue::Empty() { return head_ == tail_; }

bool Queue::Full() { return head_ == (tail_ + 1) % kMaxSize; }

bool Queue::Push(uint32_t data) {
  if (Full()) {
    LOG_ERROR("queue overflow");
    return false;
  }
  buff_[tail_++] = data;
  if (tail_ == kMaxSize) {
    tail_ = 0;
  }
  return true;
}

uint32_t Queue::Pop() {
  if (Empty()) {
    LOG_ERROR("queue underflow");
    return 0;
  }

  auto res = buff_[head_++];
  if (head_ == kMaxSize) {
    head_ = 0;
  }
  return res;
}

}  // namespace evisor
