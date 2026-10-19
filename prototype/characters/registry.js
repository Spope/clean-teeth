// Character registry. Each character file calls Characters.register({...}).
//
// Character contract:
//   id     unique string, also used as the CSS scope class `.char-<id>`
//   name   display name
//   theme  { accent, glow, ring: [4 colours, one per brushing zone] }
//   svg    inline SVG markup (viewBox 0 0 240 240 recommended)
//   css    styles scoped to `.char-<id>`, reacting to the mood classes below
//   poses  still frames for the device: idle, blink, sleepy, brush_a, brush_b, cheer
//          (each { mood, css? }; see characters/dino.js and tools/export_assets.sh)
//
// The app puts exactly one mood class on the character host element:
//   mood-idle      awake, waiting for a tap
//   mood-sleepy    nobody has tapped for a while
//   mood-brushing  timer running
//   mood-cheer     brushing finished
// It also briefly adds `giggle` when the child taps (handled generically).
window.Characters = (function () {
  'use strict';
  const map = new Map();
  return {
    register(c) { map.set(c.id, c); },
    get(id) { return map.get(id); },
    list() { return [...map.values()]; },
  };
})();
