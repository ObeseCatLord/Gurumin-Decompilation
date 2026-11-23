#!/bin/sh
# Builds mod/build/d3d9.dll (32-bit) with zig (pip install ziglang).
set -e
cd "$(dirname "$0")"
mkdir -p build
export ZIG_GLOBAL_CACHE_DIR="${ZIG_GLOBAL_CACHE_DIR:-$PWD/build/.zig-global}"
export ZIG_LOCAL_CACHE_DIR="${ZIG_LOCAL_CACHE_DIR:-$PWD/build/.zig-local}"
PY=python3
command -v python3 >/dev/null 2>&1 && python3 -c "" 2>/dev/null || PY=python
$PY -m ziglang cc -target x86-windows-gnu -shared -O2 -Wall -Wno-unused-function \
    -o build/d3d9.dll src/proxy.c src/display.c src/movies.c src/d3d9.def -luser32 -lkernel32
rm -f build/d3d9.lib build/d3d9.pdb build/proxy.lib
echo "built build/d3d9.dll"
