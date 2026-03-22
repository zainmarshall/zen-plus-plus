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
    /\/\/.*|\/\*[\s\S]*?\*\/|f"(?:\\.|[^"\\])*"|"(?:\\.|[^"\\])*"|\b(?:fn|if|else|while|for|return|true|false|struct|import|map|set|self|break|continue|in)\b|\b(?:read|readInt|readFloat|readLine|len|push|pop|print|println|min|max|abs|gcd|lcm|mid|ckmin|ckmax|prefix|binarySearch|dijkstra|ord|chr|parseInt|str|int|float|sort|sortdec|sorted|reverse|unique|flatten|zip|split|join|find|count|swap|fill|sum|lowerBound|upperBound|modpow|bfs|graph|dgraph|wgraph|dwgraph|replace|upper|lower|trim|contains|startswith|endswith|substr|all|any|rand|randvec|exit)\b|\b\d+(?:\.\d+)?\b|~|<<|>>|\?|:|(\*\*)|==|!=|<=|>=|&&|\|\||[+*/%<>=^|&!\-]/g;

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
    } else if (tok.startsWith("f\"")) {
      // f-string: highlight the f and quotes as string, expressions inside {} as normal
      let fstr = '<span class="token-string">f"</span>';
      const inner = tok.slice(2, -1); // strip f" and "
      let fi = 0;
      while (fi < inner.length) {
        if (inner[fi] === '{') {
          let depth = 1, fj = fi + 1;
          while (fj < inner.length && depth > 0) {
            if (inner[fj] === '{') depth++;
            else if (inner[fj] === '}') depth--;
            fj++;
          }
          const expr = inner.slice(fi + 1, fj - 1);
          fstr += '<span class="token-string">{</span>' + escapeHtml(expr) + '<span class="token-string">}</span>';
          fi = fj;
        } else {
          let fj = fi;
          while (fj < inner.length && inner[fj] !== '{') fj++;
          fstr += '<span class="token-string">' + escapeHtml(inner.slice(fi, fj)) + '</span>';
          fi = fj;
        }
      }
      fstr += '<span class="token-string">"</span>';
      result += fstr;
    } else if (tok.startsWith("\"")) {
      result += `<span class="token-string">${esc}</span>`;
    } else if (/^\d/.test(tok)) {
      result += `<span class="token-number">${esc}</span>`;
    } else if (/^(fn|if|else|while|for|return|true|false|struct|import|map|set|self|break|continue|in)$/.test(tok)) {
      result += `<span class="token-keyword">${esc}</span>`;
    } else if (/^(read|readInt|readFloat|readLine|len|push|pop|print|println|min|max|abs|gcd|lcm|mid|ckmin|ckmax|prefix|binarySearch|dijkstra|ord|chr|parseInt|str|int|float|sort|sortdec|sorted|reverse|unique|flatten|zip|split|join|find|count|swap|fill|sum|lowerBound|upperBound|modpow|bfs|graph|dgraph|wgraph|dwgraph|replace|upper|lower|trim|contains|startswith|endswith|substr|all|any|rand|randvec|exit)$/.test(tok)) {
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
  { text: "in", kind: "keyword" },
  // I/O
  { text: "print(...)", kind: "builtin", hint: "print values, no newline", cursor: -4 },
  { text: "println(...)", kind: "builtin", hint: "print values + newline", cursor: -4 },
  { text: "read()", kind: "builtin", hint: "read int from stdin" },
  { text: "read(n)", kind: "builtin", hint: "read n ints into vector", cursor: -1 },
  { text: "readLine()", kind: "builtin", hint: "read line as string" },
  { text: "readFloat()", kind: "builtin", hint: "read float from stdin" },
  // Collections
  { text: "len(x)", kind: "builtin", hint: "length of string/vector", cursor: -1 },
  { text: "push(v, x)", kind: "builtin", hint: "append x to vector v", cursor: -4 },
  { text: "pop(v)", kind: "builtin", hint: "remove & return last element", cursor: -1 },
  { text: "sort(v)", kind: "builtin", hint: "sort vector ascending", cursor: -1 },
  { text: "sort(v, fn(a,b){...})", kind: "builtin", hint: "sort with comparator", cursor: -16 },
  { text: "sortdec(v)", kind: "builtin", hint: "sort vector descending", cursor: -1 },
  { text: "sorted(v)", kind: "builtin", hint: "return sorted copy", cursor: -1 },
  { text: "reverse(v)", kind: "builtin", hint: "reverse in-place", cursor: -1 },
  { text: "unique(v)", kind: "builtin", hint: "remove consecutive dupes", cursor: -1 },
  { text: "fill(n, val)", kind: "builtin", hint: "vector of n copies", cursor: -5 },
  { text: "fill(n, m, val)", kind: "builtin", hint: "n×m grid", cursor: -5 },
  { text: "flatten(v)", kind: "builtin", hint: "flatten one level", cursor: -1 },
  { text: "zip(a, b)", kind: "builtin", hint: "pair up two vectors", cursor: -4 },
  { text: "find(v, x)", kind: "builtin", hint: "first index of x, -1 if missing", cursor: -4 },
  { text: "count(v, x)", kind: "builtin", hint: "count occurrences", cursor: -4 },
  { text: "swap(v, i, j)", kind: "builtin", hint: "swap elements at i,j", cursor: -5 },
  // Strings
  { text: "split(s, delim)", kind: "builtin", hint: "split string by delimiter", cursor: -7 },
  { text: "join(v, delim)", kind: "builtin", hint: "join vector into string", cursor: -7 },
  { text: "replace(s, old, new)", kind: "builtin", hint: "replace all occurrences", cursor: -10 },
  { text: "upper(s)", kind: "builtin", hint: "uppercase string", cursor: -1 },
  { text: "lower(s)", kind: "builtin", hint: "lowercase string", cursor: -1 },
  { text: "trim(s)", kind: "builtin", hint: "strip whitespace", cursor: -1 },
  { text: "contains(s, sub)", kind: "builtin", hint: "check substring", cursor: -5 },
  { text: "startswith(s, pre)", kind: "builtin", hint: "check prefix", cursor: -5 },
  { text: "endswith(s, suf)", kind: "builtin", hint: "check suffix", cursor: -5 },
  { text: "substr(s, start)", kind: "builtin", hint: "substring from start", cursor: -7 },
  { text: "substr(s, start, len)", kind: "builtin", hint: "substring with length", cursor: -11 },
  // Type casting
  { text: "str(x)", kind: "builtin", hint: "convert to string", cursor: -1 },
  { text: "int(x)", kind: "builtin", hint: "convert to integer", cursor: -1 },
  { text: "float(x)", kind: "builtin", hint: "convert to float", cursor: -1 },
  { text: "ord(s)", kind: "builtin", hint: "ASCII code of char", cursor: -1 },
  { text: "chr(n)", kind: "builtin", hint: "char from ASCII code", cursor: -1 },
  // Predicates
  { text: "all(v, fn(x){...})", kind: "builtin", hint: "true if all pass", cursor: -13 },
  { text: "any(v, fn(x){...})", kind: "builtin", hint: "true if any passes", cursor: -13 },
  { text: "min(v)", kind: "builtin", hint: "min of vector", cursor: -1 },
  { text: "max(v)", kind: "builtin", hint: "max of vector", cursor: -1 },
  // Random
  { text: "rand(lo, hi)", kind: "builtin", hint: "random int in [lo,hi]", cursor: -5 },
  { text: "randvec(n, lo, hi)", kind: "builtin", hint: "vector of n random ints", cursor: -8 },
  { text: "exit()", kind: "builtin", hint: "terminate program" },
  // Graphs
  { text: "graph(n)", kind: "builtin", hint: "empty adjacency list", cursor: -1 },
  { text: "graph(n, m)", kind: "builtin", hint: "read m undirected edges", cursor: -4 },
  { text: "wgraph(n, m)", kind: "builtin", hint: "read m weighted edges", cursor: -4 },
  { text: "dgraph(n, m)", kind: "builtin", hint: "read m directed edges", cursor: -4 },
  { text: "dwgraph(n, m)", kind: "builtin", hint: "read m directed weighted", cursor: -4 },
  // Stdlib math (import std)
  { text: "min(a, b)", kind: "stdlib", hint: "smaller of two values", cursor: -4 },
  { text: "max(a, b)", kind: "stdlib", hint: "larger of two values", cursor: -4 },
  { text: "abs(x)", kind: "stdlib", hint: "absolute value", cursor: -1 },
  { text: "gcd(a, b)", kind: "stdlib", hint: "greatest common divisor", cursor: -4 },
  { text: "lcm(a, b)", kind: "stdlib", hint: "least common multiple", cursor: -4 },
  { text: "modpow(base, exp, mod)", kind: "stdlib", hint: "modular exponentiation", cursor: -10 },
  { text: "prefix(v)", kind: "stdlib", hint: "prefix sum array", cursor: -1 },
  { text: "sum(v)", kind: "stdlib", hint: "sum of vector", cursor: -1 },
  { text: "binarySearch(v, target)", kind: "stdlib", hint: "returns index or -1", cursor: -8 },
  { text: "lowerBound(v, x)", kind: "stdlib", hint: "first index >= x", cursor: -4 },
  { text: "upperBound(v, x)", kind: "stdlib", hint: "first index > x", cursor: -4 },
  { text: "dijkstra(adj, start)", kind: "stdlib", hint: "shortest paths", cursor: -7 },
  { text: "bfs(adj, start)", kind: "stdlib", hint: "unweighted shortest paths", cursor: -7 },
  // Stdlib data structures (import std)
  { text: "Stack()", kind: "stdlib", hint: "LIFO stack" },
  { text: "Queue()", kind: "stdlib", hint: "FIFO queue" },
  { text: "DSU()", kind: "stdlib", hint: "disjoint set union" },
  { text: "PriorityQueue()", kind: "stdlib", hint: "max-heap" },
  { text: "MinPriorityQueue()", kind: "stdlib", hint: "min-heap" },
  { text: "FenwickTree()", kind: "stdlib", hint: "point update + prefix sum" },
  { text: "SegTree()", kind: "stdlib", hint: "range query + point update" },
  { text: "Pair()", kind: "stdlib", hint: "pair of values" },
  { text: "Tuple()", kind: "stdlib", hint: "tuple of values" },
  // Snippets
  { text: "import std", kind: "snippet" },
  { text: "fn name() {\n    \n}", kind: "snippet", hint: "function definition", cursor: -2 },
  { text: "for i n {\n    \n}", kind: "snippet", hint: "for loop", cursor: -2 },
  { text: "while cond {\n    \n}", kind: "snippet", hint: "while loop", cursor: -2 },
  { text: "if cond {\n    \n}", kind: "snippet", hint: "if block", cursor: -2 },
  { text: "struct Name {\n    fn init() {\n        \n    }\n}", kind: "snippet", hint: "struct definition", cursor: -6 },
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
    .map((m, i) => `<div class="ac-item${i === 0 ? " active" : ""}" data-i="${i}"><span>${escapeHtml(m.text)}</span><span class="ac-kind${m.hint ? " has-hint" : ""}">${m.hint || m.kind}</span></div>`)
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
    // Match on function name (before first paren/space), deduplicate by name
    const seen = new Set();
    const matches = AC_KEYWORDS.filter((k) => {
      const name = k.text.split(/[(\s]/)[0].toLowerCase();
      if (!name.startsWith(lower) || name === lower) return false;
      if (seen.has(name)) return false;
      seen.add(name);
      return true;
    }).slice(0, 8);
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
println(binary_search(arr, 23))    // 5
println(binary_search(arr, 10))    // -1`,
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

// map bracket syntax
freq = map()
words = ["the", "cat", "sat", "on", "the", "mat", "the"]
for w in words {
    freq[w] = freq[w] + 1
}

println("the: " + freq["the"])
println("cat: " + freq["cat"])
println("mat: " + freq["mat"])

// set bracket syntax for membership
seen = set()
seen.add(10)
seen.add(20)
seen.add(30)

println(seen[10])    // 1
println(seen[99])    // 0

// struct field defaults
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
    name: "Slicing",
    code: `// Slicing — Python-style slicing for vectors and strings

v = [10, 20, 30, 40, 50]
println(v[1:3])     // [20, 30]
println(v[:2])      // [10, 20]
println(v[3:])      // [40, 50]
println(v[::-1])    // [50, 40, 30, 20, 10]

s = "hello world"
println(s[0:5])     // hello
println(s[::-1])    // dlrow olleh`,
    input: "",
  },
  {
    name: "F-Strings",
    code: `// F-strings — string interpolation with expressions

name = "Zen++"
version = 1
println(f"{name} v0.{version}.0")

a, b = 10, 3
println(f"{a} / {b} = {a / b} remainder {a % b}")

v = [1, 2, 3, 4, 5]
println(f"sum of {v} = {v[0] + v[1] + v[2] + v[3] + v[4]}")`,
    input: "",
  },
  {
    name: "Lambdas & Predicates",
    code: `// Lambda functions — anonymous functions as expressions

// sort by custom comparator
pairs = [[1, 30], [2, 10], [3, 20]]
sort(pairs, fn(a, b) { a[1] - b[1] })
println(pairs)   // [[2, 10], [3, 20], [1, 30]]

// filter with all/any
v = [2, 4, 6, 8, 10]
println(all(v, fn(x) { x % 2 == 0 }))   // 1
println(any(v, fn(x) { x > 9 }))         // 1`,
    input: "",
  },
  {
    name: "String Methods",
    code: `// String manipulation functions

// === upper / lower ===
println(upper("hello world"))    // HELLO WORLD
println(lower("HELLO WORLD"))    // hello world

// useful for case-insensitive comparison
a = "Hello"
b = "hello"
println(lower(a) == lower(b))    // 1

// === replace ===
s = "aababca"
println(replace(s, "a", "x"))    // xxbxbcx
println(replace(s, "ab", ""))    // aaca

// cleaning up input
messy = "1, 2, 3, 4"
clean = replace(messy, " ", "")
println(clean)                   // 1,2,3,4

// === startswith / endswith ===
filename = "solution.zpp"
println(startswith(filename, "solution"))  // 1
println(endswith(filename, ".zpp"))        // 1
println(endswith(filename, ".cpp"))        // 0

// === contains ===
line = "the quick brown fox"
println(contains(line, "brown"))  // 1
println(contains(line, "red"))    // 0

// === trim ===
raw = "   hello   "
println(trim(raw))               // hello
println(len(trim(raw)))          // 5

// === substr ===
s = "hello world"
println(substr(s, 6))            // world
println(substr(s, 0, 5))         // hello

// negative start index counts from end
println(substr(s, -5))           // world
println(substr(s, -5, 3))        // wor

// === slicing (also works on strings) ===
s = "abcdefgh"
println(s[0:3])                  // abc
println(s[3:])                   // defgh
println(s[:4])                   // abcd
println(s[::2])                  // aceg
println(s[::-1])                 // hgfedcba

// === combining them ===
// check if a string is a palindrome
fn isPalindrome(s) {
    s = lower(s)
    return s == s[::-1]
}
println(isPalindrome("racecar"))  // 1
println(isPalindrome("hello"))    // 0
println(isPalindrome("Madam"))    // 1

// iterate over characters in a string
fn countVowels(s) {
    s = lower(s)
    cnt = 0
    for c in s {
        if contains("aeiou", c) { cnt++ }
    }
    return cnt
}
println(countVowels("Hello World"))  // 3

// character frequency
freq = map()
for c in "abracadabra" {
    freq[c] = freq[c] + 1
}
println(freq)`,
    input: "",
  },
  {
    name: "Bitwise Operations",
    code: `// Bitwise operations

// === shift operators ===
println(1 << 0)     // 1
println(1 << 10)    // 1024
println(1 << 20)    // 1048576
println(1024 >> 3)  // 128
println(255 >> 4)   // 15

// === compound shift assignments ===
x = 1
x <<= 8
println(x)          // 256
x >>= 3
println(x)          // 32

// === common bitmask patterns ===

// check if i-th bit is set
fn getBit(n, i) {
    return (n >> i) & 1
}
println(getBit(13, 0))   // 1 (13 = 1101)
println(getBit(13, 1))   // 0
println(getBit(13, 2))   // 1
println(getBit(13, 3))   // 1

// set the i-th bit
fn setBit(n, i) {
    return n | (1 << i)
}
println(setBit(0, 3))    // 8

// clear the i-th bit (XOR trick since there's no bitwise NOT)
fn clearBit(n, i) {
    return n ^ (n & (1 << i))
}

// toggle the i-th bit
fn toggleBit(n, i) {
    return n ^ (1 << i)
}
println(toggleBit(13, 1))  // 15 (1101 -> 1111)
println(toggleBit(15, 2))  // 11 (1111 -> 1011)

// count set bits (popcount)
fn popcount(n) {
    cnt = 0
    while n > 0 {
        cnt += n & 1
        n >>= 1
    }
    return cnt
}
println(popcount(13))     // 3 (1101)
println(popcount(255))    // 8
println(popcount(1024))   // 1

// === bitmask subset enumeration ===
// enumerate all subsets of a 3-bit mask
mask = 7   // 111 in binary
sub = mask
while sub > 0 {
    print(sub, "")
    sub = (sub - 1) & mask
}
println()
// prints: 7 6 5 4 3 2 1

// === check power of 2 ===
fn isPow2(n) {
    return n > 0 && (n & (n - 1)) == 0
}
println(isPow2(16))   // 1
println(isPow2(15))   // 0
println(isPow2(1))    // 1

// === bitwise AND/OR/XOR (already existed, shown for completeness) ===
println(12 & 10)    // 8  (1100 & 1010 = 1000)
println(12 | 10)    // 14 (1100 | 1010 = 1110)
println(12 ^ 10)    // 6  (1100 ^ 1010 = 0110)`,
    input: "",
  },
  {
    name: "Bulk I/O & Casting",
    code: `// Bulk I/O and type casting demo
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
    name: "Fenwick Tree",
    code: `// Fenwick Tree — point update + range sum queries in O(log n)
import std

ft = FenwickTree()
ft.init(8)

vals = [3, 2, 5, 1, 4, 7, 6, 8]
for i len(vals) {
    ft.update(i, vals[i])
}

println(ft.query(3))          // prefix sum [0..3] = 11
println(ft.rangeQuery(2, 5))  // sum [2..5] = 17

ft.update(2, 3)               // add 3 to index 2
println(ft.rangeQuery(2, 5))  // now 20`,
    input: "",
  },
  {
    name: "Segment Tree",
    code: `// Segment Tree — range sum queries + point updates in O(log n)
import std

arr = [1, 3, 5, 7, 9, 11]
st = SegTree()
st.build(arr)

println(st.query(0, 5))   // sum of all = 36
println(st.query(1, 3))   // 3+5+7 = 15

st.update(2, 10)          // set index 2 to 10
println(st.query(1, 3))   // 3+10+7 = 20`,
    input: "",
  },
  {
    name: "Prefix Sums",
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
    name: "BFS",
    code: `// BFS traversal of an undirected graph
import std

n, m = read(), read()
adj = graph(n, m)

visited = fill(n, 0)
visited[0] = 1

q = Queue()
q.init()
q.push(0)

while q.size() > 0 {
    node = q.pop()
    println(node)
    for neighbor in adj[node] {
        if visited[neighbor] == 0 {
            visited[neighbor] = 1
            q.push(neighbor)
        }
    }
}`,
    input: "6 7\n0 1\n0 2\n1 3\n2 3\n3 4\n4 5\n2 5",
  },
  {
    name: "Dijkstra",
    code: `// Dijkstra's shortest path demo
import std

n, m = read(), read()
adj = wgraph(n, m)
dist = dijkstra(adj, 0)
println(dist)`,
    input: "4 5\n0 1 4\n0 2 1\n2 1 2\n1 3 1\n2 3 5",
  },
  {
    name: "CF 2200A — Eating Game",
    code: `// Codeforces 2200A - Eating Game
// Simulate circular eating for each starting player, count distinct winners
import std

fn solve() {
    n = read()
    a = read(n)

    winners = set()
    for s n {
        b = a[0:n]

        total = 0
        for x in b { total += x }

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
    code: `// Codeforces 1084B - Deletion Sort
// If already sorted, answer is n; otherwise 1

fn solve() {
    n = read()
    a = read(n)

    is_sorted = 1
    for i 1 n {
        if a[i] < a[i - 1] {
            is_sorted = 0
            break
        }
    }
    if is_sorted { println(n) } else { println(1) }
}

t = read()
while t-- { solve() }`,
    input: "3\n4\n1 4 2 3\n1\n100\n2\n6 7",
  },
  {
    name: "CF 2200C — Specialty String",
    code: `// Codeforces 1084C - Specialty String
// Stack-based: cancel adjacent equal chars, check if empty
import std

fn solve() {
    n = read()
    s = readLine()

    st = Stack()
    st.init()
    for c in s {
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
    code: `// Codeforces 1084D - Portal
// Split array, rotate inner segment so min is first, merge back

fn solve() {
    n = read()
    x = read()
    y = read()
    x--
    y--

    vals = read(n)
    a = []
    b = []
    for i n {
        if i <= x || i > y {
            push(a, vals[i])
        } else {
            push(b, vals[i])
        }
    }

    if len(b) > 0 {
        // find index of minimum in b
        mi = 0
        for i 1 len(b) {
            if b[i] < b[mi] { mi = i }
        }
        // rotate so min is first
        b = b[mi:len(b)] + b[0:mi]
    }

    m = -1
    if len(b) > 0 { m = b[0] }

    // find insertion point in a
    pos = len(a)
    for i len(a) {
        if a[i] >= m {
            pos = i
            break
        }
    }

    // build result: a[:pos] + b + a[pos:]
    res = a[0:pos] + b + a[pos:len(a)]
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
    code: `// Codeforces 1084E - Divisive Battle
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
    a = read(n)

    b = []
    for x in a { push(b, primebase(x)) }

    if is_sorted(a) {
        println("Bob")
    } else {
        has_multi = any(b, fn(x) { x == -1 })
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
];

// Populate dropdown with groups
const groups = [
  { label: "Basics", items: [0, 1, 2] },
  { label: "Language Features", items: [3, 4, 5, 6, 7, 8, 9, 10] },
  { label: "Data Structures", items: [11, 12, 13, 14] },
  { label: "Graph Algorithms", items: [15, 16] },
  { label: "Codeforces 2200", items: [17, 18, 19, 20, 21] },
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
  // Update the default option text to show what's loaded
  samplesEl.querySelector('option[value=""]').textContent = sample.name;
  samplesEl.value = "";
});

// ── Theme switcher ──────────────────────────────

const THEMES = {
  "Tokyo Night": {
    bg: "#1a1b26", surface: "#1f2335", "surface-2": "#24283b",
    text: "#a9b1d6", muted: "#565f89", accent: "#7aa2f7", border: "#292e42",
    "tok-keyword": "#bb9af7", "tok-builtin": "#7aa2f7", "tok-string": "#9ece6a",
    "tok-number": "#ff9e64", "tok-comment": "#565f89", "tok-operator": "#89ddff",
    swatch: "#7aa2f7",
  },
  "Dracula": {
    bg: "#282a36", surface: "#2d2f3f", "surface-2": "#343746",
    text: "#f8f8f2", muted: "#6272a4", accent: "#bd93f9", border: "#44475a",
    "tok-keyword": "#ff79c6", "tok-builtin": "#8be9fd", "tok-string": "#f1fa8c",
    "tok-number": "#bd93f9", "tok-comment": "#6272a4", "tok-operator": "#ff79c6",
    swatch: "#bd93f9",
  },
  "Catppuccin Mocha": {
    bg: "#1e1e2e", surface: "#232334", "surface-2": "#2a2a3c",
    text: "#cdd6f4", muted: "#6c7086", accent: "#89b4fa", border: "#313244",
    "tok-keyword": "#cba6f7", "tok-builtin": "#89b4fa", "tok-string": "#a6e3a1",
    "tok-number": "#fab387", "tok-comment": "#6c7086", "tok-operator": "#89dceb",
    swatch: "#cba6f7",
  },
  "GitHub Dark": {
    bg: "#0d1117", surface: "#161b22", "surface-2": "#1c2129",
    text: "#c9d1d9", muted: "#484f58", accent: "#58a6ff", border: "#30363d",
    "tok-keyword": "#ff7b72", "tok-builtin": "#79c0ff", "tok-string": "#a5d6ff",
    "tok-number": "#79c0ff", "tok-comment": "#484f58", "tok-operator": "#ff7b72",
    swatch: "#161b22",
  },
  "Gruvbox": {
    bg: "#1d2021", surface: "#282828", "surface-2": "#32302f",
    text: "#ebdbb2", muted: "#665c54", accent: "#fabd2f", border: "#3c3836",
    "tok-keyword": "#fb4934", "tok-builtin": "#83a598", "tok-string": "#b8bb26",
    "tok-number": "#d3869b", "tok-comment": "#665c54", "tok-operator": "#fe8019",
    swatch: "#fabd2f",
  },
  "Nord": {
    bg: "#2e3440", surface: "#333a47", "surface-2": "#3b4252",
    text: "#d8dee9", muted: "#616e88", accent: "#88c0d0", border: "#434c5e",
    "tok-keyword": "#81a1c1", "tok-builtin": "#88c0d0", "tok-string": "#a3be8c",
    "tok-number": "#b48ead", "tok-comment": "#616e88", "tok-operator": "#81a1c1",
    swatch: "#88c0d0",
  },
};

function applyTheme(name) {
  const t = THEMES[name];
  if (!t) return;
  const root = document.documentElement;
  for (const [key, val] of Object.entries(t)) {
    if (key === "swatch") continue;
    root.style.setProperty("--" + key, val);
  }
  localStorage.setItem("zenpp-theme", name);
  // Update active state
  document.querySelectorAll(".theme-option").forEach((btn) => {
    btn.classList.toggle("active", btn.dataset.theme === name);
  });
}

// Build theme list
const themeListEl = document.getElementById("theme-list");
for (const [name, t] of Object.entries(THEMES)) {
  const btn = document.createElement("button");
  btn.className = "theme-option";
  btn.dataset.theme = name;
  btn.innerHTML = `<span class="theme-swatch" style="background:${t.swatch}"></span>${name}`;
  btn.addEventListener("click", () => applyTheme(name));
  themeListEl.appendChild(btn);
}

// Settings toggle
const settingsBtn = document.getElementById("settings-btn");
const settingsPopup = document.getElementById("settings-popup");
settingsBtn.addEventListener("click", (e) => {
  e.stopPropagation();
  settingsPopup.classList.toggle("visible");
});
document.addEventListener("click", (e) => {
  if (!settingsPopup.contains(e.target)) {
    settingsPopup.classList.remove("visible");
  }
});

// Restore saved theme
const savedTheme = localStorage.getItem("zenpp-theme") || "Tokyo Night";
applyTheme(savedTheme);

renderHighlight();
