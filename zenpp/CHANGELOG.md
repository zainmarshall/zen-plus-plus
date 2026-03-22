# Change Log

All notable changes to the "zenpp" extension will be documented in this file.

## [0.2.1] - 2026-03-22

### Added
- Run button in editor title bar - click the play icon or press Cmd+Enter / Ctrl+Enter to run the current .zpp file
- Reuses a "Zen++" terminal so output doesn't clutter your workspace

## [0.2.0] - 2026-03-22

### Added
- F-string highlighting with interpolation expressions inside `{}`
- All new builtins: `sorted`, `unique`, `flatten`, `zip`, `replace`, `upper`, `lower`, `trim`, `contains`, `startswith`, `endswith`, `substr`, `all`, `any`, `rand`, `randvec`, `exit`
- Stdlib class highlighting: `Stack`, `Queue`, `DSU`, `PriorityQueue`, `MinPriorityQueue`, `FenwickTree`, `SegTree`, `Pair`, `Tuple`
- User function call highlighting (any identifier followed by `(`)
- Bitwise shift operators `<<`, `>>`, `~`

### Fixed
- Builtins now only highlight when followed by `(` to avoid false positives
- `map` and `set` highlight as builtins only in function-call position

## [0.1.0] - 2026-03-11

### Added
- `println` syntax highlighting
- `ord`, `chr`, `parseInt` builtin highlighting
- `break`, `continue`, `in` keyword highlighting

### Fixed
- `print` now correctly highlights alongside `println`

## [0.0.1] - Initial release

- Syntax highlighting for Zen++ (.zpp files)
- Comment, string, number, keyword, builtin, and operator tokens
- Bracket matching and auto-indentation
