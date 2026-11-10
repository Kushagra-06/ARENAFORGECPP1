# ArenaForge build (everything is C++17)
#   make test   native build of the engine + application layer, runs the test suite (with sanitizers)
#   make wasm   compile the same sources to WebAssembly and plain JavaScript (needs Emscripten)
#   make site   wasm + inline everything into one self-contained web/arenaforge.html

CXX      := g++
CXXFLAGS := -std=c++17 -O1 -g -Wall -Wextra -Iapp
SAN      := -fsanitize=address,undefined -fno-omit-frame-pointer

ENGINE_SRC := $(wildcard app/engine_*.cpp)
APP_SRC    := app/tournament.cpp app/arena_system.cpp app/snapshot.cpp
HEADERS    := $(wildcard app/*.hpp)

build:
	mkdir -p build

build/arenaforge_tests: $(ENGINE_SRC) $(APP_SRC) tests/tests.cpp $(HEADERS) | build
	$(CXX) $(CXXFLAGS) $(SAN) $(ENGINE_SRC) $(APP_SRC) tests/tests.cpp -o $@

test: build/arenaforge_tests
	./build/arenaforge_tests

# ---- WebAssembly: engine + application layer + web_api, all compiled as C++ ----
WASM_SRC   := $(ENGINE_SRC) $(APP_SRC) app/web_api.cpp
WASM_FLAGS := -O2 -fexceptions -sMODULARIZE=1 -sEXPORT_NAME=ArenaModule -sSINGLE_FILE=1 \
              -sFORCE_FILESYSTEM=1 -sALLOW_MEMORY_GROWTH=1 -sENVIRONMENT=web,node \
              -sEXPORTED_RUNTIME_METHODS=ccall,FS

wasm: build/arena.js build/arena_js.js

build/arena.js: $(WASM_SRC) $(HEADERS)
	mkdir -p build/wasm
	for f in $(WASM_SRC); do em++ -std=c++17 -O2 -fexceptions -Iapp -c $$f -o build/wasm/$$(basename $$f .cpp).o || exit 1; done
	em++ $(WASM_FLAGS) build/wasm/*.o -o $@

site: build/arena.js build/arena_js.js
	python3 web/build_site.py

# Same sources compiled to plain JavaScript: the page falls back to this
# when its host does not allow WebAssembly (content-security-policy).
JS_FLAGS := -O2 -fexceptions -sWASM=0 -sMODULARIZE=1 -sEXPORT_NAME=ArenaModuleJS -sSINGLE_FILE=1 \
            -sFORCE_FILESYSTEM=1 -sALLOW_MEMORY_GROWTH=1 -sENVIRONMENT=web,node \
            -sEXPORTED_RUNTIME_METHODS=ccall,FS

build/arena_js.js: build/arena.js
	em++ $(JS_FLAGS) build/wasm/*.o -o $@

clean:
	rm -rf build

.PHONY: test wasm site clean
