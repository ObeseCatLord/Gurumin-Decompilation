#!/bin/sh
# Runs every test that needs no game files: the Python unit tests, the portable
# C++ tests (host g++) and the two Windows-only tests (i686 MinGW, run under
# Wine when it is installed; set GURUMIN_SKIP_WINE=1 to only compile them).
# Run ./build.sh first: native_render_test links the MinHook objects in dist/.
# The Wine tests create a D3D9 window: use xvfb-run -a when there is no display.
set -eu
cd "$(dirname "$0")/.."
out=build/tests
mkdir -p "$out"
python3 -m unittest discover -s tests -p '*_test.py'
for name in affine rhythm settings vr_math shadows; do
  ${CXX:-g++} -std=c++17 -O2 -Ivendor/minhook/include "tests/${name}_test.cpp" -o "$out/$name"
  "$out/$name" >/dev/null
  echo "ok $name"
done
i686-w64-mingw32-g++ -std=c++17 -O2 -static -Ivendor/minhook/include tests/antialias_test.cpp \
  -o "$out/antialias_test.exe" -ld3d9
i686-w64-mingw32-g++ -std=c++17 -O2 -static -Ivendor/minhook/include tests/native_render_test.cpp \
  dist/buffer.o dist/hook.o dist/trampoline.o dist/hde32.o -o "$out/native_render_test.exe" -ladvapi32
if [ "${GURUMIN_SKIP_WINE-}" = 1 ] || ! command -v wine >/dev/null 2>&1; then
  echo "skipped antialias_test.exe and native_render_test.exe (no Wine)"
  exit 0
fi
export WINEDEBUG=${WINEDEBUG:--all}
for name in antialias_test native_render_test; do
  wine "$out/$name.exe" >/dev/null
  echo "ok $name (Wine)"
done
