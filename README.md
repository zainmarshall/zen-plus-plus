# Zen++
<img width="6912" height="3456" alt="Zen++" src="https://github.com/user-attachments/assets/5198415e-4daa-4125-a5fe-1798869a574a" />

A fast, lightweight interpreted language designed for competitive programming.

Zen++ features C-style syntax with curly braces, no semicolons, automatic type inference, and a standard library with common data structures out of the box.

**[Try it in your browser](https://zainmarshall.github.io/zen-plus-plus/)**

## Table of Contents

- [Installation](#installation)
- [Language Overview](#language-overview)
- [Standard Library](#standard-library)
- [Built-in Functions](#built-in-functions)
- [Documentation](#documentation)
- [Build & Run](#build--run)
- [Optimization](#optimization)

## Installation

### Interpreter

```bash
# Fresh install
git clone https://github.com/zainmarshall/zen-plus-plus.git && cd zen-plus-plus && bash install.sh

# Update
cd zen-plus-plus && git pull && bash install.sh
```

This builds the interpreter and installs it to `/usr/local/bin`. Requires `make` and a C++17 compiler. After installing, run `zenpp` for the REPL or `zenpp file.zpp` to execute a file.

### VS Code Extension

Install **[Zen++ Language Support](https://marketplace.visualstudio.com/items?itemName=ZainMarshall.zenpp)** from the VS Code Marketplace for syntax highlighting, f-string support, and a run button (Cmd+Enter).

Or search "Zen++" in the VS Code extensions panel.

## Language Overview

### Variables

Variables are created on first assignment. No declarations needed.

```zenpp
x = 42
name = "zen"
pi = 3.14
```

### Data Types

| Type | Example |
|------|---------|
| Integer | `42`, `-7` |
| Float | `3.14`, `0.5` |
| String | `"hello"` |
| Boolean | `true`, `false` |
| Vector | `[1, 2, 3]` |
| Map | `map()` |
| Set | `set()` |

### Control Flow

```zenpp
// if / else if / else
if x > 0 {
    println("positive")
} else if x == 0 {
    println("zero")
} else {
    println("negative")
}

// while loop
while n > 0 {
    n--
}

// for loop - three forms
for i 5 { }              // i = 0, 1, 2, 3, 4
for i 2 5 { }            // i = 2, 3, 4
for i 10 0 -1 { }        // i = 10, 9, 8, ..., 1
```

### Functions

```zenpp
fn gcd(a, b) {
    while b != 0 {
        t = a % b
        a = b
        b = t
    }
    return a
}
```

### Operators

Arithmetic: `+` `-` `*` `/` `%` `**`
Comparison: `==` `!=` `<` `>` `<=` `>=`
Logical: `&&` `||` `!`
Bitwise: `&` `|` `^`
Unary: `++` `--` `!` (factorial, postfix)
Assignment: `=` `+=` `-=` `*=` `/=` `%=` `**=`

### Vectors

```zenpp
v = [1, 2, 3]
push(v, 4)
println(v[0])       // 1
println(len(v))     // 4
println(pop(v))     // 4
```

### Strings

```zenpp
s = "hello" + " world"
println(len(s))       // 11
println(s[0])         // ascii value of 'h'
```

### Maps & Sets

```zenpp
m = map()
m.set("key", 100)
println(m.get("key", 0))   // 100
println(m.has("key"))       // 1

s = set()
s.add(5)
println(s.has(5))           // 1
```

### Structs

```zenpp
struct Point {
    fn init(x, y) {
        self.x = x
        self.y = y
    }
    fn dist() {
        return (self.x ** 2 + self.y ** 2)
    }
}

p = Point()
p.init(3, 4)
println(p.dist())   // 25
```

### Standard Library

The standard library is loaded automatically - no import needed.

**Math:** `min` `max` `abs` `gcd` `lcm` `modpow` `prefix` `sum`

**Search:** `binarySearch` `lowerBound` `upperBound`

**Graph:** `dijkstra` `bfs` `graph` `wgraph` `dgraph` `dwgraph`

**Data Structures:** `Stack`, `Queue`, `DSU`, `PriorityQueue`, `MinPriorityQueue`, `FenwickTree`, `SegTree`, `Pair`, `Tuple`

### Built-in Functions

| Function | Description |
|----------|-------------|
| `print(...)` / `println(...)` | Print values (with/without newline) |
| `read()` / `read(n)` | Read int(s) from stdin |
| `readFloat()` / `readLine()` | Read float or line from stdin |
| `len(x)` | Length of string or vector |
| `push(v, x)` / `pop(v)` | Append/remove last element |
| `sort(v)` / `sorted(v)` | Sort in-place / return sorted copy |
| `reverse(v)` | Reverse vector or string in-place |
| `unique(v)` | Remove consecutive duplicates |
| `find(v, x)` / `count(v, x)` | Find index / count occurrences |
| `fill(n, val)` | Create vector of n copies |
| `flatten(v)` / `zip(a, b)` | Flatten nesting / pair up vectors |
| `rand(lo, hi)` / `randvec(n, lo, hi)` | Random int / random vector |
| `split(s, d)` / `join(v, d)` | Split/join strings |
| `str(x)` / `int(x)` / `float(x)` | Type casting |
| `ord(s)` / `chr(n)` | ASCII conversions |
| `exit()` | Terminate program |

## Documentation

- [Language Reference](docs/language-reference.md) - full docs with examples
- [Samples](samples/README.md) - 22 example programs including [Codeforces Round 2200](https://codeforces.com/contest/2200) solutions
- [Devlogs](docs/devlogs.md) - development history from day 1

## Build & Run

```bash
# build
make

# run a program
./zenpp program.zpp

# run with stdin from file
./zenpp program.zpp < input.txt

# build WebAssembly (requires emscripten)
make build-web
```

## License

MIT

## Optimization

Zen++ uses several optimization techniques in its interpreter and delivery pipeline. Run `make bench` to reproduce benchmarks.

### Flag-based control flow

Replaced C++ `throw`/`catch` for `return`/`break`/`continue` with a `Signal` enum. Exceptions are 100-1000x slower than normal returns - this drove a 17x speedup on function calls.

### Arena allocator for AST nodes

AST nodes allocated from contiguous 4096-node blocks via `ASTArena` instead of individual `new`/`delete`. Better cache locality, fewer malloc calls.

### Scope frame reuse

Function calls reuse pre-allocated `unordered_map` frames instead of allocating/freeing on every call. The cleared frame keeps its bucket array, avoiding repeated allocation.

### Constant folding

Constant integer arithmetic (`1 + 2 + 3`) is evaluated at parse time and replaced with a literal node.

### Zero external dependencies

The entire project - interpreter, web IDE, build system - has zero external dependencies. No npm packages, no pip packages, no third-party C++ libraries. The interpreter is pure C++17 stdlib. The web IDE is raw HTML/CSS/JS + WASM.

### Efficient data structures

- `std::unordered_map` for O(1) variable and function lookup
- `std::variant` for polymorphic `Value` type (no virtual dispatch overhead)
- `std::shared_ptr` for reference-counted maps/sets/objects (no GC pauses)
- Quicksort with median-of-three pivot selection (avoids O(n²) on sorted input)

### Benchmarks

Run `make bench` to reproduce.

| Benchmark | Original | + Flags & Arena | + Scope Reuse & Folding |
|---|---|---|---|
| Simple loop (1M) | 1.08s | 1.07s | **0.95s** |
| Function calls (100K) | 7.53s | 0.39s | **0.36s** |
| Variable read/write (500K) | 1.14s | 1.13s | **1.00s** |
| Vector push (100K) | 0.16s | 0.15s | **0.14s** |
| **Composite (all + fib(25))** | **33.67s** | **3.39s** | **3.05s** |

**Total speedup: 11x** on the composite benchmark.
