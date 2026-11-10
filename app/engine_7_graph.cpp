/* ArenaForge - C++ DSA engine
   Part 7 - Graph: scheduling conflicts + colouring */
#include "arena_engine.hpp"

#include <algorithm>

namespace arena {

Graph::Graph(int vertices) : adj_(static_cast<std::size_t>(vertices < 0 ? 0 : vertices)) {}

bool Graph::hasEdge(int u, int v) const {
    if (u < 0 || v < 0 || u >= size() || v >= size()) return false;
    const std::vector<int>& list = adj_[static_cast<std::size_t>(u)];
    return std::find(list.begin(), list.end(), v) != list.end();
}

bool Graph::addEdge(int u, int v) {
    if (u == v || u < 0 || v < 0 || u >= size() || v >= size()) return false;
    if (hasEdge(u, v)) return true;
    adj_[static_cast<std::size_t>(u)].push_back(v);
    adj_[static_cast<std::size_t>(v)].push_back(u);
    return true;
}

int Graph::degree(int v) const {
    return (v >= 0 && v < size()) ? static_cast<int>(adj_[static_cast<std::size_t>(v)].size()) : 0;
}

int Graph::color(std::vector<int>& colors) const {
    const std::size_t n = adj_.size();
    colors.assign(n, -1);
    if (n == 0) return 0;

    std::vector<std::size_t> order(n);
    for (std::size_t i = 0; i < n; ++i) order[i] = i;
    for (std::size_t i = 1; i < n; ++i) {                       /* insertion sort: degree descending */
        std::size_t v = order[i];
        std::size_t j = i;
        while (j > 0 && adj_[order[j - 1]].size() < adj_[v].size()) {
            order[j] = order[j - 1];
            --j;
        }
        order[j] = v;
    }

    int used = 0;
    std::vector<char> forbidden(n + 1);
    for (std::size_t v : order) {
        std::fill(forbidden.begin(), forbidden.end(), 0);
        for (int w : adj_[v]) {
            int c = colors[static_cast<std::size_t>(w)];
            if (c >= 0) forbidden[static_cast<std::size_t>(c)] = 1;
        }
        int c = 0;
        while (forbidden[static_cast<std::size_t>(c)]) ++c;
        colors[v] = c;
        used = std::max(used, c + 1);
    }
    return used;
}

Graph Graph::fromMatches(const std::vector<std::pair<int, int>>& m) {
    Graph g(static_cast<int>(m.size()));
    for (std::size_t i = 0; i < m.size(); ++i)
        for (std::size_t j = i + 1; j < m.size(); ++j)
            if (m[i].first == m[j].first || m[i].first == m[j].second ||
                m[i].second == m[j].first || m[i].second == m[j].second)
                g.addEdge(static_cast<int>(i), static_cast<int>(j));
    return g;
}

SchedulePlan planSchedule(const std::vector<std::pair<int, int>>& pairs, int stations) {
    SchedulePlan plan;
    const int m = static_cast<int>(pairs.size());
    if (m == 0) return plan;
    if (stations < 1) stations = 1;

    Graph g = Graph::fromMatches(pairs);
    plan.colorCount = g.color(plan.colors);
    for (int u = 0; u < m; ++u)
        for (int v : g.neighbors(u))
            if (u < v) plan.edges.emplace_back(u, v);

    plan.slots.assign(static_cast<std::size_t>(m), 0);
    int slot = 0;
    for (int c = 0; c < plan.colorCount; ++c) {
        int inSlot = 0;
        bool any = false;
        for (int i = 0; i < m; ++i) {
            if (plan.colors[static_cast<std::size_t>(i)] != c) continue;
            if (inSlot == stations) { ++slot; inSlot = 0; }
            plan.slots[static_cast<std::size_t>(i)] = slot;
            ++inSlot;
            any = true;
        }
        if (any) ++slot;
    }
    plan.slotCount = slot;
    return plan;
}

} // namespace arena
