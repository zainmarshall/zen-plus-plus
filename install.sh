#!/bin/bash
set -e

PREFIX="${PREFIX:-/usr/local}"
BIN_DIR="$PREFIX/bin"
LIB_DIR="$PREFIX/lib/zenpp"

echo "Building Zen++..."
make build

echo "Installing Zen++ to $PREFIX..."
sudo mkdir -p "$BIN_DIR" "$LIB_DIR/stdlib"
sudo cp build/zenpp "$BIN_DIR/zenpp"
sudo cp stdlib/manifest.txt "$LIB_DIR/stdlib/"
sudo cp stdlib/*.zpp "$LIB_DIR/stdlib/"

echo ""
echo "Zen++ installed successfully!"
echo "  Binary:  $BIN_DIR/zenpp"
echo "  Stdlib:  $LIB_DIR/stdlib/"
echo ""
echo "Run 'zenpp' for the REPL or 'zenpp file.zpp' to execute a file."
