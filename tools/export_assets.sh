#!/usr/bin/env bash
# Render the prototype's SVG artwork to PNGs (assets/png) with headless Chrome.
# The device build converts these PNGs to LVGL C images (tools/png2lvgl.py).
#
# Usage: tools/export_assets.sh [character ...]   (default: dino)
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
PNG_DIR="$ROOT/assets/png"
CHROME="${CHROME:-/Applications/Google Chrome.app/Contents/MacOS/Google Chrome}"
PROFILE="$(mktemp -d)"
trap 'rm -rf "$PROFILE"' EXIT

CHARACTER_SIZE=250 # px on the 466x466 screen
GERM_SIZE=52
STAR_SIZE=30
POSES=(idle blink sleepy brush_a brush_b cheer)
WINDOW=600 # headless Chrome has a minimum window size, so render big and crop the centre

mkdir -p "$PNG_DIR"

shot() { # size output-name query
  local out="$PNG_DIR/$2.png"
  "$CHROME" --headless=new --disable-gpu --hide-scrollbars --force-device-scale-factor=1 \
    --default-background-color=00000000 --user-data-dir="$PROFILE" \
    --window-size="$WINDOW,$WINDOW" --screenshot="$out" \
    "file://$ROOT/tools/render.html?$3&size=$1" 2>/dev/null
  sips -c "$1" "$1" "$out" >/dev/null
  echo "rendered $2.png ($(sips -g pixelWidth -g pixelHeight "$out" | awk '/pixel/{printf "%s ", $2}'))"
}

for c in "${@:-dino}"; do
  for p in "${POSES[@]}"; do
    shot "$CHARACTER_SIZE" "${c}_${p}" "what=pose&char=$c&pose=$p"
  done
done
for i in 0 1 2 3; do
  shot "$GERM_SIZE" "germ_$i" "what=germ&i=$i"
done
shot "$STAR_SIZE" "star" "what=star"

