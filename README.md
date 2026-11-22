# Brushy Buddy

A cute 2-minute tooth-brushing timer for a small round screen
([Waveshare ESP32-S3 Touch AMOLED 1.75"](https://www.waveshare.com/esp32-s3-touch-amoled-1.75.htm), 466×466).

Two themes, switched by **pressing and holding** the start screen (in the browser prototype
and the simulator; each device is built with one of them):

- **Minecraft**: Steve, mob heads as germs (creeper, zombie, slime, spider),
  diamonds as rewards, a blocky XP-bar ring, chiptune sounds.
- **Les Royaumes de feu**: Glory the RainWing, whose scales change colour with her mood
  (teal → lavender when sleepy → pink when happy), jungle bugs as germs (mosquito, beetle,
  caterpillar, spider), gems as rewards, venom "fwoosh" pops.

```
prototype/     Phase 1: browser prototype, and the source of all themes and artwork (SVG)
assets/        themes exported from the prototype for the device (PNGs + settings)
device/ui/     Phase 2: device UI in C + LVGL (platform-independent)
device/sim/    Phase 2: simulator (SDL window, or a web page)
device/esp32/  Phase 3: firmware for the board (ESP-IDF)
tools/         theme export + conversion to C
docs/          how to create a theme
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
- At the end: confetti, "Bravo !", and the reward drop.
- After 30 s without a tap, the buddy falls asleep.
- Every 8-15 s while brushing, the buddy plays a trick (never on the start screen):
  - **peek-a-boo** (ducks out, pops up big and close
    from the bottom edge, comes back), **peek from the side** (leans in from an edge to spy,
    twice), **slide** (walks off one side, hops across the whole screen, walks back in),
    and Steve's Minecraft tricks: **mining** (a grass block appears, he breaks it in three
    pickaxe hits with his left hand) and **XP orbs** (orbs appear around the ring and zip
    into him, each with a "ding", then he levels up), and Glory's **rainbow** scales and
    **flying** across the top of the screen (she briefly puts the toothbrush away);
  - never the same trick twice in a row.
- Germs are defeated the theme's way: Minecraft mobs burst into pixels (the creeper swells
  and flashes white first, with a fuse hiss); Glory spits **venom** at each bug
  (RainWings don't breathe fire), and it melts.
  The screen **shakes** and the remaining germs **shiver with scared faces**.
- At the end, the **reward drops** out of the buddy, bounces (in pixel steps for Minecraft)
  and is collected with a little chime.

The panel under the device is for development only: speed (1×/10×/30×), theme picker,
tricks (play one now; those the theme doesn't have are greyed out), sound, skip-to-end, reset,
low battery warning, USB plugged in or not (plugging in shows the battery level, and a cable
under the device).

Preview a state directly with a URL parameter:

```
index.html?theme=dragon&preview=idle | sleepy | brushing&t=50 | done
          &trick=peek | peekSide | slide | mine | xp | rainbow | fly | mirror | shake
          &battery=18   (charge at boot: 25 or less shows the low battery warning)
          &charging=62  (plugs in USB with that charge)
```

### Prototype layout

```
prototype/
  index.html            simulated device + dev panel
  css/device.css        device, dev panel (shared with the simulator's web page)
  css/style.css         what the screen shows: ring, germs, overlays
  js/app.js             timer, phases, ring, germs, input
  js/pixel.js           pixel-art helper (grid of letters -> SVG)
  js/sound.js           tiny WebAudio synth (no audio files)
  themes/
    registry.js         theme contract and checks (read this first)
    _template.js        starting point for a new theme
    minecraft.js        Steve, mobs, diamond
    dragon.js           Glory, germs, gem
```

### Adding or changing a theme

A theme bundles the buddy, germs, reward icon, colours, fonts and sound flavour, in one file:
`prototype/themes/<id>.js`. The device's images and settings are exported from it, so there is
no C to write. **See [docs/themes.md](docs/themes.md)** to create one: copy `_template.js`,
add a `<script>` tag in `prototype/index.html`, draw, then `tools/export_assets.sh <id>`.

A buddy reacts to mood classes set by the app (`mood-idle`, `mood-sleepy`, `mood-brushing`,
`mood-cheer`, `mood-grin`...). The tap "giggle" is handled generically.
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
--theme dragon             a theme id: minecraft | dragon | ... (default: last used)
--speed 10                 run time 10x faster (testing)
--preview brushing:50      start in a state: idle | sleepy | brushing[:seconds] | done
--trick peek               play an animation right away: peek | peek_side | slide |
                           mine | xp | rainbow | fly | mirror | shake
--mute                     no sound
--battery 18               battery charge at boot: 25 or less shows the low battery warning
--charging 62              show the battery level as when USB is plugged in
--zoom 0.5                 smaller window, closer to the real 4.4 cm panel size
--snapshot out.bmp --after 1500   save a screenshot after 1.5 s and quit
```

Rewards earned and the chosen theme are saved in `~/.brushy-buddy-sim`.

### On a web page

The simulator also runs in a browser: the same C, compiled to WebAssembly with
[Emscripten](https://emscripten.org), in the prototype's device and dev panel
(`prototype/css/device.css`; `device/sim/web.c` connects the panel to the device UI).

```sh
brew install emscripten                           # once
tools/build_web.sh                                # builds device/sim/build-web
python3 -m http.server -d device/sim/build-web    # then open http://localhost:8000
```

The page reads the simulator's options from its URL: `?theme=dragon&preview=brushing:40&mute`
(theme, preview, trick, speed, battery, charging, mute). Embed it with an `<iframe>`: the device
scales to the frame's width, and the page has no background of its own. Its height depends on how
the panel wraps, so it posts it to the page around it (`{brushySimHeight}`), which resizes the
frame. Sound starts after the first tap, as browsers require, and nothing is saved once the page
is closed.

To put it on a website, copy its four files there (`index.html`, `device.css`, `brushy_sim.js`,
`brushy_sim.wasm`, about 1.8 MB to download, compressed), for example into a Hugo blog:

```sh
tools/build_web.sh ~/blog/static/brushy-sim
```

For Hugo, `device/sim/web/hugo/brushy-sim.html` is a shortcode (frame and resizing): copy it to
the blog's `layouts/_shortcodes/` (`layouts/shortcodes/` before Hugo 0.146), then in a post:

```
{{< brushy-sim theme="dragon" >}}
{{< brushy-sim theme="minecraft" preview="brushing:40" size="400" >}}
```

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
- **Lettering:** "Bravo !" is an image rendered in each theme's font.
  The big countdown digits are alpha-only (A8) images, tinted with the zone colour.
- **Sounds:** note sequences in `brushy_ui.c` played in the theme's voice, synthesized by
  `brushy_synth.c` (shared by the simulator and the board) and played by the platform layer.
- **Themes:** not written in C. The build generates each theme's `brushy_theme_t`
  (`brushy_theme.h`) and images from what the prototype exported to `assets/`
  (`device/ui/themes.cmake`, `tools/png2lvgl.py`). The simulator has every exported theme.

### Changing the artwork

The prototype's themes are the single source. After editing one:

```sh
tools/export_assets.sh [theme]    # assets/png/<theme>_*.png (headless Chrome) and assets/themes/<theme>.json
cmake --build device/sim/build    # converts them to C and rebuilds
```

The poses and sprites to export are read from the theme itself (`tools/theme_assets.js`).
A new theme needs nothing more for the simulator; for the board, an `sdkconfig.<id>`
(see [docs/themes.md](docs/themes.md)).

## Phase 3 — on the real board (current)

`device/esp32/` is an ESP-IDF project that builds the same `device/ui/` code for the
Waveshare ESP32-S3 Touch AMOLED 1.75. Waveshare's board package
(`waveshare/esp32_s3_touch_amoled_1_75`) brings up the CO5300 display (rounding redraw
areas to even coordinates, which the panel needs), the CST9217 touch, the ES8311 speaker
codec and the LVGL task. `main/hal_esp32.c` implements `brushy_hal.h`: sounds are
rendered by an audio task and played through the speaker, settings go to NVS.

Battery and power come from the AXP2101 power chip. At boot on battery, 25% or less shows the
low battery warning. Plugging in USB shows the battery level with a charging bolt for 5 s
(`main.c` checks for USB power every 500 ms). The chip also switches the device on when USB is
plugged in while it's off; it then shows the level as well. The buddy wakes up, and the power-off
countdown starts again.

### Build and flash (macOS)

Needs ESP-IDF 5.5 with the ESP32-S3 toolchain (`~/esp/esp-idf/install.sh esp32s3`, once).

Each device has one theme, and only its artwork goes into the firmware: `sdkconfig.<id>` sets
it (`CONFIG_BRUSHY_THEME="<id>"`). Build each in its own folder (the first build downloads the
components):

```sh
. ~/esp/esp-idf/export.sh
cd device/esp32
idf.py -B build_minecraft -D SDKCONFIG=build_minecraft/sdkconfig \
  -D SDKCONFIG_DEFAULTS="sdkconfig.defaults;sdkconfig.minecraft" -p /dev/cu.usbmodem* flash monitor
idf.py -B build_dragon -D SDKCONFIG=build_dragon/sdkconfig \
  -D SDKCONFIG_DEFAULTS="sdkconfig.defaults;sdkconfig.dragon" -p /dev/cu.usbmodem* flash monitor
```

Ctrl-] quits the monitor. Plain `idf.py build` builds the Minecraft theme (`idf.py menuconfig`,
Brushy Buddy > Theme, to change it). The settings are read from `sdkconfig.defaults` only
when the build's `sdkconfig` is created: delete it after changing them.

If flashing fails to connect, unplug USB, hold BOOT while plugging it back in, and try again.

Memory: the artwork (about 5 MB for the dragon, 3 MB for Minecraft) is linked into the app
and read straight from flash; LVGL's heap, the draw buffers and the image cache are in the
8 MB PSRAM.

### Frame rate

LVGL draws in software, and rotating or scaling an image costs the ESP32-S3 several
milliseconds per image and frame. So what moves all the time avoids it: the germ wobble
and Glory's brushing sway swap between pre-rotated frames (`TILTS` in `tools/png2lvgl.py`),
and the ring draws each arc only where the redrawn area touches it (`ring_draw_cb`).

The peek-a-boo zooms the buddy to 1.9x: rather than scaling him, it shows the pose already
drawn at that size and tilt (`*_peek` images, `PEEK` in `tools/png2lvgl.py`). These 12 frames
are LZ4-compressed in flash (0.6 MB instead of 4.3 MB for the dragon) and decompressed into
LVGL's image cache when the theme loads, about 30 ms each.

To measure, build the benchmark: it plays every scene and trick in turn and logs the
frame rate and render time every few hundred ms.

```sh
idf.py -B build_bench -D SDKCONFIG=build_bench/sdkconfig \
  -D SDKCONFIG_DEFAULTS="sdkconfig.defaults;sdkconfig.dragon;sdkconfig.bench" -p /dev/cu.usbmodem* flash monitor
```

### Still to do

- Try it on the board: touch orientation, sound volume (`VOLUME` in `hal_esp32.c`), the
  low battery warning with a battery plugged in (the boot log prints `battery NN%`), and the
  battery level when USB is plugged in, while on and while off (the boot log prints what
  switched it on).
- Frame rate of the tricks that rotate the whole buddy (slide, mining, flying: down to
  13-16 FPS at their biggest moments) and of Glory's sleepy breathing (40-50 FPS).
- Power: dim or switch off the screen while the buddy sleeps (it powers off 10 s after the
  end of brushing, or 2 min after falling asleep; the PWR button turns it on).
