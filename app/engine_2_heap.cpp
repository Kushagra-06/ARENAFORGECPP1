/* ArenaForge - C++ DSA engine
   Part 2 - Heap / priority queue + smart matchmaking */
#include "arena_engine.hpp"

namespace arena {
namespace {

bool pairLess(const MatchPair& x, const MatchPair& y) {
    if (x.cost != y.cost) return x.cost < y.cost;
    if (x.teamA != y.teamA) return x.teamA < y.teamA;
    return x.teamB < y.teamB;
}

std::size_t indexOf(const std::vector<RankEntry>& t, int id) {
    for (std::size_t i = 0; i < t.size(); ++i)
        if (t[i].id == id) return i;
    return t.size();
}

double pairCost(const std::vector<RankEntry>& t, const HistoryLog& hist, int a, int b) {
    return fairScore(t[indexOf(t, a)].score, t[indexOf(t, b)].score, hist.meetings(a, b));
}

} // namespace

/* ------------------------------ binary min-heap ------------------------------ */

PairHeap::PairHeap(int capacityHint) {
    if (capacityHint > 0) data_.reserve(static_cast<std::size_t>(capacityHint));
}

void PairHeap::push(const MatchPair& p) {
    data_.push_back(p);
    std::size_t i = data_.size() - 1;                           /* sift up */
    while (i > 0) {
        std::size_t parent = (i - 1) / 2;
        if (!pairLess(p, data_[parent])) break;
        data_[i] = data_[parent];
        i = parent;
    }
    data_[i] = p;
}

bool PairHeap::pop(MatchPair& out) {
    if (data_.empty()) return false;
    out = data_.front();
    const MatchPair last = data_.back();
    data_.pop_back();
    if (data_.empty()) return true;

    std::size_t i = 0;                                          /* sift down */
    for (;;) {
        std::size_t c = 2 * i + 1;
        if (c >= data_.size()) break;
        if (c + 1 < data_.size() && pairLess(data_[c + 1], data_[c])) ++c;
        if (!pairLess(data_[c], last)) break;
        data_[i] = data_[c];
        i = c;
    }
    data_[i] = last;
    return true;
}

/* ------------------------------ matchmaking ------------------------------ */

double fairScore(double ra, double rb, int previousMeetings) {
    double diff = ra > rb ? ra - rb : rb - ra;
    return diff + kRepeatPenalty * previousMeetings;
}

std::vector<MatchPair> matchmake(const std::vector<RankEntry>& t, const HistoryLog& hist,
                                 std::size_t maxPairs) {
    std::vector<MatchPair> out;
    const std::size_t n = t.size();
    if (n < 2 || maxPairs < 1) return out;

    /* every possible pairing goes into the heap, cheapest on top */
    PairHeap heap(static_cast<int>(n * (n - 1) / 2));
    for (std::size_t i = 0; i < n; ++i)
        for (std::size_t j = i + 1; j < n; ++j) {
            MatchPair p;
            p.teamA = t[i].id;
            p.teamB = t[j].id;
            p.cost = fairScore(t[i].score, t[j].score, hist.meetings(t[i].id, t[j].id));
            heap.push(p);
        }

    std::vector<char> used(n, 0);
    MatchPair p;
    while (out.size() < maxPairs && heap.pop(p)) {
        std::size_t ia = indexOf(t, p.teamA), ib = indexOf(t, p.teamB);
        if (used[ia] || used[ib]) continue;                     /* a team plays once per round */
        used[ia] = used[ib] = 1;
        out.push_back(p);
    }

    /* Greedy can strand the last two teams together (a rematch). Local improvement
       (2-opt): for two pairs (a,b) and (c,d) try (a,c)+(b,d) and (a,d)+(b,c); keep a
       re-pairing when the total cost drops. The total strictly decreases, so it ends. */
    for (int pass = 0, improved = 1; improved && pass < 100; ++pass) {
        improved = 0;
        for (std::size_t i = 0; i < out.size(); ++i)
            for (std::size_t j = i + 1; j < out.size(); ++j) {
                int a = out[i].teamA, b = out[i].teamB;
                int c = out[j].teamA, d = out[j].teamB;
                double cur = out[i].cost + out[j].cost;
                double ac = pairCost(t, hist, a, c), bd = pairCost(t, hist, b, d);
                double ad = pairCost(t, hist, a, d), bc = pairCost(t, hist, b, c);
                if (ac + bd < cur - 1e-9 && ac + bd <= ad + bc) {
                    out[i].teamA = a; out[i].teamB = c; out[i].cost = ac;
                    out[j].teamA = b; out[j].teamB = d; out[j].cost = bd;
                    improved = 1;
                } else if (ad + bc < cur - 1e-9) {
                    out[i].teamA = a; out[i].teamB = d; out[i].cost = ad;
                    out[j].teamA = b; out[j].teamB = c; out[j].cost = bc;
                    improved = 1;
                }
            }
    }
    return out;
}

} // namespace arena
