# Zen++ Language Support

Syntax highlighting, intellisense, and one-click run for [Zen++](https://github.com/zainmarshall/zen-plus-plus) - a fast, lightweight interpreted language designed for competitive programming.

**[Try Zen++ in your browser](https://zainmarshall.github.io/zen-plus-plus/)** | **[Language Reference](https://github.com/zainmarshall/zen-plus-plus/blob/master/docs/language-reference.md)** | **[GitHub](https://github.com/zainmarshall/zen-plus-plus)**

## Features

### Syntax Highlighting

Full TextMate grammar covering all Zen++ syntax:

- Keywords (`fn`, `if`, `else`, `while`, `for`, `return`, `struct`, `import`, `break`, `continue`, `in`)
- 40+ built-in functions (`println`, `read`, `sort`, `reverse`, `fill`, `rand`, etc.)
- Stdlib types (`Stack`, `Queue`, `DSU`, `FenwickTree`, `SegTree`, etc.)
- F-string interpolation - expressions inside `{}` are highlighted normally, string parts stay green
- User-defined function calls get their own highlight color
- Numbers, strings, comments (line and block), operators including `**`, `<<`, `>>`, `~`

### Run Button

Click the play button in the editor title bar or press **Cmd+Enter** (Mac) / **Ctrl+Enter** (Windows/Linux) to run the current `.zpp` file. Requires `zenpp` to be installed and on your PATH ([install instructions](https://github.com/zainmarshall/zen-plus-plus#installation)).

### Editor Support

- Auto-closing brackets, quotes, and braces
- Auto-indentation on `{` and dedent on `}`
- Line and block comment toggling

## What is Zen++?

Zen++ is a language built for competitive programming. It has C-style syntax but strips away everything slow to type - no semicolons, no type declarations, no `int main()`. A Codeforces solution that takes 10 lines in C++ takes 5 in Zen++.

```zenpp
// read n numbers, sort them, print the median
n = read()
a = read(n)
sort(a)
println(a[n / 2])
```

```zenpp
// Dijkstra's shortest path in 3 lines
import std
n, m = read(), read()
adj = wgraph(n, m)
println(dijkstra(adj, 0))
```

```zenpp
// f-strings, lambdas, slicing
v = [3, 1, 4, 1, 5, 9]
sort(v, fn(a, b) { b - a })
println(f"top 3: {v[:3]}")
```

The language includes vectors, maps, sets, structs, lambdas, slicing, destructuring, f-strings, chained comparisons, and a standard library with Stack, Queue, DSU, PriorityQueue, FenwickTree, SegTree, Dijkstra, BFS, binary search, prefix sums, and more.

## Requirements

To use the run button, install the Zen++ interpreter:

```bash
git clone https://github.com/zainmarshall/zen-plus-plus.git
cd zen-plus-plus
bash install.sh
```

Syntax highlighting works without the interpreter installed.
