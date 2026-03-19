const sourceEl = document.getElementById("source");
const highlightEl = document.getElementById("source-highlight");
const inputEl = document.getElementById("stdin");
const outputEl = document.getElementById("stdout");
const runBtn = document.getElementById("run");
const clearBtn = document.getElementById("clear");
const acEl = document.getElementById("autocomplete");
const lineNumbersEl = document.getElementById("line-numbers");
const samplesEl = document.getElementById("samples");

const defaultProgram = `fn solve() {
    n = read()
    if n % 2 == 0 {
        println(n ** 2)
    } else {
        println(n)
    }
}

tc = read()
while tc-- {
    solve()
}`;

const defaultInput = `3
2
5
8
`;

sourceEl.value = localStorage.getItem("zenpp-source") ?? defaultProgram;
inputEl.value = localStorage.getItem("zenpp-input") ?? defaultInput;

let zenppModulePromise = null;
let abortSeen = false;

/* ── Syntax highlighting ────────────────────────────── */

function escapeHtml(text) {
  return text
    .replace(/&/g, "&amp;")
    .replace(/</g, "&lt;")
    .replace(/>/g, "&gt;");
}

function highlight(text) {
  // Tokenize raw text first, then escape when building HTML.
  // This avoids the bug where escapeHtml turns > into &gt; and
  // then the regex breaks the entity by matching & as an operator.
  const tokenRegex =
    /\/\/.*|\/\*[\s\S]*?\*\/|"(?:\\.|[^"\\])*"|\b(?:fn|if|else|while|for|return|true|false|struct|import|map|set|self|break|continue|in)\b|\b(?:read|readInt|readFloat|readLine|len|push|pop|print|println|min|max|abs|gcd|lcm|mid|ckmin|ckmax|prefix|binarySearch|dijkstra|ord|chr|parseInt|str|int|float|sort|sortdec|reverse|split|join|find|count|swap|fill|sum|lowerBound|upperBound|modpow|bfs|graph|dgraph|wgraph|dwgraph)\b|\b\d+(?:\.\d+)?\b|\?|:|(\*\*)|==|!=|<=|>=|&&|\|\||[+*/%<>=^|&!\-]/g;

  let result = "";
  let lastIndex = 0;
  let match;

  while ((match = tokenRegex.exec(text)) !== null) {
    // Escape plain text between tokens
    result += escapeHtml(text.slice(lastIndex, match.index));

    const tok = match[0];
    const esc = escapeHtml(tok);
    if (tok.startsWith("//") || tok.startsWith("/*")) {
      result += `<span class="token-comment">${esc}</span>`;
    } else if (tok.startsWith("\"")) {
      result += `<span class="token-string">${esc}</span>`;
    } else if (/^\d/.test(tok)) {
      result += `<span class="token-number">${esc}</span>`;
    } else if (/^(fn|if|else|while|for|return|true|false|struct|import|map|set|self|break|continue|in)$/.test(tok)) {
      result += `<span class="token-keyword">${esc}</span>`;
    } else if (/^(read|readInt|readFloat|readLine|len|push|pop|print|println|min|max|abs|gcd|lcm|mid|ckmin|ckmax|prefix|binarySearch|dijkstra|ord|chr|parseInt|str|int|float|sort|sortdec|reverse|split|join|find|count|swap|fill|sum|lowerBound|upperBound|modpow|bfs|graph|dgraph|wgraph|dwgraph)$/.test(tok)) {
      result += `<span class="token-builtin">${esc}</span>`;
    } else {
      result += `<span class="token-operator">${esc}</span>`;
    }
    lastIndex = tokenRegex.lastIndex;
  }

  // Escape remaining plain text
  result += escapeHtml(text.slice(lastIndex));
  return result;
}

function renderLineNumbers() {
  const lines = sourceEl.value.split("\n").length;
  const nums = [];
  for (let i = 1; i <= lines; i++) nums.push(i);
  lineNumbersEl.textContent = nums.join("\n");
}

function renderHighlight() {
  try {
    highlightEl.innerHTML = highlight(sourceEl.value) + "\n";
  } catch (err) {
    console.error("Highlight error:", err);
    highlightEl.textContent = sourceEl.value + "\n";
  }
  highlightEl.scrollTop = sourceEl.scrollTop;
  highlightEl.scrollLeft = sourceEl.scrollLeft;
  renderLineNumbers();
}

/* ── Tab key support ─────────────────────────────────── */

