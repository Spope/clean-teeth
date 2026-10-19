# Brushy Buddy

A cute 2-minute tooth-brushing timer for a small round screen
([Waveshare ESP32-S3 Touch AMOLED 1.75"](https://www.waveshare.com/esp32-s3-touch-amoled-1.75.htm), 466×466).

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

- **Tap** the screen to start. Brushing lasts 2 minutes, split into 4 zones of 30 s,
  shown clockwise on the ring: top-left → top-right → bottom-right → bottom-left.
- Germs around the buddy pop as time passes; a chime plays at each zone change.
- Tapping during brushing makes the buddy giggle.
- At the end: confetti, "Bravo!", and +1 star (stored in the browser).
- After 30 s without a tap, the buddy falls asleep.

The panel under the device is for development only: speed (1×/10×/30×), buddy picker,
sound, skip-to-end, reset.

Preview a state directly with a URL parameter:

```
index.html?preview=idle | sleepy | brushing&t=50 | done
```

### Prototype layout

```
prototype/
  index.html            simulated device + dev panel
  css/style.css         device, ring, germs, overlays
  js/app.js             timer, phases, ring, germs, input
  js/germs.js           germ artwork (shared with the asset exporter)
  js/sound.js           tiny WebAudio synth (no audio files)
  characters/
    registry.js         character contract (read this first)
    dino.js             the dinosaur
```

### Adding or changing a buddy

Characters are swappable. Copy `prototype/characters/dino.js` to e.g. `bunny.js`,
change `id`, `name`, `theme`, `poses`, the SVG and its CSS, then add a `<script>` tag for it
in `prototype/index.html` (after `dino.js`). It will appear in the "Buddy" picker.
To bring it to the device, see "Changing the artwork" below.

A character reacts to four mood classes set by the app: `mood-idle`, `mood-sleepy`,
`mood-brushing`, `mood-cheer`. The tap "giggle" is handled generically.

To just recolour the dino, edit the CSS variables at the top of its `css`
(`--skin`, `--skin-dark`, `--belly`, `--spikes`).

### Design notes for the real screen

- AMOLED: black pixels are off — keep the background pure black.
- The real 1.75" panel is about 4.4 cm across; the browser view is much larger,
  so check that shapes stay readable when scaled down (e.g. zoom out).
- Keep touch interaction to one big tap.

## Phase 2 — LVGL simulator (current)

The device UI (`device/ui/`) is written in C with [LVGL](https://lvgl.io) v9.5 and runs
in a 466×466 desktop window. The same code will run on the ESP32-S3; only the
platform layer (`brushy_hal.h`: sound + saved stars, plus display/touch drivers) changes.

### Build and run (macOS)

```sh
brew install cmake ninja sdl2          # once
cmake -S device/sim -B device/sim/build -G Ninja
cmake --build device/sim/build
device/sim/build/brushy_sim
```

The first configure downloads LVGL (pinned to v9.5.0). Click the window to tap.

Options:

```
--speed 10                 run time 10x faster (testing)
--preview brushing:50      start in a state: idle | sleepy | brushing[:seconds] | done
--mute                     no sound
--zoom 0.5                 smaller window, closer to the real 4.4 cm panel size
--snapshot out.bmp --after 1500   save a screenshot after 1.5 s and quit
```

Stars earned in the simulator are saved in `~/.brushy-buddy-sim`.

### How the device UI is built

- **Character:** one still image per pose (idle, blink, sleepy, brush_a, brush_b, cheer),
  swapped by the UI; the bobbing, jumping, wiggling and giggle squash are LVGL animations.
- **Ring:** 4 `lv_arc` segments. **Germs:** images that wobble and pop.
- **Sounds:** note sequences in `brushy_ui.c`, synthesized by the platform layer.
- **Text:** LVGL's built-in Montserrat font for now (the prototype uses Fredoka).

### Changing the artwork

The prototype's SVG is the single source. After editing a character or germs:

```sh
tools/export_assets.sh            # renders assets/png/*.png with headless Chrome
cmake --build device/sim/build    # converts the PNGs to LVGL images and rebuilds
```

A new character also needs a `device/ui/characters/<id>.c` descriptor (copy `dino.c`).

## Phase 3 — on the real board (next)

Add an ESP-IDF project that reuses `device/ui/`, with the board's display, touch,
audio and NVS storage behind `brushy_hal.h`.
