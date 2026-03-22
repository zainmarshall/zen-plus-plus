#!/bin/bash
# Zen++ benchmark suite
# Usage: make bench

set -e

ZENPP="${1:-build/zenpp}"
DIR="$(dirname "$0")"

if [ ! -f "$ZENPP" ]; then
    echo "ERROR: $ZENPP not found. Run 'make build' first."
    exit 1
fi

run_bench() {
    local name="$1"
    local file="$2"

    # Use /usr/bin/time for portable, parseable output
    local elapsed
    elapsed=$( { /usr/bin/time -p "$ZENPP" "$file" > /dev/null; } 2>&1 | grep real | awk '{print $2}')

    printf "  %-30s %ss\n" "$name" "$elapsed"
}

echo ""
echo "Zen++ Benchmark Suite"
echo "====================="
echo ""

run_bench "Simple loop (1M)" "$DIR/bench_loop.zpp"
run_bench "Function calls (100K)" "$DIR/bench_fn.zpp"
run_bench "Variable read/write (500K)" "$DIR/bench_var.zpp"
run_bench "Vector push (100K)" "$DIR/bench_vec.zpp"
echo "  ---"
run_bench "Composite (all + fib(25))" "$DIR/bench_composite.zpp"

echo ""
