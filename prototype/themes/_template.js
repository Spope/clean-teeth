// Starting point for a new theme (see docs/themes.md): a round, blue blob with a sprout.
// Copy this file to prototype/themes/<id>.js, set ID and NAME below, add a <script> tag for
// it in prototype/index.html, then make it yours. Everything a theme needs is here, nothing more:
// the buddy in every mood, 4 germs, the sparkle sprite, the reward, colours, fonts and sounds.
// For pixel art, start from minecraft.js instead; for extra tricks, see dragon.js.
// Files starting with _ aren't themes: the export skips them.
(function () {
  'use strict';

  const ID = 'template';           // lowercase letters and digits, the same as the file name
  const NAME = 'Template · Blobby'; // shown in the dev panel's theme picker
  const T = `.theme-${ID}`;        // every CSS rule starts with this

  // ---------- The buddy, facing us, in a 250x250 box with his feet on the bottom edge ----------
  // The CSS below shows and hides the parts for each mood (classes in quotes).
  const star = 'M0-11Q2-2 11 0Q2 2 0 11Q-2 2-11 0Q-2-2 0-11Z';
  const limb = (d) => `<path class="limb-edge" d="${d}"/><path class="limb" d="${d}"/>`;

  const character = `
<svg viewBox="0 0 250 250" aria-label="${NAME}">
  <g class="sparkles">
    <path transform="translate(24 70)" d="${star}"/>
    <path transform="translate(222 60) scale(.8)" d="${star}"/>
    <path transform="translate(18 180) scale(.7)" d="${star}"/>
    <path transform="translate(230 170)" d="${star}"/>
  </g>

  <g class="body-all">
    <g class="arms-down">
      <g class="arm-l">${limb('M56 150 L34 192')}</g>
      <g class="arm-r">${limb('M194 150 L216 192')}</g>
    </g>
    <g class="arms-up">${limb('M60 132 L30 92')}${limb('M190 132 L220 92')}</g>

    <ellipse class="feet" cx="96" cy="238" rx="22" ry="11"/>
    <ellipse class="feet" cx="154" cy="238" rx="22" ry="11"/>
    <path class="sprout" d="M125 62 Q121 46 128 34"/>
    <path class="leaf" d="M128 34 Q140 20 154 26 Q146 40 128 34 Z"/>
    <ellipse class="body" cx="125" cy="146" rx="82" ry="88"/>
    <ellipse class="belly" cx="125" cy="188" rx="52" ry="40"/>
    <ellipse class="cheek" cx="76" cy="134" rx="11" ry="7"/>
    <ellipse class="cheek" cx="174" cy="134" rx="11" ry="7"/>

    <g class="eyes-open">
      <ellipse class="eye-white" cx="98" cy="112" rx="14" ry="16"/>
      <ellipse class="eye-white" cx="152" cy="112" rx="14" ry="16"/>
      <circle class="pupil" cx="100" cy="115" r="8"/>
      <circle class="pupil" cx="150" cy="115" r="8"/>
      <circle class="glint" cx="104" cy="110" r="3"/>
      <circle class="glint" cx="154" cy="110" r="3"/>
    </g>
    <g class="eyes-closed line"><path d="M84 116 Q98 124 112 116 M138 116 Q152 124 166 116"/></g>
    <g class="eyes-happy line"><path d="M84 118 Q98 102 112 118 M138 118 Q152 102 166 118"/></g>

    <path class="mouth line" d="M113 138 Q125 148 137 138"/>
    <g class="mouth-open">
      <path class="mouth-inside" d="M106 136 Q125 166 144 136 Z"/>
      <ellipse class="tongue" cx="125" cy="152" rx="9" ry="5"/>
    </g>
    <g class="mouth-grin"><!-- the mirror check sparkles on these teeth: \`teeth\` below -->
      <path class="mouth-inside" d="M98 134 Q125 164 152 134 Z"/>
      <path class="tooth" d="M101 135 L149 135 Q147 143 125 144 Q103 143 101 135 Z"/>
    </g>

    <g class="foam">
      <circle cx="104" cy="148" r="5"/><circle cx="96" cy="140" r="3.5"/>
      <circle cx="112" cy="154" r="3"/><circle cx="148" cy="146" r="4"/>
    </g>
    <g class="brush"><!-- held by the arm on the right of the picture, so .arm-r hides meanwhile -->
      ${limb('M196 160 Q190 140 170 140')}
      <rect class="brush-handle" x="128" y="137" width="44" height="7" rx="3.5"/>
      <rect class="brush-head" x="106" y="134" width="26" height="12" rx="4"/>
      <rect class="bristles" x="108" y="128" width="22" height="7" rx="2"/>
      <circle class="hand" cx="172" cy="140" r="9"/>
    </g>
  </g>

  <g class="zzz">
    <text x="184" y="50" font-size="20">z</text>
    <text x="202" y="30" font-size="26">Z</text>
  </g>
</svg>`;

  // ---------- Germs: centred on (0,0), about 42px across; scared: same body, startled face ----------
  const INK = '#1d2540';
  const GERMS = [
    { color: '#7ed957', dark: '#4ea83a', bumps: 8 },
    { color: '#b07cff', dark: '#7d4fd6', bumps: 6 },
    { color: '#ffa94d', dark: '#e0761f', bumps: 9 },
    { color: '#ff6fa5', dark: '#d9437c', bumps: 7 },
  ];
  function germ({ color, dark, bumps }, scared) {
    let body = '';
    for (let i = 0; i < bumps; i++) {
      const a = (i / bumps) * 2 * Math.PI;
      body += `<circle cx="${(16 * Math.cos(a)).toFixed(1)}" cy="${(16 * Math.sin(a)).toFixed(1)}" r="4.5" fill="${dark}"/>`;
    }
    const face = scared
      ? `<circle cx="-5" cy="-3" r="4.5" fill="#fff"/><circle cx="5" cy="-3" r="4.5" fill="#fff"/>
         <circle cx="-5" cy="-3" r="1.5" fill="${INK}"/><circle cx="5" cy="-3" r="1.5" fill="${INK}"/>
         <ellipse cy="7" rx="2.5" ry="3" fill="${INK}"/>`
      : `<circle cx="-5" cy="-2" r="3.5" fill="#fff"/><circle cx="5" cy="-2" r="3.5" fill="#fff"/>
         <circle cx="-4.5" cy="-1.5" r="1.8" fill="${INK}"/><circle cx="4.5" cy="-1.5" r="1.8" fill="${INK}"/>
         <path d="M-9 -8 L-2 -5 M9 -8 L2 -5 M-5 8 Q0 4 5 8" fill="none" stroke="${INK}" stroke-width="2" stroke-linecap="round"/>`;
    return `<g>${body}<circle r="15" fill="${color}"/>${face}</g>`;
  }

  // ---------- Sprites and reward ----------
  const sprites = {
    // The mirror check's glints (every theme needs it)
    sparkle: { w: 36, h: 36, svg: `<svg viewBox="-12 -12 24 24">
      <path d="${star}" fill="#fff6c2"/><circle r="2.5" fill="#fff"/></svg>` },
  };

  // A gold star, drawn 32x32
  const reward = `<svg viewBox="0 0 24 24">
    <path d="M12 2l2.9 6.3 6.9.8-5.1 4.7 1.4 6.8L12 17.2 5.9 20.6l1.4-6.8L2.2 9.1l6.9-.8z"
      fill="#ffd166" stroke="#e09a1a" stroke-width="1.2" stroke-linejoin="round"/>
    <path d="M9.2 9.6l1-2" stroke="#fff" stroke-width="1.2" stroke-linecap="round" opacity=".8"/>
  </svg>`;

  Themes.register({
    id: ID,
    name: NAME,
    style: 'round',                    // 'blocky' for pixel art
    accent: '#7cc8ff',                 // start button
    glow: 'rgba(124, 200, 255, 0.22)', // behind the buddy (prototype only)
    ring: ['#7cc8ff', '#ffd166', '#ff8fab', '#8be38b'], // one colour per brushing zone
    fontUrl: 'https://fonts.googleapis.com/css2?family=Fredoka:wght@600&display=block',
    bravo: 'Bravo !',
    sound: { voice: 'triangle', pop: 'blip' },
    character,
    germs: GERMS.map((g) => germ(g, false)),
    germsScared: GERMS.map((g) => germ(g, true)),
    germParticles: GERMS.map((g) => [g.color, g.dark, '#ffffff']),
    pop: 'burst',
    tricks: [],      // theme-only tricks: 'mine', 'xp', 'rainbow', 'fly' (see docs/themes.md)
    sway: true,      // rocks while brushing on the device too (remove for pixel art)
    teeth: [125, 139], // where the mirror check sparkles, in the 250x250 buddy box
    sprites,
    reward,

    // Still frames for the device: a mood, plus extra classes (cls) or CSS (css)
    poses: {
      idle:    { mood: 'idle' },
      blink:   { mood: 'idle', css: `${T} .eyes-open{display:none} ${T} .eyes-closed{display:inline}` },
      sleepy:  { mood: 'sleepy' },
      brush_a: { mood: 'brushing', css: `${T} .brush{transform:translateX(-8px)}` },
      brush_b: { mood: 'brushing', css: `${T} .brush{transform:translateX(8px)}` },
      cheer:   { mood: 'cheer' },
      grin:    { mood: 'grin' },
    },

    css: `
${T} .t-clock { font-family: 'Fredoka', sans-serif; font-weight: 600; font-size: 80px; }
${T} .t-bravo { font-family: 'Fredoka', sans-serif; font-weight: 600; font-size: 44px;
  color: #ffd166; text-shadow: 0 3px 0 #b86b00; }

/* Colours (like Glory's, they could change with the mood: .character.mood-sleepy {...}) */
${T} .character { --body: #7cc8ff; --body-dark: #3f8fd6; --belly: #e6f5ff; --cheek: #ff9ab8;
  --ink: ${INK}; --sprout: #6fd16f; }
${T} .body       { fill: var(--body); stroke: var(--body-dark); stroke-width: 3; }
${T} .belly      { fill: var(--belly); }
${T} .feet       { fill: var(--body-dark); }
${T} .limb-edge  { fill: none; stroke: var(--body-dark); stroke-width: 24; stroke-linecap: round; }
${T} .limb       { fill: none; stroke: var(--body); stroke-width: 18; stroke-linecap: round; }
${T} .hand       { fill: var(--body); stroke: var(--body-dark); stroke-width: 3; }
${T} .sprout     { fill: none; stroke: var(--sprout); stroke-width: 4; stroke-linecap: round; }
${T} .leaf       { fill: var(--sprout); }
${T} .cheek      { fill: var(--cheek); opacity: .8; }
${T} .eye-white, ${T} .glint, ${T} .tooth, ${T} .brush-head { fill: #fff; }
${T} .pupil      { fill: var(--ink); }
${T} .line       { fill: none; stroke: var(--ink); stroke-width: 3.5; stroke-linecap: round; }
${T} .mouth-inside { fill: #5a2340; }
${T} .tongue     { fill: #ff7a9a; }
${T} .foam       { fill: #fff; opacity: .9; }
${T} .brush-handle { fill: #ff8fab; }
${T} .bristles   { fill: #c8f0ff; }
${T} .sparkles   { fill: #ffd166; }
${T} .zzz        { fill: #c3b5ff; font-family: 'Fredoka', sans-serif; font-weight: 600; }

/* Hidden unless a mood shows them */
${T} .eyes-closed, ${T} .eyes-happy, ${T} .mouth-open, ${T} .mouth-grin, ${T} .brush, ${T} .foam,
${T} .zzz, ${T} .sparkles, ${T} .arms-up { display: none; }

${T} .body-all, ${T} .eyes-open, ${T} .brush, ${T} .foam, ${T} .sparkles path, ${T} .zzz text {
  transform-box: fill-box; transform-origin: center;
}
${T} .body-all { transform-origin: 50% 100%; }

/* The animations are for the prototype: the device moves the still poses itself */
${T} .mood-idle .body-all { animation: ${ID}-bob 2.4s ease-in-out infinite; }
${T} .mood-idle .eyes-open, ${T} .mood-brushing .eyes-open { animation: ${ID}-blink 4s infinite; }

${T} .mood-sleepy .eyes-open { display: none; }
${T} .mood-sleepy .eyes-closed, ${T} .mood-sleepy .zzz { display: inline; }
${T} .mood-sleepy .body-all { animation: ${ID}-breathe 3.2s ease-in-out infinite; }
${T} .mood-sleepy .zzz text { animation: ${ID}-zzz 2.4s ease-in-out infinite; }
${T} .mood-sleepy .zzz text:nth-child(2) { animation-delay: .6s; }

/* Brushing: the right arm holds the toothbrush */
${T} .mood-brushing .arm-r { display: none; }
${T} .mood-brushing .brush, ${T} .mood-brushing .foam { display: inline; }
${T} .mood-brushing .brush { animation: ${ID}-scrub .28s ease-in-out infinite alternate; }
${T} .mood-brushing .foam { animation: ${ID}-foam .5s ease-in-out infinite alternate; }
${T} .mood-brushing .body-all { animation: ${ID}-sway .56s ease-in-out infinite alternate; }

/* The end: arms up, happy eyes, sparkles */
${T} .mood-cheer .eyes-open, ${T} .mood-cheer .mouth, ${T} .mood-cheer .arms-down { display: none; }
${T} .mood-cheer .eyes-happy, ${T} .mood-cheer .mouth-open, ${T} .mood-cheer .arms-up,
${T} .mood-cheer .sparkles { display: inline; }
${T} .mood-cheer .body-all { animation: ${ID}-jump .7s ease-in-out infinite; }
${T} .mood-cheer .sparkles path { animation: ${ID}-twinkle 1s ease-in-out infinite alternate; }
${T} .mood-cheer .sparkles path:nth-child(even) { animation-delay: .4s; }

/* Mirror check: a proud grin showing the teeth */
${T} .mood-grin .eyes-open, ${T} .mood-grin .mouth { display: none; }
${T} .mood-grin .eyes-happy, ${T} .mood-grin .mouth-grin { display: inline; }

@keyframes ${ID}-bob     { 50% { transform: translateY(-4px); } }
@keyframes ${ID}-blink   { 0%, 93%, 100% { transform: scaleY(1); } 96% { transform: scaleY(.1); } }
@keyframes ${ID}-breathe { 50% { transform: scale(1.03, .98); } }
@keyframes ${ID}-zzz     { 0% { opacity: 0; transform: translateY(6px); } 50% { opacity: 1; } 100% { opacity: 0; transform: translateY(-10px); } }
@keyframes ${ID}-scrub   { from { transform: translateX(-8px); } to { transform: translateX(8px); } }
@keyframes ${ID}-foam    { from { transform: scale(.85); } to { transform: scale(1.1); } }
@keyframes ${ID}-sway    { from { transform: rotate(-2deg); } to { transform: rotate(2deg); } }
@keyframes ${ID}-jump    { 0%, 100% { transform: translateY(0) scale(1.05, .95); } 45% { transform: translateY(-20px) scale(.97, 1.03); } }
@keyframes ${ID}-twinkle { from { transform: scale(.4) rotate(0); opacity: .4; } to { transform: scale(1.1) rotate(45deg); opacity: 1; } }
`,
  });
})();
