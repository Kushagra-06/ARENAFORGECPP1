# ArenaForge: esports tournament, ranking and smart matchmaking system

Team GameDev, DSCPP-III-2026-T104. A C++ DSA engine, a C++ application layer, and a website that runs both. Everything is C++17.

## Layout

| Folder | Language | What it holds |
|---|---|---|
| `app/` | C++ | The DSA engine (`arena_engine.hpp` and `engine_1_sorting.cpp` ... `engine_7_graph.cpp`), `models.hpp` (User, Player, Admin, Game, Team, Match, exceptions) and the application layer, below. |
| `tests/` | C++ | `tests.cpp`: 622 checks on the engine and the application layer. |
| `web/` | HTML/JS | The site template, the build script, and the finished single-file site `arenaforge.html`. |

Engine: one header, `arena_engine.hpp`, and seven modules. Every structure is written by hand (no `std::sort`, `std::priority_queue` or `std::unordered_map`) and is a class that owns its memory, so no code outside the engine calls `delete`. Application files: `tournament.hpp/.cpp` (abstract `Tournament`; `KnockoutTournament`, `RoundRobinTournament`, `SwissTournament`; `Leaderboard`; Elo), `arena_system.hpp/.cpp` (controller, hash-table search, waitlist, file handling), `snapshot.cpp` and `json.hpp` (state for the web page), `web_api.cpp` (the `extern "C"` functions the page calls).

## How the report maps to the code

| Report item | Where |
|---|---|
| Sorting for ranking and seeding | `mergeSort`, `quickSort`, `sortRanking`; used by `Tournament::start`, `Leaderboard`, standings |
| Heap for matchmaking | `PairHeap`, `fairScore`, `matchmake`; used by `SwissTournament::createRound` |
| Tree for brackets | `BracketTree`, `BracketNode`; used by `KnockoutTournament` |
| Hash table for search | `NameIndex`; team names and gamer tags in `ArenaSystem` |
| Queue for registration and waitlist | `WaitQueue`; `Tournament::registerTeam` / `withdrawTeam` |
| Linked list for match history | `HistoryLog`; also feeds the repeat-opponent penalty |
| Graph for scheduling conflicts | `Graph`; `planSchedule` colours each batch of matches into time slots |
| Inheritance, abstraction, polymorphism | `User` to `Player`/`Admin`; abstract `Tournament` with three formats |
| Exception handling | `ArenaError` family; admin-only actions throw `PermissionError` |
| File handling | `ArenaSystem::save/load` write and read `Players.dat`, `Teams.dat`, `Tournaments.dat`, `Matches.dat`, `Results.dat`, `Rankings.dat`, `History.dat` |

## Build and run

```
make test    # native build with AddressSanitizer + UBSan, runs all 622 checks
make wasm    # compile the same C++ sources to WebAssembly and to plain JavaScript (needs Emscripten)
make site    # inline both into web/arenaforge.html
```

Open `web/arenaforge.html` in any browser. No server is needed. The page runs the WebAssembly build and falls back to the JavaScript build if WebAssembly is blocked.

## Rules the system enforces

- Teams must be full (the game's players per team) and match the tournament's game to register.
- A full tournament puts extra teams on a waitlist queue; a withdrawal promotes the first in line.
- Draws and negative scores are rejected. A match can't be played twice.
- Rosters lock while a team is in a live tournament.
- Creating tournaments, registering teams and entering results need admin mode.
- Ratings are Elo (K = 32). Fair Matchmaking Score = rating gap + 1000 per previous meeting.
