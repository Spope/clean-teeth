#!/usr/bin/env bash
# Export each theme (prototype/themes/<id>.js) for the device: its artwork, rendered to PNGs
# in assets/png with headless Chrome, and its settings (colours, sounds, tricks...) to
# assets/themes/<id>.json. The device build turns both into C (tools/png2lvgl.py).
#
# Usage: tools/export_assets.sh [theme ...]   (default: every theme)
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
PNG_DIR="$ROOT/assets/png"
THEME_DIR="$ROOT/assets/themes"
CHROME="${CHROME:-/Applications/Google Chrome.app/Contents/MacOS/Google Chrome}"
PROFILE="$(mktemp -d)"
trap 'rm -rf "$PROFILE"' EXIT

WINDOW=600 # headless Chrome has a minimum window size, so render big and crop the centre

if [ $# -gt 0 ]; then
  THEMES=("$@")
else
  THEMES=()
  for f in "$ROOT"/prototype/themes/*.js; do
    case "$(basename "$f")" in
      registry.js | _*) ;; # not themes (_template.js is the starting point for new ones)
      *) THEMES+=("$(basename "$f" .js)") ;;
    esac
  done
fi

mkdir -p "$PNG_DIR" "$THEME_DIR"

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
  settings="$(node "$ROOT/tools/theme_assets.js" "$t" json)" # stops here if the theme has problems
  printf '%s\n' "$settings" > "$THEME_DIR/$t.json"
  echo "wrote assets/themes/$t.json"
  rm -f "$PNG_DIR/${t}_"*.png
  # Poses and sprites are listed by the theme itself
  while read -r kind name w h; do
    if [ "$kind" = pose ]; then
      shot 250 250 "${t}_${name}" "theme=$t&what=pose&pose=$name"
    else
      shot "$w" "$h" "${t}_${name}" "theme=$t&what=sprite&name=$name"
    fi
  done < <(node "$ROOT/tools/theme_assets.js" "$t")
  # Peek-a-boo: the poses it can show, at its zoom (BRUSHY_PEEK_ZOOM, 1.9 x 250 px);
  # png2lvgl.py tilts them and keeps the part that reaches the screen
  for pose in idle blink brush_a brush_b; do
    shot 475 475 "${t}_${pose}_peek" "theme=$t&what=pose&pose=$pose"
  done
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
