const path = require("path");

async function main() {
  let Zenpp;
  try {
    Zenpp = require(path.join(__dirname, "..", "web", "zenpp.js"));
  } catch (err) {
    console.error("FAIL: web/zenpp.js missing. Run `make build-web`.");
    process.exit(1);
  }

  const module = await Zenpp({
    locateFile: (p) => path.join(__dirname, "..", "web", p),
  });

  if (!module._zenpp_eval) {
    console.error("FAIL: zenpp_eval export missing. Rebuild wasm.");
    process.exit(1);
  }

  const evalFn = module.cwrap("zenpp_eval", "number", ["string", "string"]);
  const ptr = evalFn("print(1+3)\n", "");
  if (!ptr) {
    console.error("FAIL: zenpp_eval returned null pointer");
    process.exit(1);
  }
  const text = module.UTF8ToString(ptr);
  module._free(ptr);

  if (text.trim() !== "4") {
    console.error(`FAIL: expected 4, got '${text.trim()}'`);
    process.exit(1);
  }

  console.log("PASS: web runtime test");
}

main().catch((err) => {
  console.error("FAIL:", err);
  process.exit(1);
});
