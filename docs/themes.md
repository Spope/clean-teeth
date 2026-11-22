# Creating a theme

A theme is what gives the buddy its personality: the buddy itself in each mood, the four germs,
the reward, the colours, the lettering and the flavour of the sounds. There are two so far:
Minecraft (Steve) and Les Royaumes de feu (Glory).

A theme is written once, as one JavaScript file in the browser prototype. The device's images
and settings are exported from it, so there is no C to write:

```
prototype/themes/<id>.js       the theme: you write this
        │
        │  tools/export_assets.sh <id>        (Node + headless Chrome)
        ▼
assets/png/<id>_*.png          every pose, germ, sprite and text, as images
assets/themes/<id>.json        colours, sounds, tricks... for the device
        │
        │  device build: tools/png2lvgl.py    (automatic)
        ▼
<build>/assets/                LVGL images, and themes.c: the theme's brushy_theme_t
```

## Quick start

1. Copy the starter theme. The id is made of lowercase letters and digits, and it is also the
   file name:

   ```sh
   cp prototype/themes/_template.js prototype/themes/robot.js
   ```

   In `robot.js`, set `ID = 'robot'` and `NAME`.

2. Load it in `prototype/index.html`, after the other themes:

   ```html
   <script src="themes/robot.js"></script>
   ```

3. Open the prototype (see the README) and choose it in the dev panel's Theme menu, or open
   `index.html?theme=robot`. You get Blobby, a working buddy with everything a theme needs.
   Make it yours (see the next sections).

4. When it looks right, export it:

   ```sh
   tools/export_assets.sh robot
   ```

5. Try it on the simulator. It builds every exported theme, so there is nothing to register:

   ```sh
   cmake --build device/sim/build
   device/sim/build/brushy_sim --theme robot
   ```

6. For the board, add `device/esp32/sdkconfig.robot`:

   ```
   # Device with the robot theme (see the README)
   CONFIG_BRUSHY_THEME="robot"
   ```

   and build it like the others, in its own folder:

   ```sh
   idf.py -B build_robot -D SDKCONFIG=build_robot/sdkconfig \
     -D SDKCONFIG_DEFAULTS="sdkconfig.defaults;sdkconfig.robot" -p /dev/cu.usbmodem* flash monitor
   ```

7. Commit the theme, `index.html`, `assets/png/robot_*.png`, `assets/themes/robot.json` and
   `sdkconfig.robot`.

After each change to the theme, export it again (step 4); the builds pick the changes up.

## What a theme is made of

The theme registers an object with `Themes.register({...})`. The exact contract is at the top
of `prototype/themes/registry.js`; in short:

| Fields | What they are |
|---|---|
| `id`, `name` | Its id (lowercase letters and digits, as the file name), and the name in the theme picker. |
| `style` | `'round'`, or `'blocky'` for pixel art: the ring is cut into blocks, the button and confetti are square, and motion moves in steps. |
| `accent`, `ring` | `'#rrggbb'` colours: the start button, and one per brushing zone (they also colour the countdown and the confetti). |
| `glow` | The soft colour behind the buddy (prototype only). |
| `fontUrl`, `bravo` | The lettering's Google Font, and the title at the end. |
| `sound` | `voice` of the melodies: `'square'` (chiptune), `'triangle'` (soft) or `'sine'` (pure); `pop`: `'blip'` or `'whoosh'` when a germ pops. |
| `character`, `css`, `poses`, `sway` | The buddy: an SVG, the CSS for its moods, the still frames for the device, and whether it rocks while brushing. |
| `germs`, `germsScared`, `germParticles` | The 4 germs, calm and scared, and the colours they burst into. |
| `teeth`, `mouth` | Points in the buddy's box: where the mirror check sparkles, where venom starts. |
| `sprites`, `reward` | Small images: the sparkle (every theme) and those of the theme's tricks; the reward icon. |
| `pop`, `tricks` | Optional: how germs are defeated, and theme-only tricks. |

The registry checks the theme when it loads. Problems show in the browser console, for example
`Theme "robot": poses.grin: missing (every theme)`, and the export refuses the theme until they
are fixed.

## Drawing the buddy

`character` is an inline SVG, drawn in a 250×250 box.

