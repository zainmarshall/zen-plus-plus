const sourceEl = document.getElementById("source");
const highlightEl = document.getElementById("source-highlight");
const inputEl = document.getElementById("stdin");
const outputEl = document.getElementById("stdout");
const runBtn = document.getElementById("run");
const clearBtn = document.getElementById("clear");

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

sourceEl.value = defaultProgram;
inputEl.value = defaultInput;

let zenppModulePromise = null;
let abortSeen = false;

function escapeHtml(text) {
  return text
    .replace(/&/g, "&amp;")
    .replace(/</g, "&lt;")
    .replace(/>/g, "&gt;");
}

function highlight(text) {
  const escaped = escapeHtml(text);
  const tokenRegex =
    /\/\/.*|\/\*[\s\S]*?\*\/|"(?:\\.|[^"\\])*"|\\b(?:fn|if|else|while|for|return|true|false|struct|import|map|set)\\b|\\b(?:read|readInt|readFloat|readLine|len|push|pop)\\b|\\b\\d+(?:\\.\\d+)?\\b|\\*\\*|==|!=|<=|>=|&&|\\|\\||[+*/%<>=^|&!-]/g;

  return escaped.replace(tokenRegex, (match) => {
    if (match.startsWith("//") || match.startsWith("/*")) {
      return `<span class="token-comment">${match}</span>`;
    }
    if (match.startsWith("\"")) {
      return `<span class="token-string">${match}</span>`;
    }
    if (/^\\d/.test(match)) {
      return `<span class="token-number">${match}</span>`;
    }
    if (/^(fn|if|else|while|for|return|true|false|struct|import|map|set)$/.test(match)) {
      return `<span class="token-keyword">${match}</span>`;
    }
    if (/^(read|readInt|readFloat|readLine|len|push|pop)$/.test(match)) {
      return `<span class="token-keyword">${match}</span>`;
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
    setOutput(`Error: ${err.message || err}`);
  }
});

clearBtn.addEventListener("click", () => {
  inputEl.value = "";
});

sourceEl.addEventListener("input", renderHighlight);
sourceEl.addEventListener("scroll", renderHighlight);

renderHighlight();
