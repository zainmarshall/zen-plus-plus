# Zen++

A fast, lightweight interpreted language designed for competitive programming.

Zen++ features C-style syntax with curly braces, no semicolons, automatic type inference, and a standard library with common data structures out of the box.


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
    print("positive")
} else if x == 0 {
    print("zero")
} else {
    print("negative")
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
print(v[0])       // 1
print(len(v))     // 4
print(pop(v))     // 4
```

### Strings

```zenpp
s = "hello" + " world"
print(len(s))       // 11
print(s[0])         // ascii value of 'h'
```

### Maps & Sets

```zenpp
m = map()
m.set("key", 100)
print(m.get("key", 0))   // 100
print(m.has("key"))       // 1

s = set()
s.add(5)
print(s.has(5))           // 1
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
print(p.dist())   // 25
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
| `print(...)` | Print values separated by spaces, with newline |
| `read()` | Read integer from stdin |
| `readFloat()` | Read float from stdin |
| `readLine()` | Read line as string from stdin |
| `len(x)` | Length of string or vector |
| `push(v, x)` | Append to vector |
| `pop(v)` | Remove and return last element |

