#ifndef EVISOR_COMMON_LIST_H_
#define EVISOR_COMMON_LIST_H_

#include <cstdint>

#include "mm/new.h"

namespace evisor {

class LinkedList {
 public:
  class Node {
   public:
    uint32_t key;
    Node* prev;
    Node* next;
  };

  LinkedList();
  ~LinkedList();

  LinkedList::Node* Begin();
  LinkedList::Node* Next(LinkedList::Node* node);
  uint32_t Front();
  uint32_t Back();

  void PushFront(uint32_t key);
  void PushBack(uint32_t key);
  void PopFront();
  void PopBack();
  void Delete(uint32_t key);

 private:
  Node* SearchNode(uint32_t key);
  void DeleteNode(LinkedList::Node* node);

  Node* nil_;
};

}  // namespace evisor

#endif  // EVISOR_COMMON_LIST_H_
