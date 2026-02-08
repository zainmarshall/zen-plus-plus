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

