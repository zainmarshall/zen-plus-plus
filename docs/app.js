const sourceEl = document.getElementById("source");
const highlightEl = document.getElementById("source-highlight");
const inputEl = document.getElementById("stdin");
const outputEl = document.getElementById("stdout");
const runBtn = document.getElementById("run");
const clearBtn = document.getElementById("clear");
const acEl = document.getElementById("autocomplete");

const defaultProgram = `fn solve() {
    n = read()
    if n % 2 == 0 {
        print(n ** 2)
    } else {
        print(n)
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
    /\/\/.*|\/\*[\s\S]*?\*\/|"(?:\\.|[^"\\])*"|\b(?:fn|if|else|while|for|return|true|false|struct|import|map|set|self)\b|\b(?:read|readInt|readFloat|readLine|len|push|pop|print|min|max|abs|gcd)\b|\b\d+(?:\.\d+)?\b|\*\*|==|!=|<=|>=|&&|\|\||[+*/%<>=^|&!\-]/g;

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
    if (/^(fn|if|else|while|for|return|true|false|struct|import|map|set|self)$/.test(match)) {
      return `<span class="token-keyword">${match}</span>`;
    }
    if (/^(read|readInt|readFloat|readLine|len|push|pop|print|min|max|abs|gcd)$/.test(match)) {
      return `<span class="token-builtin">${match}</span>`;
    }
    return `<span class="token-operator">${match}</span>`;
  });
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
  // Built-in functions
  { text: "print()", kind: "builtin", cursor: -1 },
  { text: "read()", kind: "builtin" },
  { text: "readInt()", kind: "builtin" },
  { text: "readFloat()", kind: "builtin" },
  { text: "readLine()", kind: "builtin" },
  { text: "len()", kind: "builtin", cursor: -1 },
  { text: "push()", kind: "builtin", cursor: -1 },
  { text: "pop()", kind: "builtin", cursor: -1 },
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
});

renderHighlight();
