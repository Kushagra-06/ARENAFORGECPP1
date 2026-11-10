/*
 ArenaForge - C++ DSA engine
 ---------------------------------------------------------------------------
 Every data structure and algorithm is written by hand (no std::sort,
 std::priority_queue, std::unordered_map ...) and wrapped in a small C++ class
 that owns its memory (RAII), so the rest of the program never calls delete or
 checks for null by hand.

   1. Sorting       engine_1_sorting.cpp     merge sort / quick sort   (ranking & seeding)
   2. Heap          engine_2_heap.cpp        PairHeap + fair matchmaking
   3. Tree          engine_3_tree.cpp        BracketTree               (knockout bracket)
   4. Hash table    engine_4_hashtable.cpp   NameIndex                 (fast name search)
   5. Queue         engine_5_queue.cpp       WaitQueue                 (registration waitlist)
   6. Linked list   engine_6_linkedlist.cpp  HistoryLog                (match history)
   7. Graph         engine_7_graph.cpp       Graph + planSchedule      (scheduling conflicts)
 */
#pragma once

#include <cstddef>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace arena {

inline constexpr int kBye = -1;                 /* empty bracket slot (team advances automatically) */
inline constexpr int kTbd = -2;                 /* winner not decided yet */
inline constexpr double kRepeatPenalty = 1000.0;

/* ============================ 1. SORTING ============================ */
struct RankEntry {
    int id;                                     /* team id */
    double score;                               /* rating / rank key */
};

enum class SortAlgo { Merge = 0, Quick = 1 };

/* Both sorts order by score DESCENDING; ties are broken by the lower id. */
void mergeSort(std::vector<RankEntry>& a);      /* stable, O(n log n) */
void quickSort(std::vector<RankEntry>& a);      /* in place, O(n log n) on average */
void sortRanking(std::vector<RankEntry>& a, SortAlgo algo);

/* ============================ 6. LINKED LIST ======================== */
struct HistoryRecord {
    int matchId = 0, round = 0;
    int teamA = 0, teamB = 0;
    int scoreA = 0, scoreB = 0;
    int winner = 0;
};

/* Singly linked list with a tail pointer: append is O(1), reading is head -> tail. */
class HistoryLog {
public:
    HistoryLog() = default;
    ~HistoryLog();
    HistoryLog(const HistoryLog&) = delete;
    HistoryLog& operator=(const HistoryLog&) = delete;

    void append(const HistoryRecord& rec);
    int meetings(int a, int b) const;           /* how many times a met b (either order) */
    int count() const { return count_; }
    std::vector<HistoryRecord> records() const; /* chronological order */

private:
    struct Node {
        HistoryRecord rec;
        Node* next = nullptr;
    };
    Node* head_ = nullptr;
    Node* tail_ = nullptr;
    int count_ = 0;
};

/* ====================== 2. HEAP / PRIORITY QUEUE ==================== */
struct MatchPair {
    int teamA = 0, teamB = 0;
    double cost = 0.0;                          /* Fair Matchmaking Score: LOWER = better match */
};

/* Binary min-heap of candidate pairs, cheapest on top. */
class PairHeap {
public:
    explicit PairHeap(int capacityHint = 16);
    void push(const MatchPair& p);
    bool pop(MatchPair& out);                   /* false when empty */
    int size() const { return static_cast<int>(data_.size()); }

private:
    std::vector<MatchPair> data_;
};

/* Fair Matchmaking Score = |ratingA - ratingB| + penalty * previous meetings */
double fairScore(double ratingA, double ratingB, int previousMeetings);

/* Pairs teams from the cheapest candidate pair upward (heap), then improves the
   result with a 2-opt pass. An odd team is left unpaired. */
std::vector<MatchPair> matchmake(const std::vector<RankEntry>& teams, const HistoryLog& history,
                                 std::size_t maxPairs);

