#include "common/list.h"

#include "mm/new.h"

namespace evisor {

LinkedList::LinkedList() {
  nil_ = new LinkedList::Node;
  nil_->prev = nil_;
  nil_->next = nil_;
}

LinkedList::~LinkedList() {
  // TODO: delete all entries
  if (nil_) {
    delete nil_;
  }
}

LinkedList::Node* LinkedList::Begin() {
  return nil_->next;
}

LinkedList::Node* LinkedList::Next(LinkedList::Node* node) {
  return node->next;
}

uint32_t LinkedList::Front() {
  return nil_->next->key;
}

uint32_t LinkedList::Back() {
  return nil_->prev->key;
}

void LinkedList::PushFront(uint32_t key) {
  auto* x = new LinkedList::Node;
  x->key = key;

  x->next = nil_->next;
  nil_->next->prev = x;
  nil_->next = x;
  x->prev = nil_;
}

void LinkedList::PushBack(uint32_t key) {
  auto* x = new LinkedList::Node;
  x->key = key;

  x->prev = nil_->prev;
  nil_->prev->next = x;
  x->next = nil_;
}

void LinkedList::PopFront() { DeleteNode(nil_->next); }

void LinkedList::PopBack() { DeleteNode(nil_->prev); }

void LinkedList::Delete(uint32_t key) { DeleteNode(SearchNode(key)); }

LinkedList::Node* LinkedList::SearchNode(uint32_t key) {
  auto* cur = nil_->next;
  while (cur != nil_ && cur->key != key) {
    cur = cur->next;
  }
  return cur;
}

void LinkedList::DeleteNode(LinkedList::Node* node) {
  if (node == nil_) {
    return;
  }

  node->prev->next = node->next;
  node->next->prev = node->prev;
  delete node;
}

}  // namespace evisor