- **It faces the viewer**, head included: the buddy looks at the child who is brushing. When
  drawing after reference art (a character from a book or a game), take its style and colours,
  not its viewing angle.
- **Feet on the bottom edge, centred.** The device moves, tilts and zooms the buddy around the
  bottom centre of the box. The peek-a-boo pops it up ×1.9 from the bottom of the screen and only
  shows what is more than about 70 px above its feet, so keep the face well above that.
- **Scope every CSS rule** with `.theme-<id>` (`T` in the starter) so themes don't affect each
  other, and name keyframes after the theme (`${ID}-bob`).
- **Pixel art:** draw with `Pixel.grid` (`prototype/js/pixel.js`: rows of letters, one letter
  per colour) and set `style: 'blocky'`, as `minecraft.js` does.

### Moods

The app only tells the buddy its mood, with a class on the element around it. The theme's
`css` does the rest, usually by showing and hiding parts of the SVG:

| Mood class | When | What the buddy shows |
|---|---|---|
| `mood-idle` | start screen | calm, waiting, blinking |
| `mood-sleepy` | after 30 s without a tap | eyes closed, "z Z" |
| `mood-brushing` | the 2 minutes | the toothbrush in its mouth, foam |
| `mood-cheer` | the end | overjoyed: happy eyes, open mouth, arms (or wings) up, sparkles |
| `mood-grin` | mirror check | a big proud grin showing its teeth |
| `mood-spit` | only with `pop: 'venom'` | spitting |

A tap makes it giggle (handled by the app). The starter's parts show the usual way to do it
(Steve and Glory use mostly the same names):

| Part (class) | Shown |
|---|---|
| `body-all` | always: everything that moves together |
| `eyes-open` | except when sleepy, cheering or grinning |
| `eyes-closed` | sleepy, and the `blink` pose |
| `eyes-happy` | cheer and grin |
| `mouth` / `mouth-open` / `mouth-grin` | the usual mouth / cheer / grin (the mirror check sparkles at `teeth`) |
| `brush`, `foam` | brushing: the toothbrush is held by the arm on the right of the picture, so `arm-r` hides meanwhile |
| `arms-down` / `arms-up` | always / instead when cheering |
| `sparkles` | cheer |
| `zzz` | sleepy |

### Poses: what the device shows

The device doesn't run the CSS. It shows still images of the buddy, exported from `poses`, and
moves them itself. Each pose is a mood, plus extra classes (`cls`) or CSS (`css`) for that
frame. The export turns animations off, so a pose shows each part at rest: make every pose read
well as a still picture.

| Pose | When the device shows it |
|---|---|
| `idle` | start screen |
| `blink` | 150 ms every 4 s on the start screen: the idle pose with eyes closed |
| `sleepy` | asleep |
| `brush_a`, `brush_b` | while brushing, in turn every 280 ms: the toothbrush at two places (the starter moves it 8 px left and right) |
| `cheer` | the end |
| `grin` | mirror check |

The device's own motion depends on `style`: the buddy bobs on the start screen, breathes while
asleep (blocky: bobs slowly) and jumps at the end, in steps when blocky. With `sway: true` it
also rocks ±2° while brushing, from pre-rotated frames (8 more images, about 1.5 MB); pixel art
should stay upright.

## Germs

- `germs`: 4 SVG snippets centred on (0, 0), about 42 px across. They are exported in a
  52×52 box, so nothing may go beyond ±26 px.
- `germsScared`: the same 4 with a scared face, shown while another germ pops. Keep the bodies
  the same, so only the face changes.
- `germParticles`: for each germ, 1 to 4 `'#rrggbb'` colours for its burst. Not pure black: the
  device uses 0 to mean "no colour".

There are 12 germs on screen, 3 per zone; the 4 kinds take turns.

## Text, colours and sprites

- `fontUrl`: a Google Fonts stylesheet; end it with `&display=block` so the export waits for it.
- `.t-clock` (in `css`): the big, faint countdown. Each glyph (0-9 and `:`) is exported in white
  and must fit in 100×110 px; the device tints it with the zone's colour.
- `.t-bravo` (in `css`): the title at the end, colours and shadow included, exported at 300×64 px.
- The screen is an AMOLED: black pixels are off, so the background stays black. Bright colours
  read best.
