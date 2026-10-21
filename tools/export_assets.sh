#!/usr/bin/env bash
# Render each theme's artwork (prototype/themes/<id>.js) to PNGs in assets/png
# with headless Chrome. The device build converts these PNGs to LVGL C images
# (tools/png2lvgl.py).
#
# Usage: tools/export_assets.sh [theme ...]   (default: every theme)
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
PNG_DIR="$ROOT/assets/png"
CHROME="${CHROME:-/Applications/Google Chrome.app/Contents/MacOS/Google Chrome}"
PROFILE="$(mktemp -d)"
trap 'rm -rf "$PROFILE"' EXIT

WINDOW=600 # headless Chrome has a minimum window size, so render big and crop the centre

if [ $# -gt 0 ]; then
  THEMES=("$@")
else
  THEMES=()
  for f in "$ROOT"/prototype/themes/*.js; do
    [ "$(basename "$f")" = registry.js ] || THEMES+=("$(basename "$f" .js)")
  done
fi

mkdir -p "$PNG_DIR"

shot() { # width height output-name query
  local out="$PNG_DIR/$3.png"
  # Virtual time lets web fonts load before the capture (animations are disabled anyway).
  "$CHROME" --headless=new --disable-gpu --hide-scrollbars --force-device-scale-factor=1 \
    --default-background-color=00000000 --user-data-dir="$PROFILE" --virtual-time-budget=5000 \
    --window-size="$WINDOW,$WINDOW" --screenshot="$out" \
    "file://$ROOT/tools/render.html?$4&w=$1&h=$2" 2>/dev/null
  sips -c "$2" "$1" "$out" >/dev/null
  echo "rendered $3.png (${1}x$2)"
}

for t in "${THEMES[@]}"; do
  rm -f "$PNG_DIR/${t}_"*.png
  # Poses and sprites are listed by the theme itself
  while read -r kind name w h; do
    if [ "$kind" = pose ]; then
      shot 250 250 "${t}_${name}" "theme=$t&what=pose&pose=$name"
    else
      shot "$w" "$h" "${t}_${name}" "theme=$t&what=sprite&name=$name"
    fi
  done < <(node "$ROOT/tools/theme_assets.js" "$t")
  for i in 0 1 2 3; do
    shot 52 52 "${t}_germ_$i" "theme=$t&what=germ&i=$i"
    shot 52 52 "${t}_germ_scared_$i" "theme=$t&what=germ&i=$i&scared=1"
  done
  shot 32 32 "${t}_reward" "theme=$t&what=reward"
  shot 300 64 "${t}_bravo" "theme=$t&what=bravo"
  for d in 0 1 2 3 4 5 6 7 8 9 colon; do # big countdown digits
    shot 100 110 "${t}_clock_digit_$d" "theme=$t&what=clock&d=$d"
  done
done
