// Theme registry. Each theme file calls Themes.register({...}).
//
// Theme contract:
//   id, name
//   style      'blocky' (square ring blocks, button, confetti) | 'round'
//   accent     button colour            glow   soft colour behind the buddy
//   ring       [4 colours], one per brushing zone
//   fontUrl    optional Google Fonts stylesheet for the theme's text
//   bravo      end-of-brushing title
//   sound      { voice: 'square' | 'triangle' | 'sine', pop: 'blip' | 'whoosh' }
//   css        styles scoped to `.theme-<id>`: the buddy's moods and the
//              `.t-counter` / `.t-bravo` text styles
//   character  inline SVG of the buddy (drawn 250x250)
//   poses      still frames for the device: idle, blink, sleepy, brush_a, brush_b, cheer,
//              grin, plus any the theme's tricks need; each { mood, cls?, css? }
//              (see tools/export_assets.sh)
//   germs      [4 SVG snippets] centred on (0,0), about 42px across
//   germsScared  same 4 germs with scared faces (when another germ pops)
//   germParticles  [4 colour lists] for the burst when each germ pops
//   pop        how germs are defeated: 'creeper' (germ 0 swells and flashes, then all
//              burst into pixels) | 'venom' (the buddy spits venom at them)
//   tricks     theme-only tricks for the start screen: 'mine' | 'rainbow' | 'fly'
//   teeth, mouth  [x, y] in the 250x250 buddy box: mirror-check sparkle, venom start
//   sprites    { name: { w, h, svg } }: sparkle (all themes), block + block_cracked
//              (mine), venom (venom pop)
//   reward     SVG of the reward icon (drawn 32x32)
//
// The app puts one mood class on the buddy's host element (.character):
//   mood-idle | mood-sleepy | mood-brushing | mood-cheer | mood-grin | mood-spit
// plus trick classes while a theme trick runs (trick-mine, hit, trick-fly, rainbow-N)
// and briefly `giggle` when the child taps (handled generically).
window.Themes = (function () {
  'use strict';
  const map = new Map();
  return {
    register(t) { map.set(t.id, t); },
    get(id) { return map.get(id); },
    list() { return [...map.values()]; },
  };
})();
