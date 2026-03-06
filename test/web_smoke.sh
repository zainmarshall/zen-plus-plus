#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT_DIR"

if [[ ! -f web/index.html ]]; then
  echo "FAIL: web/index.html missing"
  exit 1
fi
if [[ ! -f web/app.js ]]; then
  echo "FAIL: web/app.js missing"
  exit 1
fi
if [[ ! -f web/style.css ]]; then
  echo "FAIL: web/style.css missing"
  exit 1
fi

grep -q "zenpp.js" web/index.html || { echo "FAIL: zenpp.js not referenced"; exit 1; }
grep -q "app.js" web/index.html || { echo "FAIL: app.js not referenced"; exit 1; }
grep -q "source-highlight" web/index.html || { echo "FAIL: syntax highlight layer missing"; exit 1; }

grep -q "zenpp_eval" web/app.js || { echo "FAIL: zenpp_eval not referenced"; exit 1; }

grep -q "token-keyword" web/style.css || { echo "FAIL: highlight CSS missing"; exit 1; }

echo "PASS: web smoke test"
