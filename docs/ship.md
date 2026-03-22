## Zen++ v0.2.0

A fast interpreted language for competitive programming. C-style syntax, no semicolons, auto type inference, stdlib with data structures and algorithms.

**[Try it in your browser](https://zainmarshall.github.io/zen-plus-plus/)**

### New since v0.1.0

**Language:** bulk I/O (`read(n)`), type casting, ternary, negative indexing, multiple assignment & swap, lambdas, slicing (`v[1:4]`, `v[::-1]`), tuple unpacking, default args, bitwise shifts, string builtins (`replace`, `upper`, `lower`, `trim`, `contains`, `startswith`, `endswith`, `substr`), multi-dim `fill`, f-strings, chained comparisons (`1 < x < 10`), destructuring, `min(v)`/`max(v)`, `all`/`any`, `reverse`, `unique`, `sorted`, `flatten`, `zip`, `rand`, `randvec`, string iteration.

**Stdlib:** `lcm`, `modpow`, `prefix`, `sum`, `lowerBound`, `upperBound`, `binarySearch`, `dijkstra`, `bfs`, graph builtins, `FenwickTree`, `SegTree`, `MinPriorityQueue`.

**11x faster interpreter** - flag-based control flow, arena allocator, scope frame reuse, constant folding. [Benchmarks](https://github.com/zainmarshall/zen-plus-plus#optimization).

**Web IDE** - 6 themes, intellisense with signatures, f-string highlighting, 22 samples.

**VS Code v0.2.3** - f-string highlighting, run button (Cmd+Enter), file icons.

Runs as CLI (`zenpp file.zpp`), [browser](https://zainmarshall.github.io/zen-plus-plus/), or [VS Code](https://marketplace.visualstudio.com/items?itemName=ZainMarshall.zenpp).