sourceEl.addEventListener("keydown", (e) => {
  if (e.key === "Tab") {
    e.preventDefault();
    const start = sourceEl.selectionStart;
    const end = sourceEl.selectionEnd;

    if (e.shiftKey) {
      // Shift+Tab: dedent selected lines
      const val = sourceEl.value;
      const lineStart = val.lastIndexOf("\n", start - 1) + 1;
      const lineEnd = end;
      const block = val.slice(lineStart, lineEnd);
      const dedented = block.replace(/^  /gm, "");
      const removed = block.length - dedented.length;
      sourceEl.value = val.slice(0, lineStart) + dedented + val.slice(lineEnd);
      sourceEl.selectionStart = Math.max(lineStart, start - (val.slice(lineStart, start).startsWith("  ") ? 2 : 0));
      sourceEl.selectionEnd = end - removed;
    } else if (start !== end) {
      // Tab with selection: indent all selected lines
      const val = sourceEl.value;
      const lineStart = val.lastIndexOf("\n", start - 1) + 1;
      const block = val.slice(lineStart, end);
      const indented = block.replace(/^/gm, "  ");
      const added = indented.length - block.length;
      sourceEl.value = val.slice(0, lineStart) + indented + val.slice(end);
      sourceEl.selectionStart = start + 2;
      sourceEl.selectionEnd = end + added;
    } else {
      // Simple tab: insert 2 spaces
      sourceEl.value = sourceEl.value.slice(0, start) + "  " + sourceEl.value.slice(end);
      sourceEl.selectionStart = sourceEl.selectionEnd = start + 2;
    }
    sourceEl.dispatchEvent(new Event("input"));
  }

  // Enter: auto-indent
  if (e.key === "Enter") {
    e.preventDefault();
    const val = sourceEl.value;
    const pos = sourceEl.selectionStart;
    const lineStart = val.lastIndexOf("\n", pos - 1) + 1;
    const line = val.slice(lineStart, pos);
    const indent = line.match(/^(\s*)/)[1];
    const extra = line.trimEnd().endsWith("{") ? "  " : "";
    const insert = "\n" + indent + extra;
    sourceEl.value = val.slice(0, pos) + insert + val.slice(sourceEl.selectionEnd);
    sourceEl.selectionStart = sourceEl.selectionEnd = pos + insert.length;
    sourceEl.dispatchEvent(new Event("input"));
  }

  // Closing brace auto-dedent
  if (e.key === "}") {
    const val = sourceEl.value;
    const pos = sourceEl.selectionStart;
    const lineStart = val.lastIndexOf("\n", pos - 1) + 1;
    const before = val.slice(lineStart, pos);
    if (/^\s+$/.test(before) && before.length >= 2) {
      e.preventDefault();
      const dedented = before.slice(2);
      sourceEl.value = val.slice(0, lineStart) + dedented + "}" + val.slice(pos);
      sourceEl.selectionStart = sourceEl.selectionEnd = lineStart + dedented.length + 1;
      sourceEl.dispatchEvent(new Event("input"));
    }
  }
});

/* ── Paste fix: always insert plain text ─────────────── */

sourceEl.addEventListener("paste", (e) => {
  e.preventDefault();
  const text = (e.clipboardData || window.clipboardData).getData("text/plain");
  const start = sourceEl.selectionStart;
  const end = sourceEl.selectionEnd;
  sourceEl.value = sourceEl.value.slice(0, start) + text + sourceEl.value.slice(end);
  sourceEl.selectionStart = sourceEl.selectionEnd = start + text.length;
  sourceEl.dispatchEvent(new Event("input"));
});

/* ── Autocomplete / IntelliSense ─────────────────────── */

