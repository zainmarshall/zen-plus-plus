CXX := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -pedantic

SRC_DIR := src
TEST_DIR := test
BUILD_DIR := build

TARGET := $(BUILD_DIR)/zenpp
SOURCES := $(SRC_DIR)/main.cpp $(SRC_DIR)/lexer.cpp $(SRC_DIR)/parser.cpp

DEFAULT_TEST ?= $(TEST_DIR)/test.zpp
RUN_ARG := $(word 2,$(MAKECMDGOALS))

.PHONY: help build run repl test test-all clean

help:
	@echo "Available targets:"
	@echo "  make build               - Build $(TARGET)"
	@echo "  make run                 - Start REPL"
	@echo "  make run path/to.x.zpp   - Run a .zpp file"
	@echo "  make repl                - Start REPL"
	@echo "  make test                - Validate test.zpp against output.txt"
	@echo "  make test-all            - Run all_features_test.zpp"
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

clean:
	rm -rf $(BUILD_DIR)

ifneq ($(filter run,$(MAKECMDGOALS)),)
ifneq ($(RUN_ARG),)
$(RUN_ARG):
	@:
endif
endif