- `sprites.sparkle` (36×36): the mirror check's glints. Every theme needs it.
- `reward`: an SVG drawn at 32×32 (shown bigger as it drops out of the buddy).

## Optional: defeating germs, theme tricks

Every theme has the generic tricks (peek-a-boo, peek from the side, slide). A theme can also
change how germs are defeated (`pop`) and add tricks of its own (`tricks`), each needing a few
more poses or sprites:

| Setting | Needs | What happens |
|---|---|---|
| `pop: 'burst'` (default) | | germs burst into particles |
| `pop: 'creeper'` | | germ 0 swells and flashes white first, with a fuse hiss |
| `pop: 'venom'` | pose `spit` (mood `spit`), sprite `venom` (40×40), `mouth` | the buddy spits a glob from `mouth` at each germ |
| `tricks: ['mine']` | poses `mine_a` (pickaxe raised) and `mine_b` (it hits), optionally `mine_brush_a`, `mine_brush_b` (the same while brushing); sprites `block`, `block_cracked` (48×48) | breaks a block by its feet, on the left of the picture, in three hits (classes `trick-mine`, `hit`) |
| `tricks: ['xp']` | sprite `xp_orb` (32×32) | orbs fly into its chest, at (125, 170) |
| `tricks: ['rainbow']` | poses `rainbow_1` to `rainbow_5` | its colours cycle while it hops (classes `rainbow-1` to `rainbow-5`) |
| `tricks: ['fly']` | poses `fly_a`, `fly_b`: wings up, wings down | flies across the top of the screen (classes `trick-fly`, `flap`) |

These were made for Steve and Glory, and their positions suit them. A new trick or a new way to
defeat germs is code, in the prototype (`TRICKS` in `prototype/js/app.js`) and on the device
(`device/ui/brushy_tricks.c`, `brushy_fx.c`). The theme's images then reach the device code by
name: a pose `foo_bar` fills `poses[BRUSHY_POSE_FOO_BAR]` (add it to `brushy_pose_t` in
`device/ui/brushy_theme.h`), and a sprite `name` fills the `brushy_theme_t` field of that name.

## Checking a theme

- **Console:** the registry lists the theme's problems when the page loads.
- **Preview a state** with URL parameters: `index.html?theme=robot&preview=sleepy`, or
  `preview=brushing&t=50`, `preview=done`, `trick=mirror` (the mirror check), `trick=peek`...
  The dev panel plays each trick and speeds time up 30×.
- **Real size:** the panel is about 4.4 cm across. Zoom the browser out, or run the simulator
  with `--zoom 0.5`, and check that faces and germs stay readable.
- **After exporting,** look at `assets/png/robot_*.png`: the poses as the device will show them,
  digits and "Bravo !" not cut off.
- **On the simulator:** `--preview brushing:50`, `--trick peek` (the zoomed frames), and
  `--snapshot out.bmp --after 1500` to save a screenshot.
- **Size:** the firmware holds one theme's artwork. A 250×250 pose is about 190 KB, so Glory
  (15 poses, swaying) makes a 6 MB firmware and Steve 3.5 MB, out of a 12 MB app partition.

## Under the hood

- `tools/export_assets.sh <id>` (needs Node and Google Chrome) first checks the theme and writes
  `assets/themes/<id>.json` (`node tools/theme_assets.js <id> json`), then renders each image
  with headless Chrome (`tools/render.html`). The poses and sprites come from the theme; the
  peek-a-boo frames (`idle`, `blink`, `brush_a`, `brush_b` at ×1.9), germs, reward, title and
  digits are rendered for every theme. Without an id, it exports every theme.
- The device builds call `brushy_theme_sources()` from `device/ui/themes.cmake`, which runs
  `tools/png2lvgl.py`. It converts the PNGs (pre-rotating the germs, and the brushing poses of
  swaying themes; cropping and compressing the peek-a-boo frames) and writes `themes.c`, with
  each theme's `brushy_theme_t` (`device/ui/brushy_theme.h`). The simulator builds every theme
  in `assets/themes/`, and a long press cycles through them in alphabetical order; the board
  builds the one in `CONFIG_BRUSHY_THEME`.
- Everything in `assets/` and in the builds' `assets/` folders is generated: change the theme
  and export it again rather than editing them.