const AC_KEYWORDS = [
  // Keywords
  { text: "fn", kind: "keyword" },
  { text: "if", kind: "keyword" },
  { text: "else", kind: "keyword" },
  { text: "while", kind: "keyword" },
  { text: "for", kind: "keyword" },
  { text: "return", kind: "keyword" },
  { text: "true", kind: "keyword" },
  { text: "false", kind: "keyword" },
  { text: "struct", kind: "keyword" },
  { text: "import", kind: "keyword" },
  { text: "map", kind: "keyword" },
  { text: "set", kind: "keyword" },
  { text: "self", kind: "keyword" },
  { text: "break", kind: "keyword" },
  { text: "continue", kind: "keyword" },
  // Built-in functions
  { text: "print()", kind: "builtin", cursor: -1 },
  { text: "println()", kind: "builtin", cursor: -1 },
  { text: "read()", kind: "builtin" },
  { text: "readInt()", kind: "builtin" },
  { text: "readFloat()", kind: "builtin" },
  { text: "readLine()", kind: "builtin" },
  { text: "len()", kind: "builtin", cursor: -1 },
  { text: "push()", kind: "builtin", cursor: -1 },
  { text: "pop()", kind: "builtin", cursor: -1 },
  { text: "ord()", kind: "builtin", cursor: -1 },
  { text: "chr()", kind: "builtin", cursor: -1 },
  { text: "parseInt()", kind: "builtin", cursor: -1 },
  { text: "str()", kind: "builtin", cursor: -1 },
  { text: "int()", kind: "builtin", cursor: -1 },
  { text: "float()", kind: "builtin", cursor: -1 },
  { text: "sort()", kind: "builtin", cursor: -1 },
  { text: "sortdec()", kind: "builtin", cursor: -1 },
  { text: "reverse()", kind: "builtin", cursor: -1 },
  { text: "split()", kind: "builtin", cursor: -1 },
  { text: "join()", kind: "builtin", cursor: -1 },
  { text: "find()", kind: "builtin", cursor: -1 },
  { text: "count()", kind: "builtin", cursor: -1 },
  { text: "swap()", kind: "builtin", cursor: -1 },
  { text: "fill()", kind: "builtin", cursor: -1 },
  { text: "graph()", kind: "builtin", cursor: -1 },
  { text: "dgraph()", kind: "builtin", cursor: -1 },
  { text: "wgraph()", kind: "builtin", cursor: -1 },
  { text: "dwgraph()", kind: "builtin", cursor: -1 },
  // Stdlib math (import std)
  { text: "min()", kind: "stdlib", cursor: -1 },
  { text: "max()", kind: "stdlib", cursor: -1 },
  { text: "abs()", kind: "stdlib", cursor: -1 },
  { text: "gcd()", kind: "stdlib", cursor: -1 },
  { text: "lcm()", kind: "stdlib", cursor: -1 },
  { text: "mid()", kind: "stdlib", cursor: -1 },
  { text: "ckmin()", kind: "stdlib", cursor: -1 },
  { text: "ckmax()", kind: "stdlib", cursor: -1 },
  { text: "prefix()", kind: "stdlib", cursor: -1 },
  { text: "binarySearch()", kind: "stdlib", cursor: -1 },
  { text: "dijkstra()", kind: "stdlib", cursor: -1 },
  { text: "sum()", kind: "stdlib", cursor: -1 },
  { text: "lowerBound()", kind: "stdlib", cursor: -1 },
  { text: "upperBound()", kind: "stdlib", cursor: -1 },
  { text: "modpow()", kind: "stdlib", cursor: -1 },
  { text: "bfs()", kind: "stdlib", cursor: -1 },
  // Stdlib data structures (import std)
  { text: "Stack()", kind: "stdlib" },
  { text: "Queue()", kind: "stdlib" },
  { text: "DSU()", kind: "stdlib" },
  { text: "PriorityQueue()", kind: "stdlib" },
  { text: "MinPriorityQueue()", kind: "stdlib" },
  { text: "Pair()", kind: "stdlib" },
  { text: "Tuple()", kind: "stdlib" },
  // Snippets
  { text: "import std", kind: "snippet" },
];

let acActive = false;
let acIndex = 0;
let acMatches = [];
let acPrefix = "";

function getWordBefore(text, pos) {
  let i = pos - 1;
  while (i >= 0 && /[a-zA-Z_]/.test(text[i])) i--;
  return text.slice(i + 1, pos);
}

function getCaretCoords() {
  // Create a mirror div to measure caret position
  const mirror = document.createElement("div");
  const style = getComputedStyle(sourceEl);
  for (const prop of ["fontFamily", "fontSize", "lineHeight", "padding", "paddingTop", "paddingLeft", "border", "whiteSpace", "wordWrap", "overflowWrap", "tabSize", "letterSpacing"]) {
    mirror.style[prop] = style[prop];
  }
  mirror.style.position = "absolute";
  mirror.style.visibility = "hidden";
  mirror.style.whiteSpace = "pre-wrap";
  mirror.style.wordWrap = "break-word";
  mirror.style.width = sourceEl.clientWidth + "px";
  mirror.style.height = "auto";

  const text = sourceEl.value.slice(0, sourceEl.selectionStart);
  mirror.textContent = text;
  const span = document.createElement("span");
  span.textContent = "|";
  mirror.appendChild(span);
  document.body.appendChild(mirror);

  const rect = sourceEl.getBoundingClientRect();
  const spanRect = span.getBoundingClientRect();
  const mirrorRect = mirror.getBoundingClientRect();

  const x = spanRect.left - mirrorRect.left - sourceEl.scrollLeft;
  const y = spanRect.top - mirrorRect.top - sourceEl.scrollTop + parseInt(style.lineHeight);

  document.body.removeChild(mirror);
  return { x, y };
}

