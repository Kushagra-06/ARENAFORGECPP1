/* ArenaForge - C++ DSA engine
   Part 4 - Hash table: team-name search */
#include "arena_engine.hpp"

#include <cctype>

namespace arena {
namespace {

unsigned long hashKey(const std::string& s) {                   /* djb2, case-insensitive */
    unsigned long h = 5381;
    for (unsigned char c : s) h = ((h << 5) + h) + static_cast<unsigned char>(std::tolower(c));
    return h;
}

bool sameKey(const std::string& a, const std::string& b) {
    if (a.size() != b.size()) return false;
    for (std::size_t i = 0; i < a.size(); ++i)
        if (std::tolower(static_cast<unsigned char>(a[i])) != std::tolower(static_cast<unsigned char>(b[i])))
            return false;
    return true;
}

} // namespace

NameIndex::NameIndex(int buckets) : buckets_(static_cast<std::size_t>(buckets < 8 ? 8 : buckets)) {}

std::size_t NameIndex::bucketOf(const std::string& key) const {
    return static_cast<std::size_t>(hashKey(key) % buckets_.size());
}

void NameIndex::put(const std::string& key, int value) {
    std::size_t b = bucketOf(key);
    for (Node* n = buckets_[b].get(); n; n = n->next.get())
        if (sameKey(n->key, key)) { n->value = value; return; }

    auto node = std::make_unique<Node>();                       /* chaining: insert at head */
    node->key = key;
    node->value = value;
    node->next = std::move(buckets_[b]);
    buckets_[b] = std::move(node);
    ++count_;
}

std::optional<int> NameIndex::find(const std::string& key) const {
    for (const Node* n = buckets_[bucketOf(key)].get(); n; n = n->next.get())
        if (sameKey(n->key, key)) return n->value;
    return std::nullopt;
}

bool NameIndex::remove(const std::string& key) {
    std::unique_ptr<Node>* link = &buckets_[bucketOf(key)];
    while (*link) {
        if (sameKey((*link)->key, key)) {
            std::unique_ptr<Node> dead = std::move(*link);
            *link = std::move(dead->next);
            --count_;
            return true;
        }
        link = &(*link)->next;
    }
    return false;
}

void NameIndex::clear() {
    for (auto& head : buckets_)                                 /* unlink iteratively: no deep recursion */
        while (head) {
            std::unique_ptr<Node> dead = std::move(head);
            head = std::move(dead->next);
        }
    count_ = 0;
}

} // namespace arena
