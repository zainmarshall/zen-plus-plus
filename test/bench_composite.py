import sys

# 1. Simple loop
x = 0
for i in range(1000000):
    x += 1

# 2. Function calls
def add(a, b):
    return a + b
y = 0
for i in range(100000):
    y = add(y, 1)

# 3. Variable read/write
a = 0
b = 0
c = 0
for i in range(500000):
    a = i
    b = a + 1
    c = b + 1

# 4. Recursive function calls
sys.setrecursionlimit(100000)
def fib(n):
    if n <= 1:
        return n
    return fib(n - 1) + fib(n - 2)
f = fib(25)

print(x, y, c, f)
