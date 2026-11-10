/* ArenaForge - C++ DSA engine
   Part 3 - Tree: knockout bracket */
#include "arena_engine.hpp"

namespace arena {
namespace {

std::unique_ptr<BracketNode> makeNode(int id, int round, int team) {
    auto n = std::make_unique<BracketNode>();
    n->nodeId = id;
    n->round = round;
    n->teamId = team;
    return n;
}

/* standard seeding order: 1 v 8, 4 v 5, 2 v 7, 3 v 6 ... */
std::vector<int> seedOrder(int p) {
    std::vector<int> order(static_cast<std::size_t>(p));
    order[0] = 1;
    for (int len = 1; len < p; len *= 2)
        for (int i = len - 1; i >= 0; --i) {
            order[static_cast<std::size_t>(2 * i + 1)] = 2 * len + 1 - order[static_cast<std::size_t>(i)];
            order[static_cast<std::size_t>(2 * i)] = order[static_cast<std::size_t>(i)];
        }
    return order;
}

void collect(BracketNode* n, std::vector<BracketNode*>& out) {
    if (!n || !n->left || !n->right) return;                    /* leaf */
    collect(n->left.get(), out);
    collect(n->right.get(), out);
    if (n->teamId == kTbd && n->left->teamId >= 0 && n->right->teamId >= 0) out.push_back(n);
}

BracketNode* findNode(BracketNode* n, int id) {
    if (!n) return nullptr;
    if (n->nodeId == id) return n;
    BracketNode* f = findNode(n->left.get(), id);
    return f ? f : findNode(n->right.get(), id);
}

} // namespace

bool BracketTree::build(const std::vector<int>& seeded) {
    root_.reset();
    const int n = static_cast<int>(seeded.size());
    if (n < 2) return false;
    int p = 1;
    while (p < n) p *= 2;

    const std::vector<int> order = seedOrder(p);
    std::vector<std::unique_ptr<BracketNode>> level;
    for (int i = 0; i < p; ++i) {                               /* leaves */
        int seed = order[static_cast<std::size_t>(i)];
        level.push_back(makeNode(-1, 0, seed <= n ? seeded[static_cast<std::size_t>(seed - 1)] : kBye));
    }

    int nextId = 1, round = 1;
    std::size_t count = level.size();
    while (count > 1) {                                         /* build upward */
        for (std::size_t i = 0; i < count / 2; ++i) {
            std::unique_ptr<BracketNode> l = std::move(level[2 * i]);
            std::unique_ptr<BracketNode> r = std::move(level[2 * i + 1]);
            auto m = makeNode(nextId++, round, kTbd);
            if (l->teamId == kBye && r->teamId == kBye) m->teamId = kBye;
            else if (l->teamId == kBye) m->teamId = r->teamId;  /* bye: advance */
            else if (r->teamId == kBye) m->teamId = l->teamId;
            m->left = std::move(l);
            m->right = std::move(r);
            level[i] = std::move(m);
        }
        count /= 2;
        ++round;
    }
    root_ = std::move(level[0]);
    return true;
}

std::vector<BracketNode*> BracketTree::ready() {
    std::vector<BracketNode*> out;
    collect(root_.get(), out);
    return out;
}

bool BracketTree::reportWinner(int nodeId, int winner) {
    BracketNode* n = findNode(root_.get(), nodeId);
    if (!n || !n->left || !n->right || n->teamId != kTbd) return false;
    if (n->left->teamId < 0 || n->right->teamId < 0) return false;
    if (winner != n->left->teamId && winner != n->right->teamId) return false;
    n->teamId = winner;                                         /* winner moves up */
    return true;
}

int BracketTree::champion() const {
    return (root_ && root_->teamId >= 0) ? root_->teamId : kTbd;
}

} // namespace arena