/* ============================ 3. TREE (BRACKET) ===================== */
struct BracketNode {
    int nodeId = -1;                            /* > 0 for matches, -1 for leaves */
    int round = 0;                              /* 0 = leaf, 1 = first round, ... */
    int teamId = kTbd;                          /* team id, kBye or kTbd */
    std::unique_ptr<BracketNode> left, right;
};

class BracketTree {
public:
    BracketTree() = default;
    BracketTree(const BracketTree&) = delete;
    BracketTree& operator=(const BracketTree&) = delete;

    void clear() { root_.reset(); }
    bool build(const std::vector<int>& seeded);             /* seeded[0] = seed 1 */
    const BracketNode* root() const { return root_.get(); }

    /* matches whose two participants are known but that have no winner yet */
    std::vector<BracketNode*> ready();
    bool reportWinner(int nodeId, int winner);              /* false if the result does not fit */
    int champion() const;                                   /* team id or kTbd */

private:
    std::unique_ptr<BracketNode> root_;
};

/* ============================ 4. HASH TABLE ========================= */
/* Separate chaining, djb2 hash, case-insensitive keys. */
class NameIndex {
public:
    explicit NameIndex(int buckets = 64);
    ~NameIndex() { clear(); }
    NameIndex(const NameIndex&) = delete;
    NameIndex& operator=(const NameIndex&) = delete;

    void put(const std::string& key, int value);            /* insert or update */
    std::optional<int> find(const std::string& key) const;
    bool remove(const std::string& key);
    int count() const { return count_; }
    void clear();

private:
    struct Node {
        std::string key;
        int value;
        std::unique_ptr<Node> next;
    };
    std::size_t bucketOf(const std::string& key) const;
    std::vector<std::unique_ptr<Node>> buckets_;
    int count_ = 0;
};

/* ============================ 5. QUEUE ============================== */
/* FIFO queue on a singly linked list (registration waitlist). */
class WaitQueue {
public:
    WaitQueue() = default;
    ~WaitQueue();
    WaitQueue(const WaitQueue&) = delete;
    WaitQueue& operator=(const WaitQueue&) = delete;

    void enqueue(int v);
    bool dequeue(int& out);                     /* false when empty */
    bool peek(int& out) const;
    bool remove(int v);                         /* leave from the middle of the line */
    bool contains(int v) const;
    int size() const { return size_; }
    std::vector<int> toVector() const;

private:
    struct Node {
        int value;
        Node* next;
    };
    Node* head_ = nullptr;
    Node* tail_ = nullptr;
    int size_ = 0;
};

/* ============================ 7. GRAPH ============================== */
/* Undirected graph on adjacency lists. */
class Graph {
public:
    explicit Graph(int vertices);

    int size() const { return static_cast<int>(adj_.size()); }
    bool addEdge(int u, int v);                 /* false for self loops / out of range */
    bool hasEdge(int u, int v) const;
    int degree(int v) const;
    const std::vector<int>& neighbors(int v) const { return adj_[static_cast<std::size_t>(v)]; }

    /* Welsh-Powell greedy colouring. colors[v] in [0, k). Returns k. */
    int color(std::vector<int>& colors) const;

    /* Vertex i = match i; an edge joins two matches that share a team. */
    static Graph fromMatches(const std::vector<std::pair<int, int>>& matches);

private:
    std::vector<std::vector<int>> adj_;
};

struct SchedulePlan {
    std::vector<int> colors;                    /* graph colour of each match */
    int colorCount = 0;                         /* colours used (Welsh-Powell result) */
    std::vector<int> slots;                     /* time slot of each match */
    int slotCount = 0;
    std::vector<std::pair<int, int>> edges;     /* conflicting match pairs (i < j) */
};

/* Two matches conflict when they share a team. Each colour is a set of matches
   that may run together; `stations` is how many matches can be played at the same
   time, so a colour class bigger than that is spread over consecutive slots. */
SchedulePlan planSchedule(const std::vector<std::pair<int, int>>& pairs, int stations);

} // namespace arena
