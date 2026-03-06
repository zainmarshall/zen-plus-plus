CXX := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -pedantic

SRC_DIR := src
TEST_DIR := test
BUILD_DIR := build

TARGET := $(BUILD_DIR)/zenpp
SOURCES := $(SRC_DIR)/main.cpp $(SRC_DIR)/lexer.cpp $(SRC_DIR)/parser.cpp

DEFAULT_TEST ?= $(TEST_DIR)/test.zpp
RUN_ARG := $(word 2,$(MAKECMDGOALS))

.PHONY: help build run repl test test-all build-web run-web test-web clean

help:
	@echo "Available targets:"
	@echo "  make build               - Build $(TARGET)"
	@echo "  make run                 - Start REPL"
	@echo "  make run path/to.x.zpp   - Run a .zpp file"
	@echo "  make repl                - Start REPL"
	@echo "  make test                - Validate test.zpp against output.txt"
	@echo "  make test-all            - Run all_features_test.zpp"
	@echo "  make build-web           - Build web/zenpp.js + web/zenpp.wasm (emscripten)"
	@echo "  make run-web             - Serve web/ on localhost:8080"
	@echo "  make test-web            - Run web smoke + runtime wasm test"
	@echo "  make clean               - Remove build artifacts"

build: $(TARGET)

$(TARGET): $(SOURCES)
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(SOURCES) -o $(TARGET)

run: build
	@if [ -n "$(RUN_ARG)" ]; then \
		$(TARGET) "$(RUN_ARG)"; \
	else \
		$(TARGET); \
	fi

repl: build
	$(TARGET)

test: build
	@bash $(TEST_DIR)/validate_test.sh

test-all: build
	$(TARGET) $(TEST_DIR)/all_features_test.zpp

build-web:
	@mkdir -p web
	emcc -std=c++17 -O2 \
	  src/main.cpp src/lexer.cpp src/parser.cpp \
	  -s MODULARIZE=1 \
	  -s EXPORT_NAME=Zenpp \
	  -s EXPORTED_FUNCTIONS='["_zenpp_eval","_free"]' \
	  -s EXPORTED_RUNTIME_METHODS='["cwrap","UTF8ToString"]' \
	  -o web/zenpp.js

run-web:
	@cd web && python3 -m http.server 8080

test-web: build-web
	@bash $(TEST_DIR)/web_smoke.sh
	@node $(TEST_DIR)/web_runtime_test.js

clean:
	rm -rf $(BUILD_DIR)

ifneq ($(filter run,$(MAKECMDGOALS)),)
ifneq ($(RUN_ARG),)
$(RUN_ARG):
	@:
endif
endif