function showAc(matches, prefix) {
  acMatches = matches;
  acPrefix = prefix;
  acIndex = 0;
  acActive = true;

  acEl.innerHTML = matches
    .map((m, i) => `<div class="ac-item${i === 0 ? " active" : ""}" data-i="${i}"><span>${escapeHtml(m.text)}</span><span class="ac-kind">${m.kind}</span></div>`)
    .join("");
  acEl.classList.add("visible");

  const coords = getCaretCoords();
  acEl.style.left = coords.x + "px";
  acEl.style.top = coords.y + "px";
}

function hideAc() {
  acActive = false;
  acMatches = [];
  acEl.classList.remove("visible");
}

function applyAc(item) {
  const pos = sourceEl.selectionStart;
  const val = sourceEl.value;
  const insert = item.text.slice(acPrefix.length);
  sourceEl.value = val.slice(0, pos) + insert + val.slice(pos);
  let newPos = pos + insert.length;
  if (item.cursor) newPos += item.cursor;
  sourceEl.selectionStart = sourceEl.selectionEnd = newPos;
  sourceEl.dispatchEvent(new Event("input"));
  hideAc();
  sourceEl.focus();
}

sourceEl.addEventListener("input", () => {
  renderHighlight();
  localStorage.setItem("zenpp-source", sourceEl.value);

  // Autocomplete trigger
  const word = getWordBefore(sourceEl.value, sourceEl.selectionStart);
  if (word.length >= 2) {
    const lower = word.toLowerCase();
    const matches = AC_KEYWORDS.filter((k) => k.text.toLowerCase().startsWith(lower) && k.text.toLowerCase() !== lower).slice(0, 7);
    if (matches.length > 0) {
      showAc(matches, word);
      return;
    }
  }
  hideAc();
});

sourceEl.addEventListener("keydown", (e) => {
  if (!acActive) return;

  if (e.key === "ArrowDown") {
    e.preventDefault();
    acIndex = (acIndex + 1) % acMatches.length;
    updateAcSelection();
  } else if (e.key === "ArrowUp") {
    e.preventDefault();
    acIndex = (acIndex - 1 + acMatches.length) % acMatches.length;
    updateAcSelection();
  } else if (e.key === "Enter" || e.key === "Tab") {
    if (acActive && acMatches.length > 0) {
      e.preventDefault();
      e.stopImmediatePropagation();
      applyAc(acMatches[acIndex]);
    }
  } else if (e.key === "Escape") {
    e.preventDefault();
    hideAc();
  }
}, true); // capture phase so it fires before the tab/enter handlers

acEl.addEventListener("mousedown", (e) => {
  e.preventDefault(); // don't blur textarea
  const item = e.target.closest(".ac-item");
  if (item) {
    applyAc(acMatches[parseInt(item.dataset.i)]);
  }
});

function updateAcSelection() {
  const items = acEl.querySelectorAll(".ac-item");
  items.forEach((el, i) => el.classList.toggle("active", i === acIndex));
  items[acIndex]?.scrollIntoView({ block: "nearest" });
}

sourceEl.addEventListener("blur", () => {
  // Small delay so click on autocomplete can fire
  setTimeout(hideAc, 150);
});

/* ── WASM runtime ───────────────────────────────────── */

function loadModule() {
  if (!zenppModulePromise) {
    if (typeof Zenpp === "undefined") {
      return Promise.reject(new Error("Missing web/zenpp.js. Run the wasm build first."));
    }
    zenppModulePromise = Zenpp({
      locateFile: (path) => path,
      printErr: (text) => {
        console.error(text);
      },
      onAbort: (reason) => {
        abortSeen = true;
        console.error("WASM abort:", reason);
      },
    });
  }
  return zenppModulePromise;
}

function setOutput(text) {
  outputEl.textContent = text;
}

runBtn.addEventListener("click", async () => {
  setOutput("Running...");
  try {
    const module = await loadModule();
    if (!module || !module.cwrap) {
      setOutput("Runtime error: wasm module failed to initialize");
      return;
    }
    if (!module._zenpp_eval) {
      setOutput("Runtime error: missing zenpp_eval export (rebuild wasm)");
      return;
    }
    const evalFn = module.cwrap("zenpp_eval", "number", ["string", "string"]);
    if (abortSeen) {
      setOutput("Runtime error: wasm aborted (see console). Rebuild web/zenpp.js + web/zenpp.wasm.");
      return;
    }
    const ptr = evalFn(sourceEl.value, inputEl.value);
    if (!ptr) {
      setOutput("Runtime error: failed to allocate output buffer");
      return;
    }
    const text = module.UTF8ToString(ptr);
    module._free(ptr);
    setOutput(text || "(no output)");
  } catch (err) {
    if (typeof err === "number") {
      setOutput("Runtime error: program threw an unhandled exception");
    } else {
      setOutput(`Error: ${err.message || err}`);
    }
  }
});

