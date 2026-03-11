# ZEN++ Stdlib

Use `import std` to load the stdlib bundle listed in `stdlib/manifest.txt`.

## Data Structures

- `Stack` and `Queue` (vector-backed)
- `Map` and `Set` (backed by native hash tables)
- `DSU` (disjoint set union)
- `PriorityQueue` (max-heap) and `MinPriorityQueue` (min-heap)
- `Pair` and `Tuple`

### Map / Set

Use the stdlib structs (native hash tables underneath):

```zenpp
import std

map m    // sugar for: m = Map()
m.init()
m.set("a", 5)
println(m.get("a", 0))

set s
s.init()
s.add(10)
println(s.has(10))
```

Methods:
- `m.init()`, `m.set(key, value)`, `m.get(key, default)`, `m.has(key)`, `m.remove(key)`, `m.size()`
- `s.init()`, `s.add(key)`, `s.has(key)`, `s.remove(key)`, `s.size()`
