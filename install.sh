#!/bin/bash
set -e

REPO_URL="https://github.com/zainmarshall/zen-plus-plus.git"
REPO_DIR="zen-plus-plus"
PREFIX="${PREFIX:-/usr/local}"
BIN_DIR="$PREFIX/bin"
LIB_DIR="$PREFIX/lib/zenpp"

# If run from outside the repo (e.g. curl | bash), clone or pull
if [ ! -f "Makefile" ] || [ ! -d "src" ]; then
    if [ -d "$REPO_DIR" ]; then
        echo "Updating existing Zen++ repo..."
        cd "$REPO_DIR"
        git pull
    else
        echo "Cloning Zen++..."
        git clone "$REPO_URL"
        cd "$REPO_DIR"
    fi
fi

echo "Building Zen++..."
rm -rf build
make build

echo "Installing Zen++ to $PREFIX..."
sudo mkdir -p "$BIN_DIR" "$LIB_DIR/stdlib"
sudo rm -f "$BIN_DIR/zenpp"
sudo cp build/zenpp "$BIN_DIR/zenpp"
sudo cp stdlib/manifest.txt "$LIB_DIR/stdlib/"
sudo cp stdlib/*.zpp "$LIB_DIR/stdlib/"

echo ""
echo "Zen++ installed successfully!"
echo "  Binary:  $BIN_DIR/zenpp"
echo "  Stdlib:  $LIB_DIR/stdlib/"
echo ""
echo "Run 'zenpp' for the REPL or 'zenpp file.zpp' to execute a file."
