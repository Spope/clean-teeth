// Minecraft theme: Steve brushes his teeth while the mobs around him get zapped.
(function () {
  'use strict';
  const { grid, rect } = Pixel;

  // ---------- Steve (16px blocks, head at 61,14) ----------
  const C = {
    H: '#3b2412', h: '#4f3119', S: '#c58c62', s: '#b07650', N: '#91583a',
    iris: '#5a4aa8', mouth: '#5a2e1c', cheek: '#e8807a', tongue: '#e86a7a',
    T: '#2ab3b3', t: '#1f9494', P: '#3c3ca8', p: '#2c2c80', shoe: '#6b6b6b',
  };
  const face = grid([
    'HHHHHHHH',
    'HHhHHHhH',
    'HSSSSSSH',
    'SSSSSSSS',
    'SSSSSSSS',
    'sSSNNSSs',
    'SSSSSSSS',
    'sSSSSSSs',
  ], C, 61, 14, 16);
  const torso = grid(['TTSTT', 'TTTTT', 'TTTTt', 'tTTTt'], C, 85, 142, 16);
  const armL = (cls) => `<g class="${cls}">${rect(53, 142, 32, 24, C.T)}${rect(53, 166, 32, 40, C.S)}${rect(53, 166, 6, 40, C.s)}</g>`;
  const armR = (cls) => `<g class="${cls}">${rect(165, 142, 32, 24, C.T)}${rect(165, 166, 32, 40, C.S)}${rect(191, 166, 6, 40, C.s)}</g>`;
  const armRotated = (deg) => `<g transform="rotate(${deg} 181 146)">${rect(165, 142, 32, 24, C.T)}${rect(165, 166, 32, 40, C.S)}</g>`;
  const armLRotated = (deg) => `<g transform="rotate(${deg} 69 146)">${rect(53, 142, 32, 24, C.T)}${rect(53, 166, 32, 40, C.S)}</g>`;
  const happyEye = (x) => rect(x, 88, 8, 5, C.H) + rect(x + 8, 82, 16, 5, C.H) + rect(x + 24, 88, 8, 5, C.H);
  const plus = (x, y, s) => rect(x - s / 2, y - 1.5 * s, s, 3 * s, '#ffe14d') + rect(x - 1.5 * s, y - s / 2, 3 * s, s, '#ffe14d');

  // Diamond pickaxe, origin at the bottom of the handle (where the hand holds it)
  const pickaxe = (x, y, deg) => `<g transform="translate(${x} ${y}) rotate(${deg})">${grid([
    '.DDDDD.',
    'D..H..D',
    '...H...',
    '...H...',
    '...H...',
    '...H...',
    '...H...',
  ], { D: '#4ee6dd', H: '#7a5230' }, -21, -42, 6)}</g>`;

  const character = `
<svg viewBox="0 0 250 250" aria-label="Steve">
  <g class="sparkles">
    <g>${plus(26, 60, 6)}</g><g>${plus(222, 58, 5)}</g><g>${plus(22, 176, 4)}</g><g>${plus(228, 168, 6)}</g>
  </g>
  <g class="body-all">
    ${rect(85, 206, 80, 32, C.P)}${rect(124, 206, 2, 32, C.p)}
    ${rect(85, 238, 38, 10, C.shoe)}${rect(127, 238, 38, 10, C.shoe)}
    ${armL('arm arm-l')}${armR('arm arm-r')}
    ${torso}
    <g class="arms-up">
      <g transform="rotate(160 69 146)">${rect(53, 142, 32, 24, C.T)}${rect(53, 166, 32, 40, C.S)}</g>
      ${armRotated(-160)}
    </g>
    <!-- mining with the left hand (the right one holds the toothbrush) -->
    <g class="pick-up">${armLRotated(160)}${pickaxe(49, 92, -25)}</g>
    <g class="pick-down">${armLRotated(50)}${pickaxe(23, 185, 180)}</g>
    ${face}
    ${rect(69, 98, 16, 8, C.cheek, 'cheek')}${rect(165, 98, 16, 8, C.cheek, 'cheek')}

    <g class="eyes-closed">${rect(77, 86, 32, 5, C.H)}${rect(141, 86, 32, 5, C.H)}</g>
    <g class="eyes-open">
      ${rect(77, 78, 16, 16, '#fff')}${rect(93, 78, 16, 16, C.iris)}
      ${rect(141, 78, 16, 16, C.iris)}${rect(157, 78, 16, 16, '#fff')}
      ${rect(95, 80, 4, 4, '#fff')}${rect(143, 80, 4, 4, '#fff')}
    </g>
    <g class="eyes-happy">${happyEye(77)}${happyEye(141)}</g>

    <g class="mouth">
      ${rect(93, 106, 8, 6, C.mouth)}${rect(149, 106, 8, 6, C.mouth)}
      ${rect(101, 110, 48, 7, '#fff')}${rect(101, 117, 48, 5, C.mouth)}
    </g>
    <g class="mouth-open">
      ${rect(101, 108, 48, 18, C.mouth)}${rect(105, 108, 40, 6, '#fff')}${rect(113, 118, 24, 8, C.tongue)}
    </g>
    <g class="mouth-grin">
      ${rect(89, 102, 8, 6, C.mouth)}${rect(153, 102, 8, 6, C.mouth)}
      ${rect(95, 106, 60, 18, C.mouth)}
      ${rect(97, 107, 56, 7, '#fff')}${rect(99, 117, 52, 6, '#fff')}
      ${rect(110, 107, 2, 7, C.mouth)}${rect(124, 107, 2, 7, C.mouth)}${rect(138, 107, 2, 7, C.mouth)}
    </g>

    <g class="foam">
      ${rect(95, 122, 6, 6, '#fff')}${rect(89, 130, 5, 5, '#fff')}${rect(103, 132, 4, 4, '#fff')}${rect(155, 126, 5, 5, '#fff')}
    </g>
    <g class="brush">
      ${rect(173, 130, 24, 36, C.T)}${rect(173, 106, 24, 24, C.S)}
      ${rect(125, 114, 52, 8, '#4fc3f7')}${rect(105, 110, 24, 16, '#fff')}${rect(107, 104, 20, 6, '#c8f0ff')}
    </g>
  </g>
  <g class="zzz">
    <text x="200" y="46" font-size="16">Z</text>
    <text x="218" y="24" font-size="22">Z</text>
  </g>
</svg>`;

  // ---------- Mobs as germs (8x8 heads, 5px blocks) ----------
  const mob = (rows, pal) => grid(rows, pal, -20, -20, 5);
  const MOBS = [
    { // creeper
      pal: { l: '#79d24f', g: '#5dbb3a', G: '#4a9c2c', k: '#10200c', W: '#fff' },
      calm: ['lgGglgGg', 'gGlgGglG', 'gkkGgkkg', 'Gkklgkkg', 'gGgkkgGl', 'lgkkkkgG', 'gGkkkkGg', 'Ggkgglkg'],
      scared: ['lgGglgGg', 'gGlgGglG', 'gWWGgWWg', 'GWklgkWg', 'gGgkkgGl', 'lgkkkkgG', 'gGkkkkGg', 'Ggkgglkg'],
      particles: ['#5dbb3a', '#79d24f', '#4a9c2c', '#10200c'],
    },
    { // zombie
      pal: { H: '#2c4a3a', z: '#5fa896', Z: '#4a8a7a', k: '#13201c', W: '#fff' },
      calm: ['HHHHHHHH', 'HHzHHHzH', 'zzzzzzzz', 'zkkzzkkz', 'zzzZZzzz', 'zZkkkkZz', 'zzzzzzzz', 'ZzzzzzzZ'],
      scared: ['HHHHHHHH', 'HHzHHHzH', 'zzzzzzzz', 'zWkzzkWz', 'zzzZZzzz', 'zZzkkzZz', 'zzzkkzzz', 'ZzzzzzzZ'],
      particles: ['#5fa896', '#4a8a7a', '#2c4a3a'],
    },
    { // slime
      pal: { s: '#6cc45c', S: '#93e283', k: '#1b3d16', W: '#fff' },
      calm: ['ssssssss', 'sSSSSSSs', 'sSkSSkSs', 'sSkSSkSs', 'sSSSSSSs', 'sSSSkSSs', 'sSSSSSSs', 'ssssssss'],
      scared: ['ssssssss', 'sSSSSSSs', 'sWkSSkWs', 'sWWSSWWs', 'sSSSSSSs', 'sSSkkSSs', 'sSSkkSSs', 'ssssssss'],
      particles: ['#6cc45c', '#93e283', '#b5f0a8'],
    },
    { // spider
      pal: { a: '#3b3535', A: '#2a2525', r: '#e0383e', W: '#fff' },
      calm: ['aaaaaaaa', 'aAaaaaAa', 'arraarra', 'aaaaaaaa', 'araaaara', 'aaaaaaaa', 'aAaAAaAa', 'aaaaaaaa'],
      scared: ['aaaaaaaa', 'aAaaaaAa', 'aWWaaWWa', 'aaaaaaaa', 'aWaaaaWa', 'aaaaaaaa', 'aAaAAaAa', 'aaaaaaaa'],
      particles: ['#3b3535', '#2a2525', '#e0383e'],
    },
  ];

  // ---------- Sprites ----------
  const px8 = (rows, pal) => `<svg viewBox="0 0 ${rows[0].length} ${rows.length}" shape-rendering="crispEdges">${grid(rows, pal, 0, 0, 1)}</svg>`;
  const BLOCK = ['gggggggg', 'gGgggGgg', 'dgdGdgdd', 'dddddddd', 'ddDdddDd', 'dddddddd', 'dDddddDd', 'dddddddd'];
  const BLOCK_PAL = { g: '#5dbb3a', G: '#4a9c2c', d: '#8b5a2b', D: '#6e4520', K: '#1a1008' };
  const cracked = BLOCK.map((row, y) => row.split('').map((c, x) =>
    [[2, 1], [3, 2], [3, 3], [4, 4], [5, 4], [5, 5], [6, 6], [1, 5], [2, 6]].some(([cx, cy]) => cx === x && cy === y) ? 'K' : c).join(''));

  const sprites = {
    sparkle: { w: 36, h: 36, svg: px8(['...Y...', '...Y...', '..YWY..', 'YYWWWYY', '..YWY..', '...Y...', '...Y...'], { Y: '#ffe14d', W: '#fff' }) },
    block: { w: 48, h: 48, svg: px8(BLOCK, BLOCK_PAL) },
    block_cracked: { w: 48, h: 48, svg: px8(cracked, BLOCK_PAL) },
  };

  // ---------- Diamond reward ----------
  const reward = px8([
    '..KKKK..',
    '.KLLTTK.',
    'KLLTTTDK',
    'KTTTTDDK',
    '.KTTDDK.',
    '..KTDK..',
    '...KK...',
    '........',
  ], { K: '#0f5e5c', L: '#d6fffd', T: '#4ee6dd', D: '#23a8a3' });

  const T = '.theme-minecraft';
  Themes.register({
    id: 'minecraft',
    name: 'Minecraft · Steve',
    style: 'blocky',
    accent: '#7ac943',
    glow: 'rgba(122, 201, 67, 0.22)',
    ring: ['#7ac943', '#f9d71c', '#5ce1e6', '#e23c3c'], // grass, gold, diamond, redstone
    fontUrl: 'https://fonts.googleapis.com/css2?family=Press+Start+2P&display=block',
    bravo: 'Bravo !',
    sound: { voice: 'square', pop: 'blip' },
    character,
    germs: MOBS.map((m) => mob(m.calm, m.pal)),
    germsScared: MOBS.map((m) => mob(m.scared, m.pal)),
    germParticles: MOBS.map((m) => m.particles),
    pop: 'creeper',      // the creeper (germ 0) swells and flashes before exploding
    tricks: ['mine'],    // theme-only tricks (mining also plays while brushing)
    teeth: [125, 114],   // where the mirror-check sparkle shines, in the 250x250 buddy box
    mouth: [125, 114],
    sprites,
    reward,

    poses: {
      idle:    { mood: 'idle' },
      blink:   { mood: 'idle', css: `${T} .eyes-open{display:none}` },
      sleepy:  { mood: 'sleepy' },
      brush_a: { mood: 'brushing', css: `${T} .brush{transform:translateX(-8px)}` },
      brush_b: { mood: 'brushing', css: `${T} .brush{transform:translateX(8px)}` },
      cheer:   { mood: 'cheer' },
      grin:    { mood: 'grin' },
      mine_a:  { mood: 'idle', cls: 'trick-mine' },
      mine_b:  { mood: 'idle', cls: 'trick-mine hit' },
      mine_brush_a: { mood: 'brushing', cls: 'trick-mine' },     // mining while brushing
      mine_brush_b: { mood: 'brushing', cls: 'trick-mine hit' },
    },

    css: `
${T} .t-clock   { font-family: 'Press Start 2P', monospace; font-size: 56px; }
${T} .t-bravo   { font-family: 'Press Start 2P', monospace; font-size: 26px; color: #ffff55; text-shadow: 3px 3px 0 #3f3f15; }

${T} .character svg { shape-rendering: crispEdges; }
${T} .cheek { opacity: .75; }
${T} .zzz   { fill: #b9c3ff; font-family: 'Press Start 2P', monospace; }

/* Hidden unless a mood shows them */
${T} .eyes-closed, ${T} .eyes-happy, ${T} .mouth-open, ${T} .mouth-grin, ${T} .brush, ${T} .foam,
${T} .zzz, ${T} .sparkles, ${T} .arms-up, ${T} .pick-up, ${T} .pick-down { display: none; }

/* Blocky movement: jump between positions instead of easing */
${T} .mood-idle .body-all     { animation: mc-bob 1.2s infinite; }
${T} .mood-idle .eyes-closed,
${T} .mood-brushing .eyes-closed { display: inline; }  /* shows through when eyes blink */
${T} .mood-idle .eyes-open,
${T} .mood-brushing .eyes-open { animation: mc-blink 4s infinite; }

${T} .mood-sleepy .eyes-open   { display: none; }
${T} .mood-sleepy .eyes-closed,
${T} .mood-sleepy .zzz         { display: inline; }
${T} .mood-sleepy .body-all    { animation: mc-bob 2.4s infinite; }
${T} .mood-sleepy .zzz text    { animation: mc-blink-z 2s infinite; }
${T} .mood-sleepy .zzz text:nth-child(2) { animation-delay: 1s; }

${T} .mood-brushing .arm-r     { display: none; }
${T} .mood-brushing .brush,
${T} .mood-brushing .foam      { display: inline; }
${T} .mood-brushing .brush     { animation: mc-scrub .3s infinite; }

${T} .mood-cheer .eyes-open, ${T} .mood-cheer .mouth, ${T} .mood-cheer .arm { display: none; }
${T} .mood-cheer .eyes-happy, ${T} .mood-cheer .mouth-open,
${T} .mood-cheer .arms-up, ${T} .mood-cheer .sparkles { display: inline; }
${T} .mood-cheer .body-all     { animation: mc-jump .7s infinite; }
${T} .mood-cheer .sparkles g   { animation: mc-blink-z .8s infinite; }
${T} .mood-cheer .sparkles g:nth-child(odd) { animation-delay: .4s; }

/* Mirror check: proud grin showing every tooth */
${T} .mood-grin .eyes-open, ${T} .mood-grin .mouth { display: none; }
${T} .mood-grin .eyes-happy, ${T} .mood-grin .mouth-grin { display: inline; }

/* Mining: pickaxe raised, swung down on .hit */
${T} .trick-mine .arm-l, ${T} .trick-mine.hit .pick-up { display: none; }
${T} .trick-mine .pick-up, ${T} .trick-mine.hit .pick-down { display: inline; }
${T} .trick-mine .body-all { animation: none; }
${T} .trick-mine .brush { animation: none; transform: translateX(-8px); } /* brush rests while he mines */

@keyframes mc-bob    { 0%, 49.9% { transform: translateY(0); } 50%, 100% { transform: translateY(-4px); } }
@keyframes mc-jump   { 0%, 49.9% { transform: translateY(0); } 50%, 100% { transform: translateY(-16px); } }
@keyframes mc-scrub  { 0%, 49.9% { transform: translateX(-8px); } 50%, 100% { transform: translateX(8px); } }
@keyframes mc-blink  { 0%, 94.9% { opacity: 1; } 95%, 100% { opacity: 0; } }
@keyframes mc-blink-z { 0%, 49.9% { opacity: 1; } 50%, 100% { opacity: 0; } }
`,
  });
})();
