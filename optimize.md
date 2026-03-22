# Optimization Plan

## Current Performance Baseline

| Benchmark | Iterations | Time | Ops/sec |
|---|---|---|---|
| Simple loop (`x += 1`) | 1M | 1.02s | ~1M/s |
| Variable read/write | 500K | 1.12s | ~450K/s |
| Function calls | 100K | **7.05s** | **~14K/s** |
| Vector push | 100K | 0.15s | ~667K/s |

Function calls are 70x slower than simple operations. That's the #1 bottleneck.

---

## Interpreter Optimizations

### 1. Replace exception-based control flow with flags
**Category:** Efficient algorithms
**Expected speedup:** 5-10x on function calls

Currently `return`, `break`, and `continue` all use `throw`/`catch`:
```cpp
// current — every return throws an exception
case NodeType::RETURN:
    throw ReturnSignal{evaluate(node->left)};

// every loop body wraps in try/catch
try {
    result = evaluate(body);
} catch (BreakSignal&) { break; }
  catch (ContinueSignal&) { continue; }
```

C++ exceptions are 100-1000x slower than normal returns. Replace with a flag/enum:
```cpp
enum class Signal { NONE, RETURN, BREAK, CONTINUE };
Signal lastSignal = Signal::NONE;
Value returnValue;

// return just sets flag
case NodeType::RETURN:
    returnValue = evaluate(node->left);
    lastSignal = Signal::RETURN;
    return returnValue;

// loops check flag instead of catching
result = evaluate(body);
if (lastSignal == Signal::BREAK) { lastSignal = Signal::NONE; break; }
if (lastSignal == Signal::CONTINUE) { lastSignal = Signal::NONE; continue; }
```

### 2. Pre-allocate scope frames
**Category:** Reduce memory usage
**Expected speedup:** 3-5x on function calls

Currently every function call allocates a new `unordered_map` and every return destroys it:
```cpp
scopes.push_back({});                    // malloc
scopes.back()[param] = value;            // hash insert per param
// ... evaluate ...
scopes.pop_back();                       // free
```

Replace with a flat variable array + scope depth tracking. Pre-allocate a fixed-size frame pool so function calls don't malloc/free on every invocation.

### 3. Constant folding
**Category:** Efficient algorithms
**Expected speedup:** 1.5-2x on arithmetic-heavy code

Evaluate constant expressions at parse time:
```
// before: evaluates 1+2 every iteration
for i 1000000 { x = 1 + 2 + 3 }

// after: folded to 6 at parse time
for i 1000000 { x = 6 }
```

Fold in the parser: if both children of an arithmetic node are INT/FLOAT literals, compute the result and return a literal node.

### 4. Arena allocator for AST nodes
**Category:** Reduce memory usage
**Expected speedup:** 1.2-1.5x overall

Currently every AST node is a separate heap allocation:
```cpp
new ASTNode(NodeType::ADD, left, right);  // individual malloc
```

Replace with an arena that allocates nodes from a contiguous block:
```cpp
struct ASTArena {
    std::vector<ASTNode> pool;
    ASTNode* alloc(...) { pool.emplace_back(...); return &pool.back(); }
};
```

Reduces thousands of malloc calls to ~1. Improves cache locality since nodes are contiguous in memory.

### 5. Avoid unnecessary Value copies
**Category:** Reduce memory usage
**Expected speedup:** 1.5-2x on string/vector operations

`evaluate()` returns `Value` by value. `getVariable()` copies the value out. For strings and vectors this triggers deep copies.

Where possible, return `Value*` or `const Value&` to avoid copying. For example, `getVariable` can return a pointer when the caller only needs to read.

### 6. Index-based variable lookup
**Category:** Efficient algorithms
**Expected speedup:** 2-3x on variable access

Currently variable lookup is string-based:
```cpp
Value* findVariable(const std::string& name) {
    for (auto it = scopes.rbegin(); it != scopes.rend(); ++it) {
        auto found = it->find(name);  // string hash per scope level
        ...
    }
}
```

Assign each variable a numeric index at parse time. Replace hash map lookups with direct array indexing.

---

## Asset / Delivery Optimizations

### 7. WASM bundle size optimization
**Category:** Optimize asset sizes

Current sizes:
- `zenpp.wasm`: 458 KB
- `zenpp.js`: 70 KB
- `app.js`: 33 KB
- `style.css`: 7.1 KB
- **Total: ~568 KB**

Optimizations:
- Switch emscripten from `-O2` to `-Oz` (optimize for size)
- gzip/brotli compress the WASM binary
- Minify `app.js` and `style.css`

### 8. Lazy WASM loading
**Category:** Lazy loading

Currently the WASM module loads on page open. Move it to load on first "Run" click so the page is interactive immediately. Show a loading indicator while the module initializes.

---

## Implementation Priority

| # | Optimization | Impact | Effort | Status |
|---|---|---|---|---|
| 1 | Exception → flag control flow | Highest | Medium | **Done** — 17x faster function calls |
| 2 | Pre-allocate scope frames | High | Medium | |
| 3 | Constant folding | Medium | Easy | |
| 4 | Arena allocator | Medium | Easy | **Done** — 4096-node block allocator |
| 5 | WASM size (`-Oz` + compress) | Medium | Easy | |
| 6 | Lazy WASM loading | Low-Medium | Easy | |
| 7 | Avoid Value copies | Medium | Hard | |
| 8 | Index-based variables | High | Hard | |

## Results So Far

Composite benchmark (1M loop + 100K fn calls + 500K var ops + fib(25)):
- **Before:** 32.53s
- **After:** 3.35s
- **Speedup: 9.7x**

Run `make bench` to reproduce.
