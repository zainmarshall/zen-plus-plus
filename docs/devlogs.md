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
