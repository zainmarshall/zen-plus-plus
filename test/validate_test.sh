#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
cd "$ROOT_DIR"

echo "[1/3] Compiling zen++..."
mkdir -p build
g++ -std=c++17 -Wall -Wextra -pedantic src/main.cpp src/lexer.cpp src/parser.cpp -o build/zenpp

echo "[2/3] Running test.zpp..."
./build/zenpp test/test.zpp > build/actual_output.txt

echo "[3/3] Comparing output..."
if diff -u test/output.txt build/actual_output.txt; then
  echo "PASS: test.zpp output matches output.txt"
  rm -f build/actual_output.txt
else
  echo "FAIL: output mismatch"
  echo "See build/actual_output.txt for actual output"
  exit 1
fi