clearBtn.addEventListener("click", () => {
  inputEl.value = "";
  localStorage.setItem("zenpp-input", "");
});

inputEl.addEventListener("input", () => {
  localStorage.setItem("zenpp-input", inputEl.value);
});

sourceEl.addEventListener("scroll", () => {
  highlightEl.scrollTop = sourceEl.scrollTop;
  highlightEl.scrollLeft = sourceEl.scrollLeft;
  lineNumbersEl.scrollTop = sourceEl.scrollTop;
});

/* ── Sample programs ────────────────────────────────── */

const SAMPLES = [
  {
    name: "Fibonacci",
    code: `// Fibonacci — read t test cases, print fib(n) for each
fn fib(n) {
    if n <= 1 { return n }
    return fib(n - 1) + fib(n - 2)
}

t = read()
while t-- {
    println(fib(read()))
}`,
    input: "5\n5\n10\n1\n0\n20",
  },
  {
    name: "Binary Search",
    code: `// Binary search on a sorted array
fn binary_search(v, target) {
    lo = 0
    hi = len(v) - 1
    while lo <= hi {
        mid = (lo + hi) / 2
        if v[mid] == target {
            return mid
        } else if v[mid] < target {
            lo = mid + 1
        } else {
            hi = mid - 1
        }
    }
    return -1
}

arr = [2, 5, 8, 12, 16, 23, 38, 56, 72, 91]
println(binary_search(arr, 23))
println(binary_search(arr, 10))`,
    input: "",
  },
  {
    name: "Sieve of Eratosthenes",
    code: `// Sieve of Eratosthenes — print all primes up to n
n = read()
is_prime = []
for i n + 1 {
    push(is_prime, 1)
}
is_prime[0] = 0
is_prime[1] = 0

for i 2 n + 1 {
    if is_prime[i] {
        j = i * i
        while j <= n {
            is_prime[j] = 0
            j += i
        }
    }
}

for i 2 n + 1 {
    if is_prime[i] {
        println(i)
    }
}`,
    input: "100",
  },
  {
    name: "For-each & Break/Continue",
    code: `// For-each loops, break, and continue

// iterate a vector
fruits = ["apple", "banana", "cherry"]
for fruit in fruits {
    println(fruit)
}

// break out of a loop early
for i 100 {
    if i == 5 { break }
    println(i)
}

// skip even numbers with continue
for i 10 {
    if i % 2 == 0 { continue }
    println(i)
}

// for-each over a map
m = map()
m["alice"] = 95
m["bob"] = 87
m["charlie"] = 92
for key in m {
    println(key + ": " + m[key])
}`,
    input: "",
  },
  {
    name: "Maps, Sets & Structs",
    code: `// Map and set bracket indexing

// word frequency with maps
freq = map()
words = ["the", "cat", "sat", "on", "the", "mat", "the"]
for w in words {
    freq[w] = freq[w] + 1
}

println("the: " + freq["the"])
println("cat: " + freq["cat"])
println("mat: " + freq["mat"])

// set membership
seen = set()
seen.add(10)
seen.add(20)
seen.add(30)

println(seen[10])    // 1
println(seen[99])    // 0

// struct with field defaults
struct Counter {
    count = 0

    fn inc() {
        self.count = self.count + 1
    }
    fn get() {
        return self.count
    }
}

c = Counter()
c.inc()
c.inc()
c.inc()
println(c.get())     // 3`,
    input: "",
  },
  {
    name: "DSU (Union-Find)",
    code: `// Disjoint Set Union demo
import std

d = DSU()
d.init(6)

d.unite(0, 1)
d.unite(2, 3)
d.unite(4, 5)

println(d.same(0, 1))   // 1
println(d.same(0, 2))   // 0

d.unite(1, 3)
println(d.same(0, 2))   // 1
println(d.same(0, 4))   // 0`,
    input: "",
  },
  {
    name: "GCD & Math (stdlib)",
    code: `// Standard library math functions
import std

println(gcd(12, 18))     // 6
println(gcd(100, 75))    // 25
println(min(3, 7))       // 3
println(max(3, 7))       // 7
println(abs(-42))        // 42

// Euclidean algorithm by hand
fn my_gcd(a, b) {
    while b != 0 {
        t = a % b
        a = b
        b = t
    }
    return a
}

println(my_gcd(48, 18))  // 6`,
    input: "",
  },
  {
    name: "Stack & Queue (stdlib)",
    code: `// Stack and Queue from stdlib
import std

// Stack: LIFO
s = Stack()
s.init()
s.push(10)
s.push(20)
s.push(30)
println("Stack size: " + s.size())
println("Top: " + s.peek())
println("Pop: " + s.pop())
println("Pop: " + s.pop())

// Queue: FIFO
q = Queue()
q.init()
q.push(1)
q.push(2)
q.push(3)
println("Queue pop: " + q.pop())
println("Queue pop: " + q.pop())
println("Queue pop: " + q.pop())`,
    input: "",
  },
  {
    name: "CF 2200A — Eating Game",
    code: `// Codeforces 2200A - Eating Game
// Simulate circular eating for each starting player
import std

fn solve() {
    n = read()
    a = []
    for i n { push(a, read()) }

    winners = set()
    for s n {
        b = []
        for i n { push(b, a[i]) }

        total = 0
        for i n { total += b[i] }

        last = 0
        cur = s
        while total > 0 {
            if b[cur] > 0 {
                b[cur] = b[cur] - 1
                total = total - 1
                last = cur
            }
            cur = (cur + 1) % n
        }
        winners.add(last)
    }
    println(winners.size())
}

t = read()
while t-- { solve() }`,
    input: "3\n1\n10\n2\n6 7\n4\n1 4 3 4",
  },
  {
    name: "CF 2200B — Deletion Sort",
    code: `// Codeforces 2200B - Deletion Sort
// If already sorted, answer is n; otherwise 1

fn solve() {
    n = read()
    a = []
    for i n { push(a, read()) }

    sorted = 1
    for i 1 n {
        if a[i] < a[i - 1] {
            sorted = 0
            break
        }
    }
    if sorted { println(n) } else { println(1) }
}

t = read()
while t-- { solve() }`,
    input: "3\n4\n1 4 2 3\n1\n100\n2\n6 7",
  },
  {
    name: "CF 2200C — Specialty String",
    code: `// Codeforces 2200C - Specialty String
// Stack-based: cancel adjacent equal chars
import std

fn solve() {
    n = read()
    s = readLine()

    st = Stack()
    st.init()
    for i len(s) {
        c = s[i]
        if st.size() > 0 && c == st.peek() {
            st.pop()
        } else {
            st.push(c)
        }
    }
    if st.size() == 0 { println("YES") } else { println("NO") }
}

t = read()
while t-- { solve() }`,
    input: "6\n1\na\n6\nllmllm\n6\nuwuuwu\n6\nbyebye\n6\noooioi\n12\nsiixxsevvenn",
  },
  {
    name: "CF 2200D — Portal",
    code: `// Codeforces 2200D - Portal
// Split array, rotate inner segment, merge back

fn solve() {
    n = read()
    x = read()
    y = read()
    x--
    y--

    a = []
    b = []
    for i n {
        j = read()
        if i <= x || i > y {
            push(a, j)
        } else {
            push(b, j)
        }
    }

    if len(b) > 0 {
        mi = 0
        for i 1 len(b) {
            if b[i] < b[mi] { mi = i }
        }
        nb = []
        for i mi len(b) { push(nb, b[i]) }
        for i mi { push(nb, b[i]) }
        b = nb
    }

    m = -1
    if len(b) > 0 { m = b[0] }

    pos = len(a)
    for i len(a) {
        if a[i] >= m {
            pos = i
            break
        }
    }

    res = []
    for i pos { push(res, a[i]) }
    for i len(b) { push(res, b[i]) }
    for i pos len(a) { push(res, a[i]) }
    for i len(res) {
        if i > 0 { print(" ") }
        print(res[i])
    }
    println("")
}

t = read()
while t-- { solve() }`,
    input: "4\n4 0 4\n3 1 4 2\n3 1 2\n3 2 1\n5 1 3\n1 3 5 2 4\n2 0 1\n1 2",
  },
  {
    name: "CF 2200E — Divisive Battle",
    code: `// Codeforces 2200E - Divisive Battle
// Check prime structure to determine Alice vs Bob

fn primebase(x) {
    prime = 0
    i = 2
    while i * i <= x {
        if x % i == 0 {
            if prime > 0 { return -1 }
            prime = i
            while x % i == 0 { x /= i }
        }
        i++
    }
    if x > 1 {
        if prime > 0 { return -1 }
        prime = x
    }
    if prime == 0 { return 1 }
    return prime
}

fn is_sorted(a) {
    for i 1 len(a) {
        if a[i] < a[i - 1] { return 0 }
    }
    return 1
}

fn solve() {
    n = read()
    a = []
    for i n { push(a, read()) }

    b = []
    for i n { push(b, primebase(a[i])) }

    if is_sorted(a) {
        println("Bob")
    } else {
        has_multi = 0
        for x in b {
            if x == -1 {
                has_multi = 1
                break
            }
        }
        if has_multi {
            println("Alice")
        } else if is_sorted(b) {
            println("Bob")
        } else {
            println("Alice")
        }
    }
}

t = read()
while t-- { solve() }`,
    input: "4\n10\n10 9 8 7 6 5 4 3 2 1\n3\n1 8192 677\n2\n6 5\n2\n6 7",
  },
  {
    name: "Bulk I/O & Casting",
    code: `// Bulk I/O and type casting
// read(n) reads n values into a vector at once

n = read()
v = read(n)
println(v)

// type casting
x = 42
println(str(x) + " is a string now")

s = "123"
println(int(s) + 1)

f = 3.7
println(int(f))
println(float(10) + 0.5)`,
    input: "5\n10 20 30 40 50",
  },
  {
    name: "Prefix Sums (stdlib)",
    code: `// Prefix sum — answer range sum queries in O(1)
import std

n = read()
q = read()
a = read(n)

p = prefix(a)

// answer q range sum queries [l, r]
for i q {
    l = read()
    r = read()
    println(p[r + 1] - p[l])
}`,
    input: "5 3\n1 2 3 4 5\n0 4\n1 3\n2 2",
  },
  {
    name: "Dijkstra (stdlib)",
    code: `// Dijkstra's shortest path
import std

n, m = read(), read()
adj = wgraph(n, m)
dist = dijkstra(adj, 0)
println(dist)`,
    input: "4 5\n0 1 4\n0 2 1\n2 1 2\n1 3 1\n2 3 5",
  },
  {
    name: "String Methods",
    code: `// String manipulation functions

// case conversion
println(upper("hello world"))    // HELLO WORLD
println(lower("HELLO WORLD"))    // hello world

// replace all occurrences
s = "aababca"
println(replace(s, "a", "x"))    // xxbxbcx
println(replace(s, "ab", ""))    // aaca

// prefix/suffix checks
filename = "solution.zpp"
println(startswith(filename, "solution"))  // 1
println(endswith(filename, ".zpp"))        // 1

// contains
line = "the quick brown fox"
println(contains(line, "brown"))  // 1

// trim whitespace
println(trim("   hello   "))     // hello

// substring (supports negative index)
s = "hello world"
println(substr(s, 6))            // world
println(substr(s, 0, 5))         // hello
println(substr(s, -5))           // world

// slicing works on strings too
s = "abcdefgh"
println(s[0:3])                  // abc
println(s[::-1])                 // hgfedcba

// palindrome check using slicing + lower
fn isPalindrome(s) {
    s = lower(s)
    return s == s[::-1]
}
println(isPalindrome("racecar"))  // 1
println(isPalindrome("Madam"))    // 1
println(isPalindrome("hello"))    // 0`,
    input: "",
  },
  {
    name: "Bitwise Operations",
    code: `// Bitwise shift operators and common patterns

// shift operators
println(1 << 10)    // 1024
println(1024 >> 3)  // 128

// compound shift assignment
x = 1
x <<= 8
println(x)          // 256
x >>= 3
println(x)          // 32

// check if i-th bit is set
fn getBit(n, i) {
    return (n >> i) & 1
}
println(getBit(13, 0))   // 1 (13 = 1101)
println(getBit(13, 1))   // 0
println(getBit(13, 2))   // 1

// set the i-th bit
fn setBit(n, i) {
    return n | (1 << i)
}
println(setBit(0, 3))    // 8

// toggle the i-th bit
fn toggleBit(n, i) {
    return n ^ (1 << i)
}
println(toggleBit(13, 1))  // 15 (1101 -> 1111)

// popcount (count set bits)
fn popcount(n) {
    cnt = 0
    while n > 0 {
        cnt = cnt + (n & 1)
        n >>= 1
    }
    return cnt
}
println(popcount(13))     // 3 (1101)
println(popcount(255))    // 8

// check power of 2
fn isPow2(n) {
    return n > 0 && (n & (n - 1)) == 0
}
println(isPow2(16))   // 1
println(isPow2(15))   // 0`,
    input: "",
  },
  {
    name: "Sorting & Lambdas",
    code: `// Sorting with inline lambda comparators

// basic sort
v = [5, 3, 1, 4, 2]
sort(v)
println(v)   // [1, 2, 3, 4, 5]

// sort descending with lambda
v = [5, 3, 1, 4, 2]
sort(v, fn(a, b) { b - a })
println(v)   // [5, 4, 3, 2, 1]

// sort pairs by second element
pairs = [[1, 30], [2, 10], [3, 20]]
sort(pairs, fn(a, b) { a[1] - b[1] })
println(pairs)   // [[2, 10], [3, 20], [1, 30]]

// sort strings by length
words = ["banana", "fig", "apple", "kiwi"]
sort(words, fn(a, b) { len(a) - len(b) })
println(words)   // [fig, kiwi, apple, banana]

// named comparator still works too
fn bySecThenFirst(a, b) {
    if a[1] != b[1] { return a[1] - b[1] }
    return a[0] - b[0]
}
items = [[2, 1], [1, 1], [3, 2]]
sort(items, bySecThenFirst)
println(items)   // [[1, 1], [2, 1], [3, 2]]

// slicing for reversed copy (non-destructive)
v = [1, 2, 3, 4, 5]
println(v[::-1])   // [5, 4, 3, 2, 1]
println(v)         // [1, 2, 3, 4, 5] (unchanged)`,
    input: "",
  },
  {
    name: "Tuple Unpacking & Defaults",
    code: `// Tuple unpacking in for-each loops

// unpack vector of vectors
edges = [[1, 2], [3, 4], [5, 6]]
for u, v in edges {
    println(u, "->", v)
}

// unpack key-value pairs from a map
scores = map()
scores["alice"] = 95
scores["bob"] = 87
scores["charlie"] = 92
for name, score in scores {
    println(name + ": " + score)
}

// skip values with _
pairs = [[10, 20], [30, 40], [50, 60]]
for _, second in pairs {
    println(second)
}

// default function arguments
fn greet(name, greeting = "Hello") {
    println(greeting, name)
}
greet("World")          // Hello World
greet("World", "Hi")    // Hi World

fn power(base, exp = 2) {
    result = 1
    for _ exp {
        result = result * base
    }
    return result
}
println(power(5))       // 25
println(power(5, 3))    // 125

// multi-dimensional fill
grid = fill(3, 4, 0)
println(grid)
grid[1][2] = 42
println(grid[1])   // only row 1 changed
println(grid[0])   // row 0 still zeros`,
    input: "",
  },
  {
    name: "Slicing",
    code: `// Python-style slicing for vectors and strings

v = [10, 20, 30, 40, 50]

// [start:end] — elements from start up to (not including) end
println(v[1:3])     // [20, 30]

// [:end] — first N elements
println(v[:2])      // [10, 20]

// [start:] — from start to end
println(v[3:])      // [40, 50]

// [::step] — every Nth element
println(v[::2])     // [10, 30, 50]

// [::-1] — reversed copy
println(v[::-1])    // [50, 40, 30, 20, 10]

// negative indices
println(v[-2:])     // [40, 50]
println(v[-3:-1])   // [30, 40]

// strings work the same way
s = "hello world"
println(s[0:5])     // hello
println(s[6:])      // world
println(s[::-1])    // dlrow olleh

// practical: check palindrome
word = "racecar"
println(word == word[::-1])  // 1

// practical: reverse a vector without modifying original
original = [1, 2, 3, 4, 5]
reversed = original[::-1]
println(original)   // [1, 2, 3, 4, 5]
println(reversed)   // [5, 4, 3, 2, 1]`,
    input: "",
  },
];

// Populate dropdown with groups
const groups = [
  { label: "Basics", items: [0, 1, 2] },
  { label: "Language Features", items: [3, 4, 13, 16, 17, 18, 19, 20] },
  { label: "Stdlib (import std)", items: [5, 6, 7, 14, 15] },
  { label: "Codeforces 2200", items: [8, 9, 10, 11, 12] },
];

groups.forEach((g) => {
  const optgroup = document.createElement("optgroup");
  optgroup.label = g.label;
  g.items.forEach((i) => {
    const opt = document.createElement("option");
    opt.value = i;
    opt.textContent = SAMPLES[i].name;
    optgroup.appendChild(opt);
  });
  samplesEl.appendChild(optgroup);
});

samplesEl.addEventListener("change", () => {
  const idx = samplesEl.value;
  if (idx === "") return;
  const sample = SAMPLES[idx];
  sourceEl.value = sample.code;
  inputEl.value = sample.input;
  localStorage.setItem("zenpp-source", sample.code);
  localStorage.setItem("zenpp-input", sample.input);
  renderHighlight();
  outputEl.textContent = "";
  samplesEl.value = "";
});

renderHighlight();
