import time, sys

sys.setrecursionlimit(100000)

# 1. Simple loop
t = time.time()
x = 0
for i in range(1000000):
    x += 1
print(f"Simple loop (1M):        {time.time()-t:.3f}s")

# 2. Function calls
def add(a, b):
    return a + b
t = time.time()
y = 0
for i in range(100000):
    y = add(y, 1)
print(f"Function calls (100K):   {time.time()-t:.3f}s")

# 3. Variable read/write
t = time.time()
a = 0
b = 0
c = 0
for i in range(500000):
    a = i
    b = a + 1
    c = b + 1
print(f"Variable read/write (500K): {time.time()-t:.3f}s")

# 4. Recursive fib
def fib(n):
    if n <= 1:
        return n
    return fib(n - 1) + fib(n - 2)
t = time.time()
f = fib(25)
print(f"fib(25):                 {time.time()-t:.3f}s")
