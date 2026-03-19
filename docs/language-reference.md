# Zen++ Language Reference

Complete reference for the Zen++ programming language.

## Table of Contents

- [Basics](#basics)
- [Data Types](#data-types)
- [Variables](#variables)
- [Operators](#operators)
- [Control Flow](#control-flow)
- [Functions](#functions)
- [Strings](#strings)
- [Vectors](#vectors)
- [Maps](#maps)
- [Sets](#sets)
- [Structs](#structs)
- [Imports & Standard Library](#imports--standard-library)
- [Built-in Functions](#built-in-functions)
- [Comments](#comments)
- [Examples](#examples)

---

## Basics

Zen++ uses curly-brace blocks, no semicolons, and whitespace-separated tokens. Programs are executed top-to-bottom. Functions can be called before they are defined.

```zenpp
x = 42
println(x)
```

## Data Types

### Integers

64-bit signed integers.

```zenpp
a = 42
b = -100
c = 0
```

### Floats

Double-precision floating point.

```zenpp
pi = 3.14
half = 0.5
```

### Strings

Double-quoted, with escape sequences `\n`, `\t`, `\r`, `\\`, `\"`.

```zenpp
greeting = "hello\nworld"
```

### Booleans

`true` evaluates to `1`, `false` to `0`.

```zenpp
flag = true
println(flag + 1)   // 2
```

### Vectors

Dynamic arrays. Can be nested.

```zenpp
v = [1, 2, 3]
matrix = [[1, 0], [0, 1]]
empty = []
```

### Maps

Hash tables with integer or string keys.

```zenpp
m = map()
m.set("name", "zen")
```

### Sets

Hash sets with integer or string keys.

```zenpp
s = set()
s.add(42)
```

## Variables

Created on first assignment. No type declarations. Function-scoped within functions, global at the top level.

```zenpp
x = 10
x = "now a string"   // reassignment with different type is fine

// multiple assignment
a, b = 1, 2

// swap
a, b = b, a

// use _ to discard
_, b = 0, 42
```

## Operators

### Precedence (highest to lowest)

| Precedence | Operators | Description |
|------------|-----------|-------------|
| 1 | `()` `[]` `.` `x++` `x--` `x!` | Grouping, index, access, postfix |
| 2 | `!x` `++x` `--x` `-x` | Prefix unary |
| 3 | `**` | Exponentiation (right-associative) |
| 4 | `*` `/` `%` | Multiplicative |
| 5 | `+` `-` | Additive |
| 6 | `==` `!=` `<` `>` `<=` `>=` | Comparison |
| 7 | `&` | Bitwise AND |
| 8 | `^` | Bitwise XOR |
| 9 | `\|` | Bitwise OR |
| 10 | `&&` | Logical AND |
| 11 | `\|\|` | Logical OR |
| 12 | `? :` | Ternary |
| 13 | `=` `+=` `-=` `*=` `/=` `%=` `**=` `&=` `\|=` `^=` | Assignment |

### Arithmetic

```zenpp
println(7 + 3)     // 10
println(7 - 3)     // 4
println(7 * 3)     // 21
println(7 / 3)     // 2 (integer division)
println(7 % 3)     // 1
println(2 ** 10)   // 1024
```

### Factorial

The postfix `!` operator computes the factorial of a positive integer.

```zenpp
println(5!)   // 120
println(6!)   // 720
```

### Increment / Decrement

```zenpp
x = 5
x++          // x is now 6
x--          // x is now 5
++x          // x is now 6
--x          // x is now 5
```

### Compound Assignment

```zenpp
x = 10
x += 5      // 15
x -= 3      // 12
x *= 2      // 24
x /= 4      // 6
x %= 5      // 1
x **= 3     // 1
```

### Bitwise

```zenpp
println(7 & 3)    // 3
println(7 | 3)    // 7
println(7 ^ 3)    // 4

x = 15
x &= 6         // 6
x |= 8         // 14
x ^= 3         // 13
```

### Comparison

All comparisons return `1` (true) or `0` (false).

```zenpp
println(3 == 3)    // 1
println(3 != 4)    // 1
println(3 < 4)     // 1
println(3 > 4)     // 0
println(3 >= 3)    // 1
println(3 <= 2)    // 0
```

### Logical

Short-circuit evaluation.

```zenpp
println(1 && 0)    // 0
println(1 || 0)    // 1
println(!true)     // 0
println(!false)    // 1
```

## Control Flow

### if / else if / else

```zenpp
n = read()
if n > 0 {
    println("positive")
} else if n == 0 {
    println("zero")
} else {
    println("negative")
}
```

Any non-zero value is truthy.

### Ternary Operator

```zenpp
x = n > 0 ? "positive" : "non-positive"
println(n % 2 == 0 ? "even" : "odd")
```

### while

```zenpp
i = 0
while i < 10 {
    println(i)
    i++
}
```

The post-decrement idiom works for test case loops:

```zenpp
t = read()
while t-- {
    // solve one test case
}
```

### for

Three forms based on the number of arguments:

```zenpp
// for variable count { body }
// loops i from 0 to count-1
for i 5 {
    println(i)        // 0 1 2 3 4
}

// for variable start end { body }
// loops i from start to end-1
for i 2 7 {
    println(i)        // 2 3 4 5 6
}

// for variable start end step { body }
for i 10 0 -2 {
    println(i)        // 10 8 6 4 2
}

// use _ to discard the loop variable
for _ 5 {
    println("hello")
}
```

### for-each

Iterate over vectors, maps, and sets:

```zenpp
v = [10, 20, 30]
for x in v {
    println(x)        // 10 20 30
}
```

### break and continue

```zenpp
for i 10 {
    if i == 5 { break }
    if i % 2 == 0 { continue }
    println(i)        // 1 3
}
```

## Functions

### Definition and Calling

```zenpp
fn add(a, b) {
    return a + b
}

result = add(3, 4)
println(result)       // 7
```

### Recursion

```zenpp
fn fib(n) {
    if n <= 1 { return n }
    return fib(n - 1) + fib(n - 2)
}

println(fib(10))   // 55
```

### No Return Value

Functions without an explicit `return` return `0` by default.

```zenpp
fn greet(name) {
    println("hello " + name)
}
```

## Strings

### Operations

```zenpp
s = "hello"
t = " world"

// concatenation
println(s + t)         // hello world

// length
println(len(s))        // 5

// indexing (returns single character)
println(s[0])          // h
```

### Escape Sequences

| Sequence | Meaning |
|----------|---------|
| `\n` | Newline |
| `\t` | Tab |
| `\r` | Carriage return |
| `\\` | Backslash |
| `\"` | Double quote |

## Vectors

### Creating and Accessing

```zenpp
v = [10, 20, 30]
println(v[0])          // 10
println(v[2])          // 30
println(v[-1])         // 30 (negative indexing)
println(v[-2])         // 20
v[1] = 99
println(v)             // [10, 99, 30]
```

### Manipulation

```zenpp
v = [1, 2, 3]
push(v, 4)           // [1, 2, 3, 4]
println(len(v))        // 4
last = pop(v)        // last = 4, v = [1, 2, 3]
```

### Concatenation

```zenpp
a = [1, 2]
b = [3, 4]
c = a + b
println(c)             // [1, 2, 3, 4]
```

### Nested Vectors

```zenpp
grid = [[1, 2], [3, 4], [5, 6]]
println(grid[1][0])    // 3
grid[2][1] = 99
println(grid[2][1])    // 99
```

## Maps

Hash tables with `O(1)` average lookup.

```zenpp
m = map()
m.set("alice", 95)
m.set("bob", 87)

println(m.get("alice", 0))   // 95
println(m.get("eve", -1))    // -1 (default)
println(m.has("bob"))         // 1
m.remove("bob")
println(m.has("bob"))         // 0
println(m.size())             // 1
```

Shorthand declaration:

```zenpp
map m       // equivalent to m = map()
```

### Bracket Indexing

Maps support `[]` for reading and writing, like C++:

```zenpp
m = map()
m["name"] = "zen"       // same as m.set("name", "zen")
println(m["name"])         // zen
println(m["missing"])      // 0 (default for missing keys)
```

### Methods

| Method | Description |
|--------|-------------|
| `m[key]` | Get value (returns 0 if missing) |
| `m[key] = value` | Set a key-value pair |
| `m.set(key, value)` | Set a key-value pair |
| `m.get(key, default)` | Get value or default if missing |
| `m.has(key)` | Check if key exists (1 or 0) |
| `m.remove(key)` | Delete a key |
| `m.size()` | Number of entries |
| `m.clear()` | Remove all entries |

## Sets

```zenpp
s = set()
s.add(10)
s.add(20)
s.add(10)            // duplicate, no effect

println(s.has(10))     // 1
println(s.has(30))     // 0
s.remove(10)
println(s.size())      // 1
```

Shorthand: `set s` is equivalent to `s = set()`.

### Bracket Indexing

Sets support `[]` for membership checks:

```zenpp
s = set()
s.add(42)
println(s[42])         // 1 (exists)
println(s[99])         // 0 (not found)
```

### Methods

| Method | Description |
|--------|-------------|
| `s[key]` | Check membership (1 or 0) |
| `s.add(key)` | Add an element |
| `s.has(key)` | Check membership (1 or 0) |
| `s.remove(key)` | Remove an element |
| `s.size()` | Number of elements |
| `s.clear()` | Remove all elements |

## Structs

User-defined types with fields and methods. Use `self` to access fields inside methods.

### Field Definitions

Structs can declare fields with default values directly in the body:

```zenpp
struct Point {
    x = 0
    y = 0

    fn init(a, b) {
        self.x = a
        self.y = b
    }

    fn dist() {
        return (self.x ** 2 + self.y ** 2)
    }
}

p = Point()          // x=0, y=0 (defaults applied)
p.init(3, 4)
println(p.x)           // 3
println(p.dist())      // 25
```

Fields are initialized with their default values when the struct is constructed, before `init()` is called.

### Dynamic Fields

Fields can also be created dynamically via `self.field = value` inside methods:

```zenpp
struct Counter {
    fn init(start) {
        self.value = start
    }
    fn increment() {
        self.value = self.value + 1
    }
    fn get() {
        return self.value
    }
}

c = Counter()
c.init(0)
c.increment()
c.increment()
println(c.get())       // 2
println(c.value)       // 2 (direct field access)
```

### Constructor

Define an `init()` method for initialization. Call it after creating the instance.

## Imports & Standard Library

### Importing the Standard Library

```zenpp
import std
```

This loads all modules listed in `stdlib/manifest.txt`.

### Importing Files

```zenpp
import "mylib"       // loads mylib.zpp
```

### Standard Library: Math

```zenpp
import std

println(min(3, 7))       // 3
println(max(3, 7))       // 7
println(abs(-5))          // 5
println(gcd(12, 8))      // 4
println(lcm(4, 6))       // 12
println(mid(4, 10))      // 7
println(ckmin(5, 3))     // 3
println(ckmax(2, 8))     // 8
```

#### Prefix Sums

```zenpp
import std

a = [1, 2, 3, 4, 5]
p = prefix(a)            // [0, 1, 3, 6, 10, 15]
// sum of a[1..3] = p[4] - p[1] = 10 - 1 = 9
println(p[4] - p[1])     // 9
```

#### Sum

```zenpp
import std
println(sum([1, 2, 3, 4, 5]))   // 15
```

#### Binary Search

```zenpp
import std

arr = [2, 5, 8, 12, 16, 23, 38]
println(binarySearch(arr, 23))   // 5
println(binarySearch(arr, 10))   // -1
```

#### Lower Bound / Upper Bound

```zenpp
import std

v = [1, 3, 3, 3, 5, 7]
println(lowerBound(v, 3))   // 1 (first index >= 3)
println(upperBound(v, 3))   // 4 (first index > 3)
```

#### Modular Exponentiation

```zenpp
import std
println(modpow(2, 10, 1000000007))   // 1024
```

#### Dijkstra (Shortest Paths)

```zenpp
import std

// wgraph reads weighted edges from stdin
n, m = read(), read()
adj = wgraph(n, m)
dist = dijkstra(adj, 0)
println(dist)
```

#### BFS (Unweighted Shortest Paths)

```zenpp
import std

// graph reads unweighted edges from stdin
n, m = read(), read()
adj = graph(n, m)
dist = bfs(adj, 0)
println(dist)
```

### Standard Library: Data Structures

#### Stack (LIFO)

```zenpp
import std
s = Stack()
s.init()
s.push(10)
s.push(20)
println(s.peek())      // 20
println(s.pop())       // 20
println(s.size())      // 1
```

#### Queue (FIFO)

```zenpp
import std
q = Queue()
q.init()
q.push(10)
q.push(20)
println(q.pop())       // 10
println(q.size())      // 1
```

#### DSU (Disjoint Set Union / Union-Find)

```zenpp
import std
d = DSU()
d.init(5)            // 5 elements: 0..4
d.unite(1, 2)
d.unite(3, 4)
println(d.same(1, 2))  // 1
println(d.same(1, 3))  // 0
d.unite(2, 3)
println(d.same(1, 4))  // 1
```

#### PriorityQueue (Max-Heap)

```zenpp
import std
pq = PriorityQueue()
pq.init()
pq.push(3)
pq.push(7)
pq.push(1)
println(pq.top())      // 7
println(pq.pop())      // 7
println(pq.top())      // 3
```

#### MinPriorityQueue (Min-Heap)

```zenpp
import std
pq = MinPriorityQueue()
pq.init()
pq.push(3)
pq.push(7)
pq.push(1)
println(pq.top())      // 1
println(pq.pop())      // 1
println(pq.top())      // 3
```

#### Pair and Tuple

```zenpp
import std

p = Pair()
p.init(10, 20)
println(p.first)        // 10
println(p.second)       // 20

t = Tuple()
t.init([1, 2, 3])
println(t.get(0))       // 1
t.set(1, 99)
println(t.get(1))       // 99
```

## Built-in Functions

| Function | Description |
|----------|-------------|
| `print(...)` | Print values separated by spaces (no newline) |
| `println(...)` | Print values separated by spaces, followed by newline |
| `ord(s)` | ASCII code of first character of string |
| `chr(n)` | Single-character string from ASCII code |
| `parseInt(s)` | Parse a string as an integer |
| `read()` | Read an integer from stdin |
| `read(n)` | Read `n` integers into a new vector |
| `read(v)` | Read `len(v)` integers into existing vector `v` |
| `readInt()` | Read an integer (alias for `read`) |
| `readFloat()` | Read a float from stdin |
| `readFloat(n)` | Read `n` floats into a new vector |
| `readLine()` | Read a full line as a string |
| `len(x)` | Length of a string or vector |
| `push(v, x)` | Append `x` to vector `v` |
| `pop(v)` | Remove and return the last element of `v` |
| `sort(v)` | Sort vector ascending in-place |
| `sortdec(v)` | Sort vector descending in-place |
| `reverse(v)` | Reverse vector in-place |
| `str(x)` | Convert any value to a string |
| `int(x)` | Convert string or float to integer |
| `float(x)` | Convert string or integer to float |
| `split(s, delim)` | Split string by delimiter, returns vector of strings |
| `join(v, delim)` | Join vector elements into a string with delimiter |
| `find(v, x)` | First index of `x` in vector or string, -1 if not found |
| `count(v, x)` | Count occurrences of `x` in vector or string |
| `swap(v, i, j)` | Swap elements at indices `i` and `j` in vector |
| `fill(n, val)` | Create a vector of `n` copies of `val` |
| `graph(n)` | Create adjacency list with `n` empty vectors |
| `graph(n, m)` | Read `m` undirected edges from stdin, build adjacency list |
| `dgraph(n, m)` | Read `m` directed edges from stdin |
| `wgraph(n, m)` | Read `m` undirected weighted edges (`u v w`), stores `[v, w]` |
| `dwgraph(n, m)` | Read `m` directed weighted edges |
| `map()` | Create an empty map |
| `set()` | Create an empty set |

## Comments

```zenpp
// single-line comment

/* multi-line
   block comment */

x = 42  // inline comment
```

## Examples

### Simple Competitive Programming Contest

```zenpp
fn solve() {
    n = read()
    if n % 2 == 0 {
        println(n ** 2)
    } else {
        println(n)
    }
}

tc = read()
while tc-- {
    solve()
}
```

### Sieve of Eratosthenes

```zenpp
n = 100
is_prime = []
for i n + 1 {
    push(is_prime, 1)
}
is_prime[0] = 0
is_prime[1] = 0

for i 2 n + 1 {
    if is_prime[i] {
        j = i * i
        while j <= n {
            is_prime[j] = 0
            j += i
        }
    }
}

for i 2 n + 1 {
    if is_prime[i] {
        println(i)
    }
}
```

### Binary Search

```zenpp
fn binary_search(v, target) {
    lo = 0
    hi = len(v) - 1
    while lo <= hi {
        mid = (lo + hi) / 2
        if v[mid] == target {
            return mid
        } else if v[mid] < target {
            lo = mid + 1
        } else {
            hi = mid - 1
        }
    }
    return -1
}

arr = [2, 5, 8, 12, 16, 23, 38, 56, 72, 91]
println(binary_search(arr, 23))    // 5
println(binary_search(arr, 10))    // -1
```

### Frequency Counter with Maps

```zenpp
n = read()
freq = map()
for i n {
    x = read()
    count = freq.get(x, 0)
    freq.set(x, count + 1)
}

// find the most frequent
best = 0
best_count = 0
for i n {
    x = read()
    c = freq.get(x, 0)
    if c > best_count {
        best = x
        best_count = c
    }
}
println(best)
```

### Graph BFS with Queue

```zenpp
import std

// read adjacency list
n = read()
adj = []
for i n {
    push(adj, [])
}

m = read()
for i m {
    u = read()
    v = read()
    push(adj[u], v)
    push(adj[v], u)
}

// BFS from node 0
visited = []
for i n { push(visited, 0) }
visited[0] = 1

q = Queue()
q.init()
q.push(0)

while q.size() > 0 {
    node = q.pop()
    println(node)
    for j len(adj[node]) {
        neighbor = adj[node][j]
        if visited[neighbor] == 0 {
            visited[neighbor] = 1
            q.push(neighbor)
        }
    }
}
```

### Struct: Linked List Node

```zenpp
struct Node {
    fn init(val) {
        self.val = val
        self.next = -1
    }
}

// build a list: 1 -> 2 -> 3
a = Node()
a.init(1)
b = Node()
b.init(2)
c = Node()
c.init(3)

// traverse using a vector of nodes
nodes = [a, b, c]
for i len(nodes) {
    println(nodes[i].val)
}
```
