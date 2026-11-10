/* ArenaForge - C++ DSA engine
   Part 6 - Linked list: match history */
#include "arena_engine.hpp"

namespace arena {

HistoryLog::~HistoryLog() {
    while (head_) {
        Node* next = head_->next;
        delete head_;
        head_ = next;
    }
}

void HistoryLog::append(const HistoryRecord& rec) {
    Node* n = new Node{rec, nullptr};
    if (tail_) tail_->next = n; else head_ = n;
    tail_ = n;
    ++count_;
}

int HistoryLog::meetings(int a, int b) const {
    int c = 0;
    for (const Node* n = head_; n; n = n->next)
        if ((n->rec.teamA == a && n->rec.teamB == b) || (n->rec.teamA == b && n->rec.teamB == a)) ++c;
    return c;
}

std::vector<HistoryRecord> HistoryLog::records() const {
    std::vector<HistoryRecord> out;
    for (const Node* n = head_; n; n = n->next) out.push_back(n->rec);
    return out;
}

} // namespace arena
