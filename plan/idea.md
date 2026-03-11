# ZEN++ Syntax Guide

ZEN++ is a simple, clean, C++-inspired language with `{}` blocks, no semicolons, and whitespace-separated tokens. 

Blocks:
{
    x = 1
    y = 2
}

Variables:

Created on first assignment

Re-assignment allowed

No let keyword
x = 1
x = 5
y = x + 2

Expressions:

Arithmetic operators: + - * / %

Parentheses for grouping

z = (x + y) * 3
Booleans and Comparisons:

flag = true
x = 5
y = 10
cond = x < y
cond2 = x == 5
Conditionals:

if x < y {
    println(x)
} else if x == y {
    println("equal")
} else {
    println(y)
}
Loops:

tc = read()
while tc-- {
    println(tc)
}
Functions:

fn square(n) {
    return n * n
}
x = square(5)
println(x)
Built-in Functions:

x = read()
println(x)
Example Program:

fn solve(){
    n = read()
    if n%2==0{
        println(n ** 2)
    }
}

tc=read()
while tc--{
    solve()
}
