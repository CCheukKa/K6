#!/bin/bash
# K6-IME-Setup.sh
# Setup script for K6 IME on Unix-like systems (macOS, Linux with Wine)
# Note: K6 is designed for Windows. This script is for cross-platform build/dev.

set -e

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$PROJECT_ROOT/build"
INSTALL_MODE="${1:-dev}"

echo ""
echo "========================================"
echo "K6 Stroke IME Setup"
echo "========================================"
echo ""

# Check for CMake
if ! command -v cmake &> /dev/null; then
    echo "ERROR: CMake not found. Please install CMake 3.20 or later."
    exit 1
fi
echo "✓ CMake found"

# Check for C++ compiler
if ! command -v clang++ &> /dev/null && ! command -v g++ &> /dev/null; then
    echo "ERROR: C++ compiler not found. Please install clang or gcc."
    exit 1
fi
echo "✓ C++ compiler found"

echo ""
echo "Step 1: Cleaning previous build..."
if [ -d "$BUILD_DIR" ]; then
    rm -rf "$BUILD_DIR"
fi

echo ""
echo "Step 2: Configuring CMake..."
cd "$PROJECT_ROOT"

if [[ "$OSTYPE" == "darwin"* ]]; then
    # macOS
    cmake -S . -B build -G "Xcode"
elif [[ "$OSTYPE" == "linux-gnu"* ]]; then
    # Linux
    cmake -S . -B build -G "Unix Makefiles"
else
    # Fallback
    cmake -S . -B build
fi

echo "✓ CMake configured"

echo ""
echo "Step 3: Building K6 IME..."
cmake --build build --config Release --parallel
echo "✓ Build successful"

DLL_PATH="$BUILD_DIR/Release/K6.dll"
# On Unix, it might be .so or .dylib
if [ ! -f "$DLL_PATH" ]; then
    if [ -f "$BUILD_DIR/Release/libK6.so" ]; then
        DLL_PATH="$BUILD_DIR/Release/libK6.so"
    elif [ -f "$BUILD_DIR/Release/libK6.dylib" ]; then
        DLL_PATH="$BUILD_DIR/Release/libK6.dylib"
    else
        echo "ERROR: Library not found"
        exit 1
    fi
fi

echo ""
echo "========================================"
echo "Development Setup Complete!"
echo "========================================"
echo ""
echo "Library location: $DLL_PATH"
echo ""
echo "NOTE: K6 is a Windows IME (Text Input Processor for Windows TSF)"
echo "      On Unix systems, this is for development and testing only."
echo ""
echo "For Windows deployment, see INSTALL.md"
echo ""
