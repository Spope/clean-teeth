# Brushy Buddy

A cute 2-minute tooth-brushing timer for a small round screen
([Waveshare ESP32-S3 Touch AMOLED 1.75"](https://www.waveshare.com/esp32-s3-touch-amoled-1.75.htm), 466×466).

## Phase 1 — browser prototype (current)

No build step. Open `index.html` in a browser (double-click works), or serve it:

```sh
python3 -m http.server 8000   # then open http://localhost:8000
```

To try it on a phone/tablet on the same Wi-Fi, open `http://<your-mac-ip>:8000`.

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

### Project layout

```
index.html            simulated device + dev panel
css/style.css         device, ring, germs, overlays
js/app.js             timer, phases, ring, germs, input
js/sound.js           tiny WebAudio synth (no audio files)
characters/
  registry.js         character contract (read this first)
  dino.js             the dinosaur
```

### Adding or changing a buddy

Characters are swappable. Copy `characters/dino.js` to e.g. `characters/bunny.js`,
change `id`, `name`, `theme`, the SVG and its CSS, then add a `<script>` tag for it
in `index.html` (after `dino.js`). It will appear in the "Buddy" picker.

A character reacts to four mood classes set by the app: `mood-idle`, `mood-sleepy`,
`mood-brushing`, `mood-cheer`. The tap "giggle" is handled generically.

To just recolour the dino, edit the CSS variables at the top of its `css`
(`--skin`, `--skin-dark`, `--belly`, `--spikes`).

### Design notes for the real screen

- AMOLED: black pixels are off — keep the background pure black.
- The real 1.75" panel is about 4.4 cm across; the browser view is much larger,
  so check that shapes stay readable when scaled down (e.g. zoom out).
- Keep touch interaction to one big tap.

## Phase 2 — LVGL (next)

Port the UI to [LVGL](https://lvgl.io) and run it in a desktop SDL simulator at 466×466,
then swap in the board's display/touch drivers once the hardware arrives.
