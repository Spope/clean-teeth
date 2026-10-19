// "Les Royaumes de feu" theme: Glory the RainWing. RainWing scales change colour with
// mood (teal when calm, lavender when sleepy, pink when happy), and so does she.
(function () {
  'use strict';

  const sparkle = 'M0-11Q2-2 11 0Q2 2 0 11Q-2 2-11 0Q-2-2 0-11Z';

  const character = `
<svg viewBox="0 0 240 240" aria-label="Glory">
  <g class="sparkles">
    <path transform="translate(22 60)" d="${sparkle}"/>
    <path transform="translate(218 52) scale(.8)" d="${sparkle}"/>
    <path transform="translate(14 176) scale(.7)" d="${sparkle}"/>
    <path transform="translate(228 166)" d="${sparkle}"/>
  </g>

  <g class="body-all">
    <g class="wings-folded">
      <path class="wing" d="M86 150 Q58 132 46 100 Q40 128 50 150 Q58 166 88 168 Z"/>
      <path class="wing" d="M154 150 Q182 132 194 100 Q200 128 190 150 Q182 166 152 168 Z"/>
    </g>
    <g class="wings-open">
      <path class="wing" d="M92 150 L26 66 Q34 96 14 112 Q36 118 26 142 Q50 142 46 166 Q68 158 92 170 Z"/>
      <path class="wing" d="M148 150 L214 66 Q206 96 226 112 Q204 118 214 142 Q190 142 194 166 Q172 158 148 170 Z"/>
      <path class="wing-bone" d="M92 150 L26 66 M148 150 L214 66"/>
    </g>

    <path class="tail" d="M158 206 Q212 214 214 178 Q216 150 194 150 Q174 152 180 170 Q186 182 198 174"/>

    <ellipse class="scale" cx="120" cy="184" rx="50" ry="44"/>
    <ellipse class="belly" cx="120" cy="194" rx="30" ry="32"/>
    <path class="belly-lines" d="M94 180 Q120 186 146 180 M92 194 Q120 200 148 194 M96 208 Q120 214 144 208"/>
    <ellipse class="scale-dark" cx="94"  cy="226" rx="17" ry="9"/>
    <ellipse class="scale-dark" cx="146" cy="226" rx="17" ry="9"/>
    <g class="claws">
      <circle cx="84" cy="229" r="3"/><circle cx="94" cy="231" r="3"/><circle cx="104" cy="229" r="3"/>
      <circle cx="136" cy="229" r="3"/><circle cx="146" cy="231" r="3"/><circle cx="156" cy="229" r="3"/>
    </g>
    <ellipse class="scale arm" cx="76" cy="186" rx="11" ry="17" transform="rotate(25 76 186)"/>
    <ellipse class="scale arm arm-r" cx="164" cy="186" rx="11" ry="17" transform="rotate(-25 164 186)"/>

    <g class="frills">
      <ellipse cx="56"  cy="74"  rx="26" ry="10" transform="rotate(-35 56 74)"/>
      <ellipse cx="48"  cy="96"  rx="27" ry="10" transform="rotate(-8 48 96)"/>
      <ellipse cx="54"  cy="118" rx="24" ry="9"  transform="rotate(22 54 118)"/>
      <ellipse cx="184" cy="74"  rx="26" ry="10" transform="rotate(35 184 74)"/>
      <ellipse cx="192" cy="96"  rx="27" ry="10" transform="rotate(8 192 96)"/>
      <ellipse cx="186" cy="118" rx="24" ry="9"  transform="rotate(-22 186 118)"/>
      <ellipse cx="104" cy="40"  rx="7"  ry="16" transform="rotate(-25 104 40)"/>
      <ellipse cx="120" cy="34"  rx="7"  ry="18"/>
      <ellipse cx="136" cy="40"  rx="7"  ry="16" transform="rotate(25 136 40)"/>
    </g>

    <ellipse class="scale" cx="120" cy="92" rx="62" ry="54"/>
    <ellipse class="snout" cx="120" cy="120" rx="34" ry="22"/>
    <ellipse class="scale-dark" cx="110" cy="114" rx="3" ry="2"/>
    <ellipse class="scale-dark" cx="130" cy="114" rx="3" ry="2"/>

    <g class="eyes-open">
      <ellipse class="eye-white" cx="94"  cy="86" rx="16" ry="18"/>
      <ellipse class="eye-white" cx="146" cy="86" rx="16" ry="18"/>
      <circle class="iris" cx="96"  cy="88" r="11"/>
      <circle class="iris" cx="144" cy="88" r="11"/>
      <ellipse class="pupil" cx="96"  cy="88" rx="3.5" ry="8"/>
      <ellipse class="pupil" cx="144" cy="88" rx="3.5" ry="8"/>
      <circle class="eye-white" cx="100" cy="82" r="3.5"/>
      <circle class="eye-white" cx="148" cy="82" r="3.5"/>
    </g>
    <g class="eyes-closed line">
      <path d="M78 90 Q94 100 110 90"/>
      <path d="M130 90 Q146 100 162 90"/>
    </g>
    <g class="eyes-happy line">
      <path d="M78 94 Q94 76 110 94"/>
      <path d="M130 94 Q146 76 162 94"/>
    </g>

    <ellipse class="cheek" cx="74"  cy="112" rx="10" ry="6"/>
    <ellipse class="cheek" cx="166" cy="112" rx="10" ry="6"/>
    <g class="mouth">
      <path class="line" d="M102 128 Q120 140 138 128"/>
      <path class="fang" d="M108 130 l3 6 l3 -5 Z"/>
      <path class="fang" d="M126 131 l3 5 l3 -6 Z"/>
    </g>
    <g class="mouth-grin">
      <path class="mouth-inside" d="M98 124 Q120 150 142 124 Q120 130 98 124 Z"/>
      <path class="fang" d="M101 126 Q120 132 139 126 L138 131 Q120 138 102 131 Z"/>
    </g>
    <g class="mouth-fire">
      <ellipse class="mouth-inside" cx="120" cy="132" rx="11" ry="9"/>
      <ellipse cx="120" cy="134" rx="6" ry="5" fill="#ff8a3d"/>
    </g>

    <g class="foam">
      <circle cx="100" cy="140" r="5"/>
      <circle cx="92"  cy="132" r="3.5"/>
      <circle cx="106" cy="147" r="3"/>
      <circle cx="144" cy="141" r="4"/>
    </g>
    <g class="brush">
      <path class="arm-brush" d="M164 182 L184 138"/>
      <rect class="brush-handle" x="126" y="124" width="60" height="9" rx="4.5"/>
      <rect class="brush-head" x="110" y="120" width="22" height="14" rx="4"/>
      <rect class="bristles" x="112" y="113" width="18" height="8" rx="2"/>
      <circle class="scale" cx="186" cy="130" r="10"/>
    </g>
  </g>

  <g class="zzz">
    <text x="178" y="40" font-size="20">z</text>
    <text x="196" y="22" font-size="26">Z</text>
  </g>
</svg>`;

  // Grumpy germ blobs (the original germ design), in jungle colours.
  // Scared: wide eyes, no frown, "o" mouth.
  function germ(color, scared = false) {
    let bumps = '';
    for (let k = 0; k < 8; k++) {
      const a = (k / 8) * Math.PI * 2;
      bumps += `<circle cx="${(15 * Math.cos(a)).toFixed(1)}" cy="${(15 * Math.sin(a)).toFixed(1)}" r="5.5"/>`;
    }
    const face = scared
      ? `<circle cx="-5.5" cy="-3" r="5.5" fill="#fff"/><circle cx="5.5" cy="-3" r="5.5" fill="#fff"/>
         <circle cx="-5.5" cy="-3" r="1.8" fill="#1e1a33"/><circle cx="5.5" cy="-3" r="1.8" fill="#1e1a33"/>
         <circle cy="7" r="2.8" fill="#1e1a33"/>`
      : `<circle cx="-5.5" cy="-3" r="4.5" fill="#fff"/><circle cx="5.5" cy="-3" r="4.5" fill="#fff"/>
         <circle cx="-4.5" cy="-2" r="2.2" fill="#1e1a33"/><circle cx="6.5" cy="-2" r="2.2" fill="#1e1a33"/>
         <path d="M-10 -10 L-2 -7 M10 -10 L2 -7" stroke="#1e1a33" stroke-width="2" stroke-linecap="round"/>
         <path d="M-5 7 Q0 4 5 7" fill="none" stroke="#1e1a33" stroke-width="2" stroke-linecap="round"/>`;
    return `<g fill="${color}">${bumps}<circle r="15"/>${face}</g>`;
  }
  const GERM_COLORS = ['#8a5cff', '#ff5fa2', '#9be15d', '#ff9f43'];

  const sprites = {
    sparkle: { w: 36, h: 36, svg: `<svg viewBox="-12 -12 24 24">
      <path d="M0-11Q1.5-1.5 11 0Q1.5 1.5 0 11Q-1.5 1.5-11 0Q-1.5-1.5 0-11Z" fill="#fff6c2"/>
      <circle r="2.5" fill="#fff"/></svg>` },
    fireball: { w: 40, h: 40, svg: `<svg viewBox="-20 -20 40 40">
      <path d="M-16 0 Q-14 -12 -2 -15 Q-6 -8 2 -10 Q14 -12 16 0 Q14 14 0 16 Q-14 14 -16 0Z" fill="#ff5a1c"/>
      <circle r="10" fill="#ffa23d"/><circle r="5" fill="#ffe066"/></svg>` },
  };

  // RainWing rainbow: 5 scale palettes cycled by the "rainbow" trick
  const RAINBOW = [
    ['#ff5a6e', '#d93a50', '#ff8a98', '#ffe3e7', '#ffc2ca', '#ffd23f', '#ff9f43', '#c93a4e'],
    ['#ff9a3c', '#e07a1f', '#ffb870', '#fff0dc', '#ffd9b0', '#ff5a6e', '#ffd23f', '#d0741f'],
    ['#ffd23f', '#e0b21f', '#ffe27a', '#fffbe0', '#fff0a8', '#3fbfa0', '#ff9a3c', '#d0a81f'],
    ['#4aa8ff', '#2f86dd', '#86c4ff', '#e3f1ff', '#bcdcff', '#ffd23f', '#a66bff', '#2f78c9'],
    ['#a66bff', '#844ddd', '#c49bff', '#f0e6ff', '#dcc8ff', '#7ce3ff', '#ff7ac2', '#7a44c9'],
  ];
  const rainbowCss = RAINBOW.map(([sc, dark, snout, belly, line, frill, frill2, wing], i) =>
    `.theme-dragon .character.rainbow-${i + 1} { --scale: ${sc}; --scale-dark: ${dark}; --snout: ${snout}; --belly: ${belly};
      --belly-line: ${line}; --frill: ${frill}; --frill-2: ${frill2}; --wing: ${wing}; }`).join('\n');

  // Treasure gem (dragons love treasure)
  const reward = `<svg viewBox="0 0 24 24">
    <path d="M6 4h12l4 5-10 12L2 9z" fill="#ff4d6d"/>
    <path d="M6 4l3 5h6l3-5z" fill="#ff8fa3"/>
    <path d="M2 9h7l3 12z" fill="#e0304f"/>
    <path d="M15 9h7L12 21z" fill="#c9203f"/>
    <path d="M9 9h6l-3 12z" fill="#ff6b85"/>
    <path d="M7.5 5.5l1.2 2" stroke="#fff" stroke-width="1.2" stroke-linecap="round" opacity=".8"/>
  </svg>`;

  const T = '.theme-dragon';
  Themes.register({
    id: 'dragon',
    name: 'Royaumes de feu · Glory',
    style: 'round',
    accent: '#3fbfa0',
    glow: 'rgba(63, 191, 160, 0.25)',
    ring: ['#3fbfa0', '#ffd23f', '#ff7ac2', '#6fa8ff'],
    fontUrl: 'https://fonts.googleapis.com/css2?family=Cinzel+Decorative:wght@700&family=Metamorphous&display=block',
    bravo: 'Bravo !',
    sound: { voice: 'triangle', pop: 'whoosh' },
    character,
    germs: GERM_COLORS.map((c) => germ(c)),
    germsScared: GERM_COLORS.map((c) => germ(c, true)),
    germParticles: GERM_COLORS.map((c) => [c, '#ff8a3d', '#ffe066']), // burnt to embers
    pop: 'fireball',             // Glory breathes a fireball at each germ
    tricks: ['rainbow', 'fly'],  // theme-only tricks, start screen only
    teeth: [125, 134],           // in the 250x250 buddy box
    mouth: [125, 137],
    sprites,
    reward,

    poses: {
      idle:    { mood: 'idle' },
      blink:   { mood: 'idle', css: `${T} .eyes-open{display:none} ${T} .eyes-closed{display:inline}` },
      sleepy:  { mood: 'sleepy' },
      brush_a: { mood: 'brushing', css: `${T} .brush{transform:translateX(-9px)}` },
      brush_b: { mood: 'brushing', css: `${T} .brush{transform:translateX(9px)}` },
      cheer:   { mood: 'cheer' },
      grin:    { mood: 'grin' },
      fire:    { mood: 'fire' },
      fly_a:   { mood: 'idle', cls: 'trick-fly' },
      fly_b:   { mood: 'idle', cls: 'trick-fly flap' },
      rainbow_1: { mood: 'idle', cls: 'rainbow-1' },
      rainbow_2: { mood: 'idle', cls: 'rainbow-2' },
      rainbow_3: { mood: 'idle', cls: 'rainbow-3' },
      rainbow_4: { mood: 'idle', cls: 'rainbow-4' },
      rainbow_5: { mood: 'idle', cls: 'rainbow-5' },
    },

    css: `
${T} .t-counter { font-family: 'Metamorphous', serif; font-size: 26px; color: #ffd166; text-shadow: 0 2px 0 #7a3b00; }
${T} .t-bravo {
  font-family: 'Cinzel Decorative', serif; font-weight: 700; font-size: 40px;
  background: linear-gradient(#fff1a8, #ffc23d 45%, #ff7b1c);
  -webkit-background-clip: text; background-clip: text; color: transparent;
  filter: drop-shadow(0 3px 0 #7a1f00);
}

/* Scale colours by mood (RainWing!) */
${T} .character {
  --scale: #3fbfa0; --scale-dark: #2a9a80; --snout: #6fd6bd; --belly: #d8f5c4; --belly-line: #b5e3a0;
  --frill: #ffcf4a; --frill-2: #ff9f43; --wing: #2f8f78; --iris: #2f9e6b; --ink: #1e1a33;
}
${T} .character.mood-sleepy {
  --scale: #8a8fe0; --scale-dark: #6c70c4; --snout: #a9adf2; --belly: #e4e2ff; --belly-line: #c9c6f5;
  --frill: #c3b5ff; --frill-2: #9a8cf0; --wing: #6a6ec0;
}
${T} .character.mood-cheer {
  --scale: #ff7ac2; --scale-dark: #e0559f; --snout: #ff9fd3; --belly: #ffe3f1; --belly-line: #ffc2df;
  --frill: #ffe066; --frill-2: #7ce3ff; --wing: #d94d96;
}

${T} .scale      { fill: var(--scale); }
${T} .scale-dark { fill: var(--scale-dark); }
${T} .snout      { fill: var(--snout); }
${T} .belly      { fill: var(--belly); }
${T} .belly-lines { fill: none; stroke: var(--belly-line); stroke-width: 3; stroke-linecap: round; }
${T} .tail       { fill: none; stroke: var(--scale-dark); stroke-width: 14; stroke-linecap: round; }
${T} .wing       { fill: var(--wing); stroke: var(--scale-dark); stroke-width: 3; stroke-linejoin: round; }
${T} .wing-bone  { fill: none; stroke: var(--scale-dark); stroke-width: 5; stroke-linecap: round; }
${T} .frills ellipse { fill: var(--frill); stroke: var(--frill-2); stroke-width: 3; }
${T} .claws      { fill: #fff6e0; }
${T} .eye-white  { fill: #fff; }
${T} .iris       { fill: var(--iris); }
${T} .pupil      { fill: var(--ink); }
${T} .line       { fill: none; stroke: var(--ink); stroke-width: 4; stroke-linecap: round; }
${T} .fang       { fill: #fff; }
${T} .mouth-inside { fill: #4a1f3d; }
${T} .cheek      { fill: #ff6f9c; opacity: .7; }
${T} .foam       { fill: #fff; opacity: .9; }
${T} .arm-brush  { fill: none; stroke: var(--scale); stroke-width: 18; stroke-linecap: round; }
${T} .brush-handle { fill: #4fc3f7; }
${T} .brush-head   { fill: #fff; }
${T} .bristles     { fill: #c8f0ff; }
${T} .sparkles   { fill: #ffd166; }
${T} .zzz        { fill: #c3b5ff; font-family: 'Cinzel Decorative', serif; font-weight: 700; }

/* Hidden unless a mood shows them */
${T} .eyes-closed, ${T} .eyes-happy, ${T} .brush, ${T} .foam, ${T} .zzz,
${T} .sparkles, ${T} .wings-open, ${T} .mouth-grin, ${T} .mouth-fire { display: none; }

${T} .body-all, ${T} .eyes-open, ${T} .brush, ${T} .foam, ${T} .sparkles path, ${T} .zzz text {
  transform-box: fill-box; transform-origin: center;
}
${T} .body-all { transform-origin: 50% 100%; }

${T} .mood-idle .body-all  { animation: rw-bob 2s ease-in-out infinite; }
${T} .mood-idle .eyes-open { animation: rw-blink 4s infinite; }

${T} .mood-sleepy .eyes-open   { display: none; }
${T} .mood-sleepy .eyes-closed,
${T} .mood-sleepy .zzz         { display: inline; }
${T} .mood-sleepy .body-all    { animation: rw-breathe 3.2s ease-in-out infinite; }
${T} .mood-sleepy .zzz text    { animation: rw-zzz 2.4s ease-in-out infinite; }
${T} .mood-sleepy .zzz text:nth-child(2) { animation-delay: .6s; }

${T} .mood-brushing .arm-r { display: none; }
${T} .mood-brushing .brush,
${T} .mood-brushing .foam  { display: inline; }
${T} .mood-brushing .brush { animation: rw-scrub .28s ease-in-out infinite alternate; }
${T} .mood-brushing .foam  { animation: rw-foam .5s ease-in-out infinite alternate; }
${T} .mood-brushing .body-all  { animation: rw-wiggle .56s ease-in-out infinite alternate; }
${T} .mood-brushing .eyes-open { animation: rw-blink 3s infinite; }

${T} .mood-cheer .eyes-open, ${T} .mood-cheer .wings-folded { display: none; }
${T} .mood-cheer .eyes-happy, ${T} .mood-cheer .sparkles, ${T} .mood-cheer .wings-open { display: inline; }
${T} .mood-cheer .body-all { animation: rw-jump .7s ease-in-out infinite; }
${T} .mood-cheer .sparkles path { animation: rw-twinkle 1s ease-in-out infinite alternate; }
${T} .mood-cheer .sparkles path:nth-child(2) { animation-delay: .3s; }
${T} .mood-cheer .sparkles path:nth-child(3) { animation-delay: .6s; }
${T} .mood-cheer .sparkles path:nth-child(4) { animation-delay: .15s; }

/* Mirror check: proud toothy grin */
${T} .mood-grin .eyes-open, ${T} .mood-grin .mouth { display: none; }
${T} .mood-grin .eyes-happy, ${T} .mood-grin .mouth-grin { display: inline; }

/* Breathing fire at a germ */
${T} .mood-fire .mouth { display: none; }
${T} .mood-fire .mouth-fire { display: inline; }

/* Flying: wings spread and flapping */
${T} .trick-fly .wings-folded { display: none; }
${T} .trick-fly .wings-open { display: inline; transform-box: fill-box; transform-origin: 50% 100%; }
${T} .trick-fly .wings-open { animation: rw-flap .3s infinite; }
${T} .trick-fly.flap .wings-open { transform: scaleY(.55); }
@keyframes rw-flap { 0%, 49.9% { transform: none; } 50%, 100% { transform: scaleY(.55); } }

${rainbowCss}

@keyframes rw-bob     { 50% { transform: translateY(-5px); } }
@keyframes rw-blink   { 0%, 93%, 100% { transform: scaleY(1); } 96% { transform: scaleY(.1); } }
@keyframes rw-breathe { 50% { transform: scale(1.03, .98); } }
@keyframes rw-zzz     { 0% { opacity: 0; transform: translateY(6px); } 50% { opacity: 1; } 100% { opacity: 0; transform: translateY(-10px); } }
@keyframes rw-scrub   { from { transform: translateX(-9px); } to { transform: translateX(9px); } }
@keyframes rw-foam    { from { transform: scale(.85); } to { transform: scale(1.1); } }
@keyframes rw-wiggle  { from { transform: rotate(-2deg); } to { transform: rotate(2deg); } }
@keyframes rw-jump    { 0%, 100% { transform: translateY(0) scale(1.05, .95); } 45% { transform: translateY(-20px) scale(.97, 1.03); } }
@keyframes rw-twinkle { from { transform: scale(.4) rotate(0); opacity: .4; } to { transform: scale(1.1) rotate(45deg); opacity: 1; } }
`,
  });
})();
