// Theme registry. Each theme file calls Themes.register({...}); docs/themes.md explains how
// to make one (start from _template.js). The device's images and settings are exported from
// these files (tools/export_assets.sh), so a theme is written once, here.
//
// Theme contract (Themes.check() enforces it: problems are logged in the browser console,
// and the export refuses the theme):
//   id         lowercase letters and digits, the same as the file name (prototype/themes/<id>.js)
//   name       shown in the dev panel's theme picker
//   style      'blocky' (square ring blocks, button, confetti, stepped motion) | 'round'
//   accent     '#rrggbb' start button       glow   soft colour behind the buddy (prototype only)
//   ring       4 '#rrggbb' colours, one per brushing zone (also the countdown and confetti)
//   fontUrl    optional Google Fonts stylesheet for the theme's text
//   bravo      end-of-brushing title
//   sound      { voice: 'square' | 'triangle' | 'sine', pop: 'blip' | 'whoosh' }
//   css        styles scoped to `.theme-<id>`: the buddy's moods and the
//              `.t-clock` / `.t-bravo` text styles
//   character  inline SVG of the buddy, drawn in a 250x250 box, feet on its bottom edge
//   poses      still frames for the device: idle, blink, sleepy, brush_a, brush_b, cheer,
//              grin, plus those the theme's tricks need; each { mood, cls?, css? }
//              (mood: one of MOODS below; cls: extra classes; css: extra styles)
//   sway       optional: on the device, the buddy rocks while brushing (round style)
//   germs      [4 SVG snippets] centred on (0,0), about 42px across (52px at most)
//   germsScared  same 4 germs with scared faces (when another germ pops)
//   germParticles  [4 lists of 1 to 4 '#rrggbb' colours, not black] for each germ's burst
//   pop        how germs are defeated: 'burst' (default: they burst into particles) |
//              'creeper' (germ 0 swells and flashes first) | 'venom' (the buddy spits venom)
//   tricks     theme-only tricks: 'mine' | 'xp' | 'rainbow' | 'fly' (see NEEDS)
//   teeth, mouth  [x, y] in the 250x250 buddy box: mirror-check sparkle, venom start
//   sprites    { name: { w, h, svg } }: sparkle (every theme), plus those of NEEDS
//   reward     SVG of the reward icon (drawn 32x32)
//
// The app puts one mood class on the buddy's host element (.character): mood-<MOODS>,
// plus trick classes while a theme trick runs (trick-mine, hit, trick-fly, flap, rainbow-N)
// and briefly `giggle` when the child taps (handled generically).
window.Themes = (function () {
  'use strict';
  const map = new Map();

  const MOODS = ['idle', 'sleepy', 'brushing', 'cheer', 'grin', 'spit'];
  const POSES = ['idle', 'blink', 'sleepy', 'brush_a', 'brush_b', 'cheer', 'grin'];
  // What an optional feature needs from the theme
  const NEEDS = {
    'pop venom':     { poses: ['spit'], sprites: ['venom'], points: ['mouth'] },
    'trick mine':    { poses: ['mine_a', 'mine_b'], sprites: ['block', 'block_cracked'] },
    'trick xp':      { sprites: ['xp_orb'] },
    'trick rainbow': { poses: ['rainbow_1', 'rainbow_2', 'rainbow_3', 'rainbow_4', 'rainbow_5'] },
    'trick fly':     { poses: ['fly_a', 'fly_b'] },
  };
  const COLOR = /^#[0-9a-f]{6}$/i; // the device reads these as 0xRRGGBB

  // Returns the theme's problems, as a list of messages (empty if none)
  function check(t) {
    const p = [];
    const isList = (v, n) => Array.isArray(v) && v.length === n;
    const isPoint = (v) => isList(v, 2) && v.every(Number.isFinite);
    const oneOf = (v, list) => list.includes(v);

    if (!/^[a-z][a-z0-9]*$/.test(t.id)) p.push(`id "${t.id}": use lowercase letters and digits only`);
    if (typeof t.name !== 'string') p.push('name: missing');
    if (!oneOf(t.style, ['blocky', 'round'])) p.push(`style: 'blocky' or 'round', not ${t.style}`);
    if (!COLOR.test(t.accent)) p.push(`accent: a '#rrggbb' colour, not ${t.accent}`);
    if (!isList(t.ring, 4) || !t.ring.every((c) => COLOR.test(c))) p.push("ring: 4 '#rrggbb' colours");
    if (typeof t.bravo !== 'string') p.push('bravo: missing');
    if (!t.sound || !oneOf(t.sound.voice, ['square', 'triangle', 'sine']) || !oneOf(t.sound.pop, ['blip', 'whoosh'])) {
      p.push("sound: { voice: 'square' | 'triangle' | 'sine', pop: 'blip' | 'whoosh' }");
    }
    if (typeof t.css !== 'string') p.push('css: missing');
    if (typeof t.character !== 'string' || !t.character.includes('<svg')) p.push('character: an inline <svg>');
    for (const key of ['germs', 'germsScared']) {
      if (!isList(t[key], 4)) p.push(`${key}: 4 SVG snippets`);
    }
    if (!isList(t.germParticles, 4) || !t.germParticles.every((list) => Array.isArray(list)
        && list.length >= 1 && list.length <= 4 && list.every((c) => COLOR.test(c) && !/^#0{6}$/.test(c)))) {
      p.push("germParticles: 4 lists of 1 to 4 '#rrggbb' colours, not black");
    }
    if (!isPoint(t.teeth)) p.push('teeth: [x, y] in the 250x250 buddy box');
    if (t.mouth !== undefined && !isPoint(t.mouth)) p.push('mouth: [x, y] in the 250x250 buddy box');
    if (typeof t.reward !== 'string') p.push('reward: an SVG');

    const poses = t.poses || {};
    const sprites = t.sprites || {};
    for (const [name, pose] of Object.entries(poses)) {
      if (!/^[a-z][a-z0-9_]*$/.test(name)) p.push(`poses.${name}: use lowercase letters, digits and _`);
      if (!pose || !oneOf(pose.mood, MOODS)) p.push(`poses.${name}: mood is one of ${MOODS.join(', ')}`);
    }
    for (const [name, s] of Object.entries(sprites)) {
      if (!s || !(s.w > 0 && s.h > 0) || typeof s.svg !== 'string') p.push(`sprites.${name}: { w, h, svg }`);
    }
    const need = (what, { poses: ps = [], sprites: ss = [], points = [] }) => {
      for (const name of ps) if (!poses[name]) p.push(`poses.${name}: missing (${what})`);
      for (const name of ss) if (!sprites[name]) p.push(`sprites.${name}: missing (${what})`);
      for (const name of points) if (!isPoint(t[name])) p.push(`${name}: missing (${what})`);
    };
    need('every theme', { poses: POSES, sprites: ['sparkle'] });

    if (t.pop !== undefined && !oneOf(t.pop, ['burst', 'creeper', 'venom'])) p.push(`pop: 'burst', 'creeper' or 'venom', not ${t.pop}`);
    if (t.pop === 'venom') need("pop: 'venom'", NEEDS['pop venom']);
    for (const trick of t.tricks || []) {
      if (NEEDS[`trick ${trick}`]) need(`trick '${trick}'`, NEEDS[`trick ${trick}`]);
      else p.push(`tricks: unknown trick '${trick}'`);
    }
    return p;
  }

  return {
    register(t) {
      const problems = check(t);
      if (problems.length) console.warn(`Theme "${t.id}" (see prototype/themes/registry.js):\n  ${problems.join('\n  ')}`);
      map.set(t.id, t);
    },
    get(id) { return map.get(id); },
    list() { return [...map.values()]; },
    check,
  };
})();
