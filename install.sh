#!/usr/bin/env bash

set -e

echo "🚀 Installing Linux Mini Shell..."

# Detect OS
OS="$(uname -s)"
echo "✓ Detected OS: $OS"

# Check for compiler
if command -v gcc >/dev/null 2>&1; then
    COMPILER="gcc"
elif command -v clang >/dev/null 2>&1; then
    COMPILER="clang"
else
    echo "❌ Error: Neither gcc nor clang found. Please install a C compiler."
    exit 1
fi
echo "✓ Found compiler: $COMPILER"

# Check for make
if ! command -v make >/dev/null 2>&1; then
    echo "❌ Error: make is not installed. Please install make."
    exit 1
fi
echo "✓ Found make"

# Clone if not in repo
if [ ! -f "Makefile" ] || [ ! -d "src" ]; then
    echo "⬇️  Cloning repository..."
    TMP_DIR=$(mktemp -d)
    git clone https://github.com/HarshEvolves/linux-mini-shell.git "$TMP_DIR"
    cd "$TMP_DIR"
fi

echo "🔨 Building..."
make clean >/dev/null 2>&1 || true
make CC="$COMPILER"

echo "📦 Installing globally (may require sudo)..."
if [ "$EUID" -ne 0 ]; then
    if command -v sudo >/dev/null 2>&1; then
        sudo make install
    else
        echo "⚠️  sudo not found. Attempting to install without sudo..."
        make install
    fi
else
    make install
fi

echo "✅ Installed successfully!"
echo "🎉 Run 'minishell' to start."
