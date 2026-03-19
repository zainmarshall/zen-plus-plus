R"STDLIB(
// Math helpers for ZEN++.

fn min(a, b) {
    if a < b { return a }
    return b
}

fn max(a, b) {
    if a > b { return a }
    return b
}

fn abs(x) {
    if x < 0 { return -x }
    return x
}

fn gcd(a, b) {
    a = abs(a)
    b = abs(b)
    while b != 0 {
        t = a % b
        a = b
        b = t
    }
    return a
}

fn lcm(a, b) {
    return a / gcd(a, b) * b
}

fn mid(a, b) {
    return (a + b) / 2
}

fn ckmin(a, b) {
    if b < a { return b }
    return a
}

fn ckmax(a, b) {
    if b > a { return b }
    return a
}

fn prefix(v) {
    p = [0]
    for i len(v) {
        push(p, p[i] + v[i])
    }
    return p
}

fn sum(v) {
    s = 0
    for x in v { s += x }
    return s
}

fn lowerBound(v, target) {
    lo = 0
    hi = len(v)
    while lo < hi {
        m = (lo + hi) / 2
        if v[m] < target { lo = m + 1 }
        else { hi = m }
    }
    return lo
}

fn upperBound(v, target) {
    lo = 0
    hi = len(v)
    while lo < hi {
        m = (lo + hi) / 2
        if v[m] <= target { lo = m + 1 }
        else { hi = m }
    }
    return lo
}

fn binarySearch(v, target) {
    lo = 0
    hi = len(v) - 1
    while lo <= hi {
        m = (lo + hi) / 2
        if v[m] == target {
            return m
        } else if v[m] < target {
            lo = m + 1
        } else {
            hi = m - 1
        }
    }
    return -1
}

// dijkstra(adj, start) — shortest paths from start
// adj[u] = [[v, w], [v2, w2], ...] adjacency list with weights
// returns dist[] array where dist[v] = shortest distance from start to v
fn dijkstra(adj, start) {
    n = len(adj)
    INF = 1000000000000000007
    MAXN = 1000001
    dist = fill(n, INF)
    dist[start] = 0
    pq = MinPriorityQueue()
    pq.init()
    // encode (dist, node) as dist * MAXN + node so min-heap orders by dist
    pq.push(start)
    while pq.size() > 0 {
        top = pq.pop()
        d = top / MAXN
        u = top % MAXN
        if d > dist[u] { continue }
        edges = adj[u]
        for i len(edges) {
            e = edges[i]
            v = e[0]
            w = e[1]
            nd = dist[u] + w
            if nd < dist[v] {
                dist[v] = nd
                pq.push(nd * MAXN + v)
            }
        }
    }
    return dist
}

fn modpow(base, exp, mod) {
    result = 1
    base = base % mod
    while exp > 0 {
        if exp % 2 == 1 {
            result = result * base % mod
        }
        exp = exp / 2
        base = base * base % mod
    }
    return result
}

// bfs(adj, start) — shortest unweighted paths from start
// adj[u] = [v1, v2, ...] adjacency list (no weights)
// returns dist[] array where dist[v] = shortest distance, -1 if unreachable
fn bfs(adj, start) {
    n = len(adj)
    dist = fill(n, -1)
    dist[start] = 0
    q = Queue()
    q.init()
    q.push(start)
    while q.size() > 0 {
        u = q.pop()
        neighbors = adj[u]
        for i len(neighbors) {
            v = neighbors[i]
            if dist[v] == -1 {
                dist[v] = dist[u] + 1
                q.push(v)
            }
        }
    }
    return dist
}

// Data structure helpers for ZEN++.

struct Stack {
    fn init() {
        self.data = []
    }
    fn push(v) {
        push(self.data, v)
        return len(self.data)
    }
    fn pop() {
        return pop(self.data)
    }
    fn peek() {
        return self.data[len(self.data) - 1]
    }
    fn size() {
        return len(self.data)
    }
}

struct Queue {
    fn init() {
        self.data = []
    }
    fn push(v) {
        push(self.data, v)
        return len(self.data)
    }
    fn pop() {
        if len(self.data) == 0 { return 0 }
        v = self.data[0]
        i = 1
        while i < len(self.data) {
            self.data[i - 1] = self.data[i]
            i++
        }
        pop(self.data)
        return v
    }
    fn size() {
        return len(self.data)
    }
}

