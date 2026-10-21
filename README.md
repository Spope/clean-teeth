# Brushy Buddy

A cute 2-minute tooth-brushing timer for a small round screen
([Waveshare ESP32-S3 Touch AMOLED 1.75"](https://www.waveshare.com/esp32-s3-touch-amoled-1.75.htm), 466×466).

Two themes, switched by **pressing and holding** the start screen:

- **Minecraft**: Steve, mob heads as germs (creeper, zombie, slime, spider),
  diamonds as rewards, a blocky XP-bar ring, chiptune sounds.
- **Les Royaumes de feu**: Glory the RainWing, whose scales change colour with her mood
  (teal → lavender when sleepy → pink when happy), jungle bugs as germs (mosquito, beetle,
  caterpillar, spider), gems as rewards, venom "fwoosh" pops.

```
prototype/     Phase 1: browser prototype, and the source of all artwork (SVG)
assets/png/    artwork exported from the prototype for the device
device/ui/     Phase 2: device UI in C + LVGL (platform-independent)
device/sim/    Phase 2: desktop simulator (SDL window)
tools/         asset export + PNG → LVGL converter
```

## Phase 1 — browser prototype

No build step. Open `prototype/index.html` in a browser (double-click works), or serve it:

```sh
python3 -m http.server 8123   # then open http://localhost:8123/prototype/
```

To try it on a phone/tablet on the same Wi-Fi, open `http://<your-mac-ip>:8123/prototype/`.

### How it works

- **Tap** the screen to start; **press and hold** (about 1 s) to switch theme. Brushing lasts 2 minutes, split into 4 zones of 30 s,
  shown clockwise on the ring: top-left → top-right → bottom-right → bottom-left.
- A big, faint **countdown** (in the theme's font and the current zone's colour) sits
  behind everything above the buddy's head while brushing.
- Germs around the buddy pop as time passes; a chime plays at each zone change.
- Tapping during brushing makes the buddy giggle.
- At the end: confetti, "Bravo !", and +1 reward (stored in the browser).
- After 30 s without a tap, the buddy falls asleep.
- Every 8-14 s while awake, the buddy plays a trick:
  - on the start screen and while brushing: **peek-a-boo** (ducks out, pops up big and close
    from the bottom edge, comes back), **peek from the side** (leans in from an edge to spy,
    twice), **slide** (walks off one side, hops across the whole screen, walks back in);
  - on the start screen only (they put the toothbrush away): Minecraft **mining** (a grass
    block appears, Steve breaks it in three pickaxe hits); Glory's **rainbow** scales and
    **flying** across the top of the screen.
- Germs are defeated the theme's way: Minecraft mobs burst into pixels (the creeper swells
  and flashes white first, with a fuse hiss); Glory spits **venom** at each bug
  (RainWings don't breathe fire), and it melts.
  The screen **shakes** and the remaining germs **shiver with scared faces**.
- At the end, the **reward drops** out of the buddy, bounces (in pixel steps for Minecraft)
  and flies to the counter, which goes up when it lands.

The panel under the device is for development only: speed (1×/10×/30×), theme picker,
tricks (play one now; those the theme doesn't have are greyed out), sound, skip-to-end, reset.

Preview a state directly with a URL parameter:

```
index.html?theme=dragon&preview=idle | sleepy | brushing&t=50 | done
          &trick=peek | peekSide | slide | mine | rainbow | fly | mirror | shake
```

### Prototype layout

```
prototype/
  index.html            simulated device + dev panel
  css/style.css         device, ring, germs, overlays
  js/app.js             timer, phases, ring, germs, input
  js/pixel.js           pixel-art helper (grid of letters -> SVG)
  js/sound.js           tiny WebAudio synth (no audio files)
  themes/
    registry.js         theme contract (read this first)
    minecraft.js        Steve, mobs, diamond
    dragon.js           Glory, germs, gem
```

### Adding or changing a theme

A theme bundles the buddy, germs, reward icon, colours, fonts and sound flavour
(see `prototype/themes/registry.js`). To add one, copy `dragon.js` (vector art) or
`minecraft.js` (pixel art), change it, and add a `<script>` tag in `prototype/index.html`.
It joins the theme picker and the long-press cycle. To bring it to the device, see
"Changing the artwork" below.

A buddy reacts to four mood classes set by the app: `mood-idle`, `mood-sleepy`,
`mood-brushing`, `mood-cheer`. The tap "giggle" is handled generically.
Glory's mood colours are CSS variables at the top of her `css` (`--scale`, `--frill`...).

### Design notes for the real screen

- AMOLED: black pixels are off — keep the background pure black.
- The real 1.75" panel is about 4.4 cm across; the browser view is much larger,
  so check that shapes stay readable when scaled down (e.g. zoom out).
- Keep touch interaction to one big tap.

## Phase 2 — LVGL simulator (current)

The device UI (`device/ui/`) is written in C with [LVGL](https://lvgl.io) v9.5 and runs
in a 466×466 desktop window. The same code will run on the ESP32-S3; only the
platform layer (`brushy_hal.h`: sound + saved settings, plus display/touch drivers) changes.

### Build and run (macOS)

```sh
brew install cmake ninja sdl2          # once
cmake -S device/sim -B device/sim/build -G Ninja
cmake --build device/sim/build
device/sim/build/brushy_sim
```

The first configure downloads LVGL (pinned to v9.5.0). Click the window to tap;
hold the mouse button down to switch theme.

Options:

```
--theme dragon             minecraft | dragon (default: last used)
--speed 10                 run time 10x faster (testing)
--preview brushing:50      start in a state: idle | sleepy | brushing[:seconds] | done
--trick peek               play an animation right away: peek | peek_side | slide |
                           mine | rainbow | fly | mirror | shake
--mute                     no sound
--zoom 0.5                 smaller window, closer to the real 4.4 cm panel size
--snapshot out.bmp --after 1500   save a screenshot after 1.5 s and quit
```

Rewards earned and the chosen theme are saved in `~/.brushy-buddy-sim`.

### How the device UI is built

- **Buddy:** one still image per pose (idle, blink, sleepy, brush_a, brush_b, cheer, grin,
  plus the theme's trick poses),
  swapped by the UI; the bobbing, jumping, wiggling and giggle squash are LVGL animations
  (stepped instead of smooth for the blocky Minecraft style).
- **Code layout:** `brushy_ui.c` (phases, moods, ring, input, public API), `brushy_fx.c`
  (germs and their defeat, particles, shake, sparkles, reward drop), `brushy_tricks.c`
  (tricks), sharing state through `brushy_ui_priv.h`.
- **Tricks:** keyframe tables (position, scale, rotation around the feet) plus timed events,
  mirroring `TRICKS` in `prototype/js/app.js`. Theme tricks swap in extra poses (pickaxe,
  wings, rainbow colours). The shake moves a full-screen "stage" container holding everything.
- **Ring:** 4 `lv_arc` segments (plus black notches in blocky style). **Germs:** images
  that wobble and pop.
- **Lettering:** "Bravo !" and the digits 0-9 are images rendered in each theme's font.
  The big countdown digits are alpha-only (A8) images, tinted with the zone colour.
- **Sounds:** note sequences in `brushy_ui.c` played in the theme's voice, synthesized by
  the platform layer.
- **Themes:** `device/ui/themes/<id>.c` (colours, sound flavour; images are wired up by
  `BRUSHY_THEME_IMAGES`), listed in `themes/themes.c`.

### Changing the artwork

The prototype's themes are the single source. After editing one:

```sh
tools/export_assets.sh [theme]    # renders assets/png/<theme>_*.png with headless Chrome
cmake --build device/sim/build    # converts the PNGs to LVGL images and rebuilds
```

The poses and sprites to export are read from the theme itself (`tools/theme_assets.js`).
A new theme also needs `device/ui/themes/<id>.c` (copy `dragon.c`), an entry in
`themes/themes.c`, and a line in `device/sim/CMakeLists.txt`.

## Phase 3 — on the real board (next)

Add an ESP-IDF project that reuses `device/ui/`, with the board's display, touch,
audio and NVS storage behind `brushy_hal.h`.
