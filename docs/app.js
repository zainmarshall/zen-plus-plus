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
  const escaped = escapeHtml(text);
  const tokenRegex =
    /\/\/.*|\/\*[\s\S]*?\*\/|"(?:\\.|[^"\\])*"|\b(?:fn|if|else|while|for|return|true|false|struct|import|map|set|self|break|continue|in)\b|\b(?:read|readInt|readFloat|readLine|len|push|pop|print|println|min|max|abs|gcd|ord|chr|parseInt)\b|\b\d+(?:\.\d+)?\b|\*\*|==|!=|<=|>=|&&|\|\||[+*/%<>=^|&!\-]/g;

  return escaped.replace(tokenRegex, (match) => {
    if (match.startsWith("//") || match.startsWith("/*")) {
      return `<span class="token-comment">${match}</span>`;
    }
    if (match.startsWith("\"")) {
      return `<span class="token-string">${match}</span>`;
    }
    if (/^\d/.test(match)) {
      return `<span class="token-number">${match}</span>`;
    }
    if (/^(fn|if|else|while|for|return|true|false|struct|import|map|set|self|break|continue|in)$/.test(match)) {
      return `<span class="token-keyword">${match}</span>`;
    }
    if (/^(read|readInt|readFloat|readLine|len|push|pop|print|println|min|max|abs|gcd|ord|chr|parseInt)$/.test(match)) {
      return `<span class="token-builtin">${match}</span>`;
    }
    return `<span class="token-operator">${match}</span>`;
  });
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
  // Stdlib math (import std)
  { text: "min()", kind: "stdlib", cursor: -1 },
  { text: "max()", kind: "stdlib", cursor: -1 },
  { text: "abs()", kind: "stdlib", cursor: -1 },
  { text: "gcd()", kind: "stdlib", cursor: -1 },
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
    input: `5\n5\n10\n1\n0\n20`,
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
    input: `3\n4\n1 4 2 3\n1\n100\n2\n6 7`,
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
    input: `4\n10\n10 9 8 7 6 5 4 3 2 1\n3\n1 8192 677\n2\n6 5\n2\n6 7`,
  },
];

// Populate dropdown
SAMPLES.forEach((s, i) => {
  const opt = document.createElement("option");
  opt.value = i;
  opt.textContent = s.name;
  samplesEl.appendChild(opt);
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
