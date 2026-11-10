/* ArenaForge - C++ DSA engine
   Part 5 - Queue: registration / waitlist */
#include "arena_engine.hpp"

namespace arena {

WaitQueue::~WaitQueue() {
    int ignored;
    while (dequeue(ignored)) {}
}

void WaitQueue::enqueue(int v) {
    Node* n = new Node{v, nullptr};
    if (tail_) tail_->next = n; else head_ = n;
    tail_ = n;
    ++size_;
}

bool WaitQueue::dequeue(int& out) {
    if (!head_) return false;
    Node* n = head_;
    out = n->value;
    head_ = n->next;
    if (!head_) tail_ = nullptr;
    delete n;
    --size_;
    return true;
}

bool WaitQueue::peek(int& out) const {
    if (!head_) return false;
    out = head_->value;
    return true;
}

bool WaitQueue::remove(int v) {
    Node* prev = nullptr;
    for (Node* n = head_; n; prev = n, n = n->next)
        if (n->value == v) {
            if (prev) prev->next = n->next; else head_ = n->next;
            if (n == tail_) tail_ = prev;
            delete n;
            --size_;
            return true;
        }
    return false;
}

bool WaitQueue::contains(int v) const {
    for (const Node* n = head_; n; n = n->next)
        if (n->value == v) return true;
    return false;
}

std::vector<int> WaitQueue::toVector() const {
    std::vector<int> out;
    for (const Node* n = head_; n; n = n->next) out.push_back(n->value);
    return out;
}

} // namespace arena
