#include <iostream>

int add(int a, int b) { return a + b; }

int fib(int n) {
    if (n <= 1) return n;
    return fib(n - 1) + fib(n - 2);
}

int main() {
    // 1. Simple loop
    long long x = 0;
    for (int i = 0; i < 1000000; i++) x += 1;

    // 2. Function calls
    long long y = 0;
    for (int i = 0; i < 100000; i++) y = add(y, 1);

    // 3. Variable read/write
    long long a = 0, b = 0, c = 0;
    for (int i = 0; i < 500000; i++) {
        a = i;
        b = a + 1;
        c = b + 1;
    }

    // 4. Recursive function calls
    int f = fib(25);

    std::cout << x << " " << y << " " << c << " " << f << std::endl;
    return 0;
}
