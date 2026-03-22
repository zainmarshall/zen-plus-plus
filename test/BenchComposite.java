public class BenchComposite {
    static int add(int a, int b) { return a + b; }

    static int fib(int n) {
        if (n <= 1) return n;
        return fib(n - 1) + fib(n - 2);
    }

    public static void main(String[] args) {
        // 1. Simple loop
        long x = 0;
        for (int i = 0; i < 1000000; i++) x += 1;

        // 2. Function calls
        long y = 0;
        for (int i = 0; i < 100000; i++) y = add((int)y, 1);

        // 3. Variable read/write
        long a = 0, b = 0, c = 0;
        for (int i = 0; i < 500000; i++) {
            a = i;
            b = a + 1;
            c = b + 1;
        }

        // 4. Recursive function calls
        int f = fib(25);

        System.out.println(x + " " + y + " " + c + " " + f);
    }
}
