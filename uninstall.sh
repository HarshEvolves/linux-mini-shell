#!/usr/bin/env bash

set -e

echo "🗑️  Uninstalling Linux Mini Shell..."

if [ ! -f "Makefile" ]; then
    echo "❌ Error: Makefile not found. Please run this from the project root."
    exit 1
fi

echo "📦 Removing globally (may require sudo)..."
if [ "$EUID" -ne 0 ]; then
    if command -v sudo >/dev/null 2>&1; then
        sudo make uninstall
    else
        echo "⚠️  sudo not found. Attempting to uninstall without sudo..."
        make uninstall
    fi
else
    make uninstall
fi

echo "✅ Uninstalled successfully!"
