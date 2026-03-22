# Devlogs

## Zen++ Devlog I
So I did a few things:
1. I decided on the syntax of Zen++. Since I want to use it for like codeforces and stuff it should be very fast to type out while still being fast. So I decided on the syntax of the language. It will use curly braces to delineate blocks of code, no semicolons, for variables no let keyword and optional type so like x=1 for now. For loops and if statements no need for parentheses, just do while tc-- or if bool. See [idea.md](https://github.com/zainmarshall/zen-plus-plus/blob/master/idea.md) for more.
2. I decided it'll be interpreted and I got started on basic math. So the way it works is you start with the statement in code, then a lexer tokenizes it (it identifies what is a parenthesis what is a plus sign, stuff like that), and then a parser takes the tokens and creates a hierarchy (so like pemdas) by making an Abstract Syntax Tree which we then evaluate by going to the bottom nodes, evaluating them and then propagating upwards. So very cool things. So far I just did basic binary arithmetic operations like + - * / % ** and also a unary operator !. The code is made so its decently easy to add more tokens. 

### Changelog
- [Binary and unary math parsing](https://github.com/zainmarshall/zen-plus-plus/commit/6b47222ef6b7e25698af0d838f0f9ab84607b67c)
- [sytnax](https://github.com/zainmarshall/zen-plus-plus/commit/d969b5538b9025b8346f398d5ab1bef62d91978a)

## Zen++ Devlog II
I added variables and assignment. You can declare a variable like `x=4` and reassign it like `x=5`

What changed:
1. The lexer now recognizes identifiers and the = token
2. The AST supports variable nodes and assignment nodes.
3. The parser now uses a lookahead to treat identifier = expr as an assignment, and it also lets identifiers show up inside normal expressions.
4. The evaluator keeps a small variable map so assignments store values and identifiers read them back.

### Changelog
- [Variables and Assignment](https://github.com/zainmarshall/zen-plus-plus/commit/17212a585f6c47f7d316112cb932087e08c600d3)
- [devlogs](https://github.com/zainmarshall/zen-plus-plus/commit/44d1ca303ce65997e834ffe0bc69ff032e80ec9a)

## Zen++ Devlog III
This step adds booleans and comparisons so I can start building if/else statements later. Booleans are just like they are in C++, a true boolean is a 1 and a false boolean is a 0. 

What changed:
1. The lexer now recognizes `true/false` and multi-character operators like `==`, `!=`, `<=`, `>=`.
2. The AST got boolean literals, comparison operators, and logical NOT.
3. The parser now handles comparison expressions and prefix `!`, while keeping postfix `!` for factorial.
4. The evaluator now returns 0/1 for comparisons and supports logical NOT.

### Changelog
- [Booleans and comparisions](https://github.com/zainmarshall/zen-plus-plus/commit/5d06bdf80711aed21ce1fb90b0060879086123ba)

## Zen++ Devlog IV
I added if else statments to the code! The way an if else-if else statment will be written is like tihs:
```zen
if bool1{
    x=1
}else if bool2{
    x=2
}else{
    x=3
}
```
Note the lack of parantehsis, you don't rly need them beacuse you know your condition is just gonna be sandwiched between the if and the next curly brace. Also now that I added braces I made the parser handle the braces and the AST handle blocks of code.

What changed:
1. The lexer recognizes `if`, `else`, `{` and `}`.
2. The parser builds block nodes and full if/else chains, and parses the full program instead of a single statement.
3. The evaluator now runs blocks and if/else nodes.
4. The REPL buffers lines and runs the program when you submit a blank line.

## Zen++ Devlog V
Ok so in this devlog I spent a lot of time thinking about syntax and I would like some feedback about for loops. 
First though, while loops were simple (not simple to implement these loops cooked me in coding but syntaxically simple), its just. 
```
while boolean {
do this code here yeah
}
```
So while loops are very simple. Now for loops were a pain. I wanted them to be simple as this is meant to be super fast to write for competitive programming and the likes. So for this I took inspiration from my C++ template file. 
```
#define FOR(i, a) for(int i = 0; i < a; i++)
#define ROF(i, a) for(int i = a; i >= 0; i--)
#define FORA(i, a, b) for(int i = a; i <= b; i++)
#define ROFA(i, a, b) for(int i = a; i >= b; i--)
```
That's how I macro for loops during Codeforces contests. So instead of
`for(int i=0;i<n;i++){...}` I can just do `FOR(i,n)`. Now I wanted something nice and simple for Zen++ to. So I landed on this:
A four loop has four components, the variable used, the start, the end, and the step. So why not instead of writing those as a declaration, a boolean, and an incrementation like in C++, just treat those almost as paramateres to a method. So thats what I did. 
`for i start end step {...}`
Thats the syntax. If you want a reverse for loop you just make end bigger than start and step will be negative. 
There is a 3-arg version which defaults step to 1 or -1 depending on direction and looks like: `for i start end{...}` and there is a 2-arg one which is: `for i end{...}` and defaults step to +1 and start to 0. So for codeforces you can just write `for i n{...}`. Very simple, but leave comments if you think its ugly. Its exclusive by default rn with no way to make it inclusive, I'll fix that later.

### Changelog
- [for and while loops](https://github.com/zainmarshall/zen-plus-plus/commit/76827c96c3a7b0ecc0506863044d49ed92a2dfe3)


## Zen++ Devlog VI
This was a pretty big update but I didn't add anything that is syntaxically weird. I added functions, vectors, and strings. 

### What changed:

1. Functions + Returns
    - Added parsing/eval support for `fn name(args){...}` and `return expr`.
    - Function definitions are stored and callable later.
    - Added function scope behavior so params/locals resolve correctly.
    - This includes built-ins like `read()` and `println(...)`.

2. Strings
    - Lexer now tokenizes string literals.
    - Parser builds string AST nodes.
    - Runtime can store/print/concatenate strings.
    - You can index into strings (`s[i]`) and use `len(s)`.

3. Vectors (Dynamic Arrays)
    - Added vector syntax: `[1, 2, 3]`.
    - Added vector indexing: `v[i]`.
    - Added built-ins:
      - `len(v)`
      - `push(v, x)`
      - `pop(v)`
    - Added vector concatenation through `+` when both sides are vectors.

4. Multidimensonal vectors
    - Because vectors can contain vectors, nested structures like `[[1,2],[3,4]]` just work.
    - Nested indexing works (`m[1][0]`).

5. Comments + Illegal Character Throwing
    - Added support for comments:
      - line comments `// ...`
      - block comments `/* ... */`
    - Added stricter behavior for unknown/invalid characters so bad input doesn’t silently pass.

6. Reorginization
    - `src/` now contains all `.cpp` + `.hpp` source files.
    - `test/` now contains test scripts + expected output.
    - `build/` now contains the compiled binaries
    - `docs/` contains docs/devlogs.

7. Makefile + Testing
    - Added a `Makefile` with nice commands 
    - `make run` launches REPL.
    - `make run <file.zpp>` runs a file directly.
    - Made `test.zpp` as a file with every command in the language and then wrote the expected output and made `make test` run the test script and compare output.

### Changelog
- [functions and vectors](https://github.com/zainmarshall/zen-plus-plus/commit/d19fdffdb83e6ddd26f84fa117deca582f15105b)
- [multi line buffer](https://github.com/zainmarshall/zen-plus-plus/commit/21d4ee0f158fcb87086fad285c3050c5ea399126)

## Zen++ Devlog VII
This batch saw a lot of new, big features, and some minor ones. Big: data structures, structs, imports. Small: bitwise operators, numerical type fixes, IO expansion. 

### What changed
1. Numeric types
   - Integers are now 64-bit.
   - Added float

2. Operators
   - Exponent operator is now `**`.
   - `^`, `&`, `|` are bitwise operators.
   - Added compound assignments for bitwise ops.

3. IO
   - Added `readInt()`, `readFloat()`, and `readLine()`.
   - `read()` maps to `readInt()`.

4. Structs + methods + member access
   - New `struct Name { fn ... }` syntax.
   - Method calls with `obj.method(...)` and field access with `obj.field`.
   - Assignment to fields supported (`obj.field = x`).

5. Imports + stdlib bundle
   - New `import file` and `import std`.
   - Stdlib is bundled via `stdlib/manifest.txt` and loaded by `import std`.

6. Data structures
   - Hash table in runtime (map/set) for O(1).
   - Exposed as stdlib `Map` and `Set` structs.
   - Added DSU, PriorityQueue, Stack, Queue, Pair, Tuple in stdlib.

7. Tests
    - Test runner has more complete coverage.

8. Math functions
    - Added standard math functions like `min(a,b)`, `gcd(a,b)` and `abs(a)` to stdlib bundle


## Zen++ Devlog VIII
I compiled the C++ Interpreter over to Web Assembly so I could make a web interpreter for it. 

### What Changed
1. Made a web UI inspired by USACO IDE, with Input and outputed
2. Added syntax highlighting to it
3. Added runtime error messages
4. Fixed bugs in the langauge
5. Added tests for the website

### Changelog
- [move webs to docs](https://github.com/zainmarshall/zen-plus-plus/commit/6875f9a)
- [web view](https://github.com/zainmarshall/zen-plus-plus/commit/2473e45)
- [Structs + StdLib + Web](https://github.com/zainmarshall/zen-plus-plus/commit/002f27a)

## Zen++ Devlog IX - v1 Polish
Big batch of changes getting ready for v1. A mix of language features, website fixes, tooling, and docs cleanup.

### What Changed

1. Map/Set bracket indexing
   - Maps now support `m[key]` for reading and `m[key] = val` for writing, like C++.
   - Works on member maps too (`obj.data[key] = val`).
   - Sets support `s[key]` to check membership (returns 1 or 0).
   - `.get()`, `.set()`, `.has()`, `.add()` all still work alongside `[]`.

2. Struct field definitions
   - Structs can now declare fields with default values directly in the body:
     ```
     struct Point {
         x = 0
         y = 0
         fn init(a, b) { self.x = a  self.y = b }
     }
     ```
   - Defaults are applied when the struct is constructed, before `init()` runs.

3. Break, continue, for-each
   - Added `break` and `continue` for loops.
   - Added for-each syntax: `for x in collection { }` to iterate over vectors, maps, and sets.

4. Website: fixed intellisense
   - Added better intellisnse and removed filler words

5. Website: docs button
   - Added a "Docs" link in the toolbar of website

6. Packaging / install script
   - Added `install.sh`

7. VSCode extension
   - Built and packaged a VS Code extension (`zenpp-0.0.1.vsix`) with TextMate grammar for syntax highlighting and language configuration.
   - Published it to VSCode Extnsion Marketplace

8. Docs polish
   - Fixed some errors in the docs

## Attachments
1. A screenshot of the extnension in the VSCode Marketplace (so cool!!!)
2. A screenshot of intellsense in the web interpreter 
3. Here is the link to the documentation: https://github.com/zainmarshall/zen-plus-plus/blob/master/docs/language-reference.md 

### Changelog
- [Extension](https://github.com/zainmarshall/zen-plus-plus/commit/44e6e90)
- [Create zenpp](https://github.com/zainmarshall/zen-plus-plus/commit/e70cf37)
- [Extension](https://github.com/zainmarshall/zen-plus-plus/commit/5a3cb6d)
- [Delete docs/README.md](https://github.com/zainmarshall/zen-plus-plus/commit/4039a76)

## Zen++ Devlog X - v0.1.0 Release Prep

Prepping for v0.1.0. Implemented the last missing language features, synced up the website/extension/docs, added a version flag, and just fixed a lot of bugs.

### What Changed

1. **Website updates**
   - Syntax highlighting and autocomplete now cover all builtins (`println`, `ord`, `chr`, `parseInt`) and new keywords (`break`, `continue`, `in`).
   - Added line numbers to the code editor.
   - Grouped toolbar buttons together, default program uses `println`.

2. **VS Code extension v0.1.0**
   - Updated TextMate grammar with all new builtins and keywords.
   - Bumped version, updated CHANGELOG, repackaged VSIX.

3. **CLI `--version` flag**
   - `zenpp --version` now prints `zenpp 0.1.0`.

4. **Docs, samples, tests**
   - Language reference now documents for-each, break, continue.
   - New samples: `foreach_demo.zpp`, `bracket_indexing.zpp`.
   - Added test coverage for new features. All tests passing.
   - Rebuilt WASM so the web version has everything too.

### Changelog
- [Syntax Highlighting Fix + Docs](https://github.com/zainmarshall/zen-plus-plus/commit/1c05d9b)
- [add image](https://github.com/zainmarshall/zen-plus-plus/commit/983c1d1)
- [Documentatio](https://github.com/zainmarshall/zen-plus-plus/commit/cb18d1b)
- [more samples](https://github.com/zainmarshall/zen-plus-plus/commit/b5900d9)

## Zen++ v0.1.0 Ship

Zen++ is a fast, lightweight interpreted language designed for competitive programming.

Zen++ features C-style syntax with curly braces, no semicolons, automatic type inference, and a standard library with common data structures out of the box.

**[Try it in your browser](https://zainmarshall.github.io/zen-plus-plus/)**

### What's in v0.1.0

The language has everything you need for competitive programming and then some:
- Implicit variable declarations, 64-bit integers, floats, strings, booleans
- Vectors, maps, sets with bracket indexing
- Structs with fields, methods, and defaults
- For loops (`for i n`, `for i a b`, `for i a b step`), for-each, while, break, continue
- Functions with recursion, closures over global scope
- Standard library: Stack, Queue, DSU, PriorityQueue, min/max/abs/gcd
- Built-in IO: `read()`, `readInt()`, `readFloat()`, `readLine()`, `print()`, `println()`

It runs three ways:
1. **CLI interpreter** - `zenpp file.zpp` or just `zenpp` for the REPL
2. **Browser** - full web IDE at [zainmarshall.github.io/zen-plus-plus](https://zainmarshall.github.io/zen-plus-plus/) with syntax highlighting, autocomplete, and the interpreter compiled to WebAssembly
3. **VS Code** - syntax highlighting extension on the Marketplace

I also solved a the full Codeforces Round 1084 (Div. 3) with Zen++ to prove it works.

## Zen++ Devlog XI - Competitive Programming Power-Up

I compared my C++ competitive programming macro/template with Zen++ and realized there were a bunch of things the macro could do that Zen++ couldn't. So I went through and added them. 

### What Changed

1. **Bulk I/O**
   - `read(n)` reads `n` integers from stdin and returns a vector. No more manual loops to read input.
   - `read(v)` reads `len(v)` integers directly into an existing vector.
   - Same for `readFloat(n)` and `readFloat(v)`.
   - Before: `a = []` then `for i n { push(a, read()) }`. Now: `a = read(n)`. One line.

2. **Type casting**
   - `str(x)` converts any value (int, float, vector) to a string.
   - `int(x)` converts a string or float to an integer (truncates floats, parses strings).
   - `float(x)` converts a string or int to a float.
   - These replace the old `parseInt()` pattern with cleaner, more general casts.

3. **New stdlib math functions** (`import std`)
   - `lcm(a, b)` - least common multiple.
   - `mid(a, b)` - midpoint, equivalent to `(a + b) / 2`.
   - `ckmin(a, b)` - returns the smaller of two values.
   - `ckmax(a, b)` - returns the larger of two values.
   - `prefix(v)` - builds a prefix sum array. Returns array of length `len(v) + 1` where `p[i]` = sum of first `i` elements.

4. **Binary search** (`import std`)
   - `binarySearch(v, target)` - searches a sorted vector for `target`, returns index or -1.

5. **Dijkstra's algorithm** 
   - `dijkstra(adj, start)` - runs Dijkstra's shortest path from `start`.
   - `adj[u]` is a list of `[v, w]` pairs (neighbor, weight).
   - Returns a distance array. 



## Zen++ Devlog XII - Language Features & Graph Builtins

Big batch of language features, new builtins, and stdlib additions. The main theme was closing the gap between Zen++ and a C++ competitive programming template.

### What Changed

1. **Ternary operator**
   - `x = a > b ? a : b` - inline conditionals, nestable.
   - Added `?` and `:` tokens to lexer, `TERNARY` node type, and right-associative parsing so `a ? b : c ? d : e` works as expected.

2. **Negative indexing**
   - `v[-1]` for last element, `v[-2]` for second-to-last. Works for vectors and strings, both reading and assignment.

3. **Multiple assignment**
   - `a, b = 1, 2` assigns multiple variables at once.
   - `a, b = b, a` swaps values - all RHS values are evaluated before any assignment happens.
   - `_` discards a value: `_, b = 0, 42`.

4. **`_` in for loops**
   - `for _ n { }` when you don't need the loop variable. Works in for-each too.

5. **String repeat**
   - `"ha" * 3` gives `"hahaha"`. Works both ways: `3 * "ha"`.

6. **New builtins**
   - `split(s, delim)` - split string into vector of strings.
   - `join(v, delim)` - join vector elements into a string.
   - `find(v, x)` - first index of `x` in vector or string, -1 if not found.
   - `count(v, x)` - count occurrences in vector or string.
   - `swap(v, i, j)` - swap two elements in a vector (supports negative indices).
   - `fill(n, val)` - create a vector of `n` copies of `val`. Deep-copies vectors so each element is independent.

7. **Graph builtins**
   - `graph(n)` - create adjacency list with `n` empty vectors.
   - `graph(n, m)` - read `m` undirected edges (`u v`) from stdin, build adjacency list.
   - `dgraph(n, m)` - read `m` directed edges.
   - `wgraph(n, m)` - read `m` undirected weighted edges (`u v w`), stores `[v, w]` pairs.
   - `dwgraph(n, m)` - read `m` directed weighted edges.
   - Dijkstra sample went from 8 lines of graph setup to `adj = wgraph(n, m)`.

8. **New stdlib functions** (`import std`)
   - `sum(v)` - sum a vector.
   - `lowerBound(v, x)` / `upperBound(v, x)` - binary search returning insertion index in a sorted vector.
   - `modpow(base, exp, mod)` - modular exponentiation.
   - `bfs(adj, start)` - BFS shortest paths on unweighted graph, returns distance array.

9. **Internal fix**
   - `push(v[i], x)` now works - you can push to indexed vectors directly (needed for manual adjacency list building).

## Zen++ Devlog XIII

This one was all about making Zen++ more expressive and closing gaps that were annoying when solving problems.

### What Changed

1. **Lambda functions**
   - Anonymous functions as expressions: `fn(a, b) { a - b }`.
   - `sort(v, fn(a, b) { a[1] - b[1] })` - sort a vector of pairs by second element, inline.

2. **Slicing**
   - Python-style slicing for vectors and strings: `v[1:4]`, `s[0:5]`, `v[::-1]`.
   - Full syntax: `[start:end]`, `[start:end:step]`. All three parts are optional.
   - `v[::-1]` - reversed copy. 
   - Works for strings too: `s[::-1]` reverses a string, `s[0:5]` takes a substring.

3. **Tuple unpacking in for-each**
   - `for u, v in edges { }` - destructure each element when iterating vectors of vectors.
   - `for k, v in myMap { }` - iterate key-value pairs of a map.
   - Supports `_` for discarding: `for _, v in pairs { }`.
   - This is huge for graph problems where adjacency lists store `[neighbor, weight]` pairs.

4. **Default function arguments**
   - `fn solve(n, mod = 1000000007) { ... }` - parameters with `= value` get defaults.
   - Call with fewer args and the defaults fill in: `solve(5)` uses the default mod.
   - Works in regular functions, struct methods, and lambdas.
   - Required params must come before optional ones.

5. **Bitwise shift operators**
   - `<<` and `>>` with correct precedence (between addition and comparison, like C).
   - Compound assignments: `x <<= 3`, `x >>= 1`.
   - Essential for bitmask DP and bit manipulation problems.

6. **String manipulation builtins**
   - `replace(s, old, new)` - replace all occurrences.
   - `upper(s)` / `lower(s)` - case conversion.
   - `startswith(s, prefix)` / `endswith(s, suffix)` - boolean checks.
   - `trim(s)` - strip leading/trailing whitespace.
   - `substr(s, start)` or `substr(s, start, len)` - substring with negative index support.
   - `contains(s, sub)` - check if string contains substring.

7. **Multi-dimensional `fill()`**
   - `fill(n, m, val)` creates an n×m grid filled with `val`.
   - `fill(n, m, k, val)` creates a 3D array. Works for any number of dimensions.
   - Each row is an independent deep copy, so `grid[0][0] = 5` doesn't affect `grid[1][0]`.
   - Before: `grid = fill(n, fill(m, 0))`. Now: `grid = fill(n, m, 0)`. Cleaner and fewer mistakes.

## Zen++ Devlog XIV

More language features. Bitwise NOT, f-strings, chained comparisons, destructuring, min/max on vectors, and all/any predicates.

### What Changed

1. **Bitwise NOT (`~`)**
   - `~x` gives bitwise complement. `clearBit` is now just `n & ~(1 << i)`.

2. **`min(v)` / `max(v)` on vectors**
   - `min([3, 1, 4])` returns `1`. `max([3, 1, 4])` returns `4`.
   - Still works with 2 args: `min(a, b)`. No `import std` needed anymore.

3. **String interpolation**
   - `f"hello {name}, {1 + 2} = {3}"` - expressions inside `{}` are evaluated and stringified.

4. **`all(v, fn)` / `any(v, fn)`**
   - `all(v, fn(x) { x > 0 })` - true if every element passes.
   - `any(v, fn(x) { x > 0 })` - true if at least one passes. Short-circuits.

5. **Chained comparisons**
   - `1 < x < 10` works like `1 < x && x < 10`. Any number of comparisons can be chained.

6. **Destructuring assignment**
   - `[a, b, c] = v` unpacks a vector into variables. Supports `_` to discard.
   - `[x, _, z] = getPoint()` - grab first and third, skip second.

7. **Newline-aware postfix parsing**
   - Fixed a parser ambiguity where `[` on a new line was consumed as an index operator from the previous expression. Indexing with `[` now requires it to be on the same line as the expression.

## Zen++ Devlog XV - Interpreter Optimization

Four optimizations to the interpreter. Combined: **11x speedup** on composite benchmark (33.7s → 3.05s).

### What Changed

1. **Flag-based control flow** - replaced C++ `throw`/`catch` for `return`/`break`/`continue` with a `Signal` enum. Function calls: 7.5s → 0.4s (**17x**).
2. **Arena allocator** - AST nodes allocated from contiguous 4096-node blocks instead of individual `new`. Better cache locality.
3. **Scope frame reuse** - function calls reuse cleared `unordered_map` frames instead of alloc/free each call. Variable ops: 1.14s → 1.00s.
4. **Constant folding** - `1 + 2 + 3` folds to `6` at parse time.

### Benchmarks (before → after)

- **Simple loop (1M):** 1.08s → 0.95s (1.1x)
- **Function calls (100K):** 7.53s → 0.36s (**21x**)
- **Variable read/write (500K):** 1.14s → 1.00s (1.1x)
- **Vector push (100K):** 0.16s → 0.14s (1.1x)
- **Composite (all + fib(25)):** 33.67s → 3.05s (**11x**)

## Zen++ Devlog XVI - Website Polish

### What Changed

1. **Sample dropdown label** - selecting a sample now updates the dropdown text to show what's loaded instead of always saying "Samples".
2. **Settings / theme switcher** - added a settings cog in the toolbar that opens a theme picker. Ships with 6 themes: Tokyo Night (default), Dracula, Catppuccin Mocha, GitHub Dark, Gruvbox, and Nord. Selection persists via localStorage.

## Zen++ Devlog XVII - New Builtins, Data Structures & Polish

### What Changed

1. **New builtins**
   - `rand(lo, hi)`, `randvec(n, lo, hi)` - random number generation.
   - `exit()` - terminate program.
   - `reverse(v)` - reverse vector or string in-place.
   - `unique(v)` - remove consecutive duplicates.
   - `sorted(v)` - return a sorted copy without modifying the original.
   - `flatten(v)` - flatten one level of nesting.
   - `zip(a, b)` - pair up two vectors into `[[a[0],b[0]], ...]`.

2. **String iteration** - `for c in s { }` iterates over each character in a string.

3. **FenwickTree and SegTree** (`import std`)
   - `FenwickTree` - point update + prefix/range sum queries in O(log n).
   - `SegTree` - range sum queries + point updates in O(log n). Build from an array with `st.build(arr)`.

4. **Scoping fix** - fixed a bug where local variables in recursive functions would leak into parent calls. Functions now correctly see only their own scope + globals.

5. **VS Code extension v0.2.0 + web IDE syntax highlighting**
   - F-string interpolation highlighting - expressions inside `{}` are colored normally, string parts stay green.
   - All new builtins highlighted (`sorted`, `unique`, `flatten`, `zip`, `rand`, `replace`, `upper`, `lower`, `all`, `any`, etc.).
   - Stdlib classes highlighted as types (`FenwickTree`, `SegTree`, `MinPriorityQueue`, etc.).
   - Bitwise shift (`<<`, `>>`) and NOT (`~`) operator highlighting.
   - User-defined function calls get their own highlight color in VS Code.
   - VS Code extension and web IDE are now in sync.

6. **Web IDE intellisense overhaul**
   - Autocomplete now shows full function signatures with parameter names and short descriptions instead of just empty parens (e.g. `push(v, x)` with "append x to vector v").
   - Added code snippets for `fn`, `for`, `while`, `if`, `struct`.

7. **Samples cleanup**
   - Removed test files, added 5 new demos (fenwick tree, segment tree, lambdas, slicing, f-strings).
   - Modernized all 8 Codeforces solutions with new syntax (`read(n)`, slicing, `for c in s`, `any()`).
   - Website samples synced 1:1 with file samples (22 total across 5 categories).