struct Map {
    fn init() { self.data = map() }
    fn set(key, value) { return self.data.set(key, value) }
    fn get(key, default) { return self.data.get(key, default) }
    fn has(key) { return self.data.has(key) }
    fn remove(key) { return self.data.remove(key) }
    fn size() { return self.data.size() }
    fn clear() { return self.data.clear() }
}

struct Set {
    fn init() { self.data = set() }
    fn add(key) { return self.data.add(key) }
    fn has(key) { return self.data.has(key) }
    fn remove(key) { return self.data.remove(key) }
    fn size() { return self.data.size() }
    fn clear() { return self.data.clear() }
}

struct DSU {
    fn init(n) {
        self.parent = []
        self.size = []
        i = 0
        while i < n {
            push(self.parent, i)
            push(self.size, 1)
            i++
        }
    }
    fn find(x) {
        if self.parent[x] == x { return x }
        self.parent[x] = self.find(self.parent[x])
        return self.parent[x]
    }
    fn unite(a, b) {
        ra = self.find(a)
        rb = self.find(b)
        if ra == rb { return 0 }
        if self.size[ra] < self.size[rb] {
            t = ra
            ra = rb
            rb = t
        }
        self.parent[rb] = ra
        self.size[ra] = self.size[ra] + self.size[rb]
        return 1
    }
    fn same(a, b) {
        return self.find(a) == self.find(b)
    }
}

struct PriorityQueue {
    fn init() { self.data = [] }
    fn size() { return len(self.data) }
    fn top() { return self.data[0] }
    fn push(v) {
        push(self.data, v)
        i = len(self.data) - 1
        while i > 0 {
            p = (i - 1) / 2
            if self.data[p] >= self.data[i] { i = 0 }
            if i > 0 {
                t = self.data[p]
                self.data[p] = self.data[i]
                self.data[i] = t
                i = p
            }
        }
    }
    fn pop() {
        if len(self.data) == 0 { return 0 }
        out = self.data[0]
        last = pop(self.data)
        if len(self.data) == 0 { return out }
        self.data[0] = last
        i = 0
        done = 0
        while done == 0 {
            l = i * 2 + 1
            r = i * 2 + 2
            if l >= len(self.data) { done = 1 }
            if done == 0 {
                j = l
                if r < len(self.data) && self.data[r] > self.data[l] { j = r }
                if self.data[i] >= self.data[j] { done = 1 }
                if done == 0 {
                    t = self.data[i]
                    self.data[i] = self.data[j]
                    self.data[j] = t
                    i = j
                }
            }
        }
        return out
    }
}

struct MinPriorityQueue {
    fn init() { self.data = [] }
    fn size() { return len(self.data) }
    fn top() { return self.data[0] }
    fn push(v) {
        push(self.data, v)
        i = len(self.data) - 1
        while i > 0 {
            p = (i - 1) / 2
            if self.data[p] <= self.data[i] { i = 0 }
            if i > 0 {
                t = self.data[p]
                self.data[p] = self.data[i]
                self.data[i] = t
                i = p
            }
        }
    }
    fn pop() {
        if len(self.data) == 0 { return 0 }
        out = self.data[0]
        last = pop(self.data)
        if len(self.data) == 0 { return out }
        self.data[0] = last
        i = 0
        done = 0
        while done == 0 {
            l = i * 2 + 1
            r = i * 2 + 2
            if l >= len(self.data) { done = 1 }
            if done == 0 {
                j = l
                if r < len(self.data) && self.data[r] < self.data[l] { j = r }
                if self.data[i] <= self.data[j] { done = 1 }
                if done == 0 {
                    t = self.data[i]
                    self.data[i] = self.data[j]
                    self.data[j] = t
                    i = j
                }
            }
        }
        return out
    }
}

struct Pair {
    fn init(a, b) {
        self.first = a
        self.second = b
    }
}

struct Tuple {
    fn init(values) { self.values = values }
    fn get(i) { return self.values[i] }
    fn set(i, v) { self.values[i] = v }
    fn size() { return len(self.values) }
}
)STDLIB"
