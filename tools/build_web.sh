#!/usr/bin/env bash
# Build the simulator for web pages: the device UI compiled to WebAssembly with Emscripten
# (brew install emscripten), with every exported theme, in device/sim/build-web.
# With OUT_DIR, also copy the files a website needs there: index.html, device.css,
# brushy_sim.js and brushy_sim.wasm (see README, "On a web page").
#
# Usage: tools/build_web.sh [OUT_DIR]     e.g. tools/build_web.sh ~/blog/static/brushy-sim
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
BUILD="$ROOT/device/sim/build-web"
FILES=(index.html device.css brushy_sim.js brushy_sim.wasm)

if ! command -v emcmake >/dev/null; then
  echo "Emscripten is needed: brew install emscripten" >&2
  exit 1
fi
if [ ! -f "$BUILD/CMakeCache.txt" ]; then
  emcmake cmake -S "$ROOT/device/sim" -B "$BUILD" -G Ninja -DCMAKE_BUILD_TYPE=Release
fi
cmake --build "$BUILD"

if [ $# -gt 0 ]; then
  mkdir -p "$1"
  for f in "${FILES[@]}"; do cp "$BUILD/$f" "$1/"; done
  echo "copied ${FILES[*]} to $1"
else
  echo "built in $BUILD (try it: python3 -m http.server -d $BUILD, then open http://localhost:8000)"
fi
