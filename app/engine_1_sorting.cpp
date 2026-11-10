/* ArenaForge - C++ DSA engine
   Part 1 - Sorting: merge sort and quick sort (ranking & seeding) */
#include "arena_engine.hpp"

#include <utility>

namespace arena {
namespace {

/* strict ordering: higher score first, then lower id */
bool before(const RankEntry& x, const RankEntry& y) {
    if (x.score != y.score) return x.score > y.score;
    return x.id < y.id;
}

void mergeRange(std::vector<RankEntry>& a, std::vector<RankEntry>& tmp, std::size_t lo, std::size_t hi) {
    if (hi - lo < 2) return;                                    /* [lo, hi) */
    std::size_t mid = lo + (hi - lo) / 2;
    mergeRange(a, tmp, lo, mid);
    mergeRange(a, tmp, mid, hi);

    std::size_t i = lo, j = mid, k = lo;
    while (i < mid && j < hi) tmp[k++] = before(a[j], a[i]) ? a[j++] : a[i++];
    while (i < mid) tmp[k++] = a[i++];
    while (j < hi) tmp[k++] = a[j++];
    for (std::size_t x = lo; x < hi; ++x) a[x] = tmp[x];
}

void quickRange(std::vector<RankEntry>& a, long lo, long hi) {  /* [lo, hi] inclusive */
    while (lo < hi) {
        const RankEntry pivot = a[static_cast<std::size_t>(lo + (hi - lo) / 2)];
        long i = lo, j = hi;
        while (i <= j) {
            while (before(a[static_cast<std::size_t>(i)], pivot)) ++i;
            while (before(pivot, a[static_cast<std::size_t>(j)])) --j;
            if (i <= j) {
                std::swap(a[static_cast<std::size_t>(i)], a[static_cast<std::size_t>(j)]);
                ++i;
                --j;
            }
        }
        /* recurse on the smaller half, loop on the larger (bounded stack) */
        if (j - lo < hi - i) { quickRange(a, lo, j); lo = i; }
        else                 { quickRange(a, i, hi); hi = j; }
    }
}

} // namespace

void mergeSort(std::vector<RankEntry>& a) {
    if (a.size() < 2) return;
    std::vector<RankEntry> tmp(a.size());
    mergeRange(a, tmp, 0, a.size());
}

void quickSort(std::vector<RankEntry>& a) {
    if (a.size() < 2) return;
    quickRange(a, 0, static_cast<long>(a.size()) - 1);
}

void sortRanking(std::vector<RankEntry>& a, SortAlgo algo) {
    if (algo == SortAlgo::Merge) mergeSort(a);
    else                         quickSort(a);
}

} // namespace arena
