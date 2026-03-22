# Zen++
<img width="6912" height="3456" alt="Zen++" src="https://github.com/user-attachments/assets/5198415e-4daa-4125-a5fe-1798869a574a" />

A fast, lightweight interpreted language designed for competitive programming.

Zen++ features C-style syntax with curly braces, no semicolons, automatic type inference, and a standard library with common data structures out of the box.

**[Try it in your browser](https://zainmarshall.github.io/zen-plus-plus/)**

## Installation

### Interpreter

```bash
git clone https://github.com/zainmarshall/zen-plus-plus.git
cd zen-plus-plus
bash install.sh
```

This builds the interpreter and installs it to `/usr/local/bin`. Requires `make` and a C++17 compiler. After installing, run `zenpp` for the REPL or `zenpp file.zpp` to execute a file.

### VS Code Extension

Install **[Zen++ Language Support](https://marketplace.visualstudio.com/items?itemName=ZainMarshall.zenpp)** from the VS Code Marketplace for syntax highlighting.

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

// for loop — three forms
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

```zenpp
import std
```

**Math:** `min(a,b)` `max(a,b)` `abs(x)` `gcd(a,b)`

**Data Structures:** `Stack`, `Queue`, `Map`, `Set`, `DSU`, `PriorityQueue`, `Pair`, `Tuple`

### Built-in Functions

| Function | Description |
|----------|-------------|
| `print(...)` | Print values separated by spaces (no newline) |
| `println(...)` | Print values separated by spaces, with newline |
| `ord(s)` | ASCII code of first character of string |
| `chr(n)` | Single-character string from ASCII code |
| `parseInt(s)` | Parse a string as an integer |
| `read()` | Read integer from stdin |
| `readFloat()` | Read float from stdin |
| `readLine()` | Read line as string from stdin |
| `len(x)` | Length of string or vector |
| `push(v, x)` | Append to vector |
| `pop(v)` | Remove and return last element |

## Documentation

See the full [Language Reference](docs/language-reference.md) for complete documentation with examples.

## Samples & Docs

- Samples index: [`samples/README.md`](samples/README.md)
- Codeforces Round 2200: https://codeforces.com/contest/2200

## Examples

See the [`samples/`](samples/) directory for example programs.

## License

MIT

### Build & Run

```bash
# build
make

# run a program
./zenpp program.zpp

# run with stdin from file
./zenpp program.zpp < input.txt
```

### WebAssembly

```bash
# requires emscripten
emcc -std=c++17 -O2 \
  src/main.cpp src/lexer.cpp src/parser.cpp \
  -s MODULARIZE=1 -s EXPORT_NAME=Zenpp \
  -s EXPORTED_FUNCTIONS='["_zenpp_eval","_free"]' \
  -s EXPORTED_RUNTIME_METHODS='["cwrap","UTF8ToString"]' \
  -o docs/zenpp.js
```

## Optimization

Zen++ uses several optimization techniques in its interpreter and delivery pipeline. Run `make bench` to reproduce benchmarks.

### Flag-based control flow (replaced C++ exceptions)

`return`, `break`, and `continue` originally used C++ `throw`/`catch` for control flow. C++ exceptions are designed for error handling, not regular control flow — they're 100-1000x slower than normal returns due to stack unwinding.

Replaced with a global `Signal` enum. Return/break/continue set a flag; loops and function calls check it after each evaluation. All `try`/`catch` blocks removed from hot paths.

**Result:** Function calls went from 7.05s → 0.41s (**17x faster**). Composite benchmark 32.5s → 3.4s (**9.7x faster**).

### Arena allocator for AST nodes

Every AST node was individually heap-allocated with `new`. For a program with thousands of nodes, this means thousands of `malloc`/`free` calls with poor cache locality — nodes scattered across the heap.

Replaced with `ASTArena`, which allocates nodes from contiguous 4096-node blocks using placement `new`. Reduces allocation calls from ~N to ~N/4096. Nodes are physically adjacent in memory for better CPU cache performance.

### Zero external dependencies

The entire project — interpreter, web IDE, build system — has zero external dependencies. No npm packages, no pip packages, no third-party C++ libraries. The interpreter is pure C++17 stdlib. The web IDE is raw HTML/CSS/JS + WASM. This means:
- No dependency vulnerabilities
- No supply chain risk
- Minimal bundle size (468 KB WASM + 110 KB JS/CSS)

### Efficient data structures

- `std::unordered_map` for O(1) variable and function lookup
- `std::variant` for polymorphic `Value` type (no virtual dispatch overhead)
- `std::shared_ptr` for reference-counted maps/sets/objects (no GC pauses)
- Quicksort with median-of-three pivot selection (avoids O(n²) on sorted input)

### Benchmarks

| Benchmark | Iterations | Time |
|---|---|---|
| Simple loop (`x += 1`) | 1,000,000 | ~1.1s |
| Function calls (`add(x, 1)`) | 100,000 | ~0.4s |
| Variable read/write | 500,000 | ~1.1s |
| Vector push | 100,000 | ~0.2s |
| Composite (all above + fib(25)) | — | ~3.4s |
