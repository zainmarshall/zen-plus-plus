# Devlogs

## Zen++ Devlog I
So I did a few things:
1. I decided on the syntax of Zen++. Since I want to use it for like codeforces and stuff it should be very fast to type out while still being fast. So I decided on the syntax of the language. It will use curly braces to delineate blocks of code, no semicolons, for variables no let keyword and optional type so like x=1 for now. For loops and if statements no need for parentheses, just do while tc-- or if bool. See [idea.md](https://github.com/zainmarshall/zen-plus-plus/blob/master/idea.md) for more.
2. I decided it'll be interpreted and I got started on basic math. So the way it works is you start with the statement in code, then a lexer tokenizes it (it identifies what is a parenthesis what is a plus sign, stuff like that), and then a parser takes the tokens and creates a hierarchy (so like pemdas) by making an Abstract Syntax Tree which we then evaluate by going to the bottom nodes, evaluating them and then propagating upwards. So very cool things. So far I just did basic binary arithmetic operations like + - * / % ^ and also a unary operator !. The code is made so its decently easy to add more tokens. 

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

