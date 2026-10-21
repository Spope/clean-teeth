// "Les Royaumes de feu" theme: Glory the RainWing. RainWing scales change colour with
// mood (teal when calm, lavender when sleepy, pink when happy), and so does she.
(function () {
  'use strict';

  const sparkle = 'M0-11Q2-2 11 0Q2 2 0 11Q-2 2-11 0Q-2-2 0-11Z';

  // Drawn in the style of the graphic novels rather than as a baby dragon: horned head on a
  // neck, narrow slit-pupil eyes under brow ridges, pointed ruff, long prehensile tail.
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
      <path class="wing" d="M100 146 L66 92 Q60 124 50 150 Q60 176 84 190 L102 172 Z"/>
      <path class="wing" d="M140 146 L174 92 Q180 124 190 150 Q180 176 156 190 L138 172 Z"/>
      <path class="wing-bone" d="M100 146 L66 92 M140 146 L174 92"/>
    </g>
    <g class="wings-open">
      <path class="wing" d="M98 148 L26 62 Q34 94 14 110 Q36 116 26 140 Q50 140 46 164 Q68 156 96 172 Z"/>
      <path class="wing" d="M142 148 L214 62 Q206 94 226 110 Q204 116 214 140 Q190 140 194 164 Q172 156 144 172 Z"/>
      <path class="wing-bone" d="M98 148 L26 62 M142 148 L214 62 M74 104 L36 116 M166 104 L204 116"/>
    </g>

    <path class="tail" d="M146 214 Q196 226 214 198 Q230 170 210 152 Q194 140 184 154 Q178 166 192 168"/>

    <ellipse class="scale" cx="88" cy="204" rx="17" ry="22"/>
    <ellipse class="scale" cx="152" cy="204" rx="17" ry="22"/>
    <path class="scale" d="M102 104 L98 146 Q78 176 92 206 Q120 222 148 206 Q162 176 142 146 L138 104 Z"/>
    <path class="belly" d="M110 108 Q120 112 130 108 L134 146 Q144 180 120 212 Q96 180 106 146 Z"/>
    <path class="belly-lines" d="M110 120 H130 M109 132 H131 M108 144 H132 M104 158 H136 M102 172 H138 M104 186 H136 M110 200 H130"/>
    <ellipse class="scale-dark" cx="84" cy="226" rx="17" ry="7"/>
    <ellipse class="scale-dark" cx="156" cy="226" rx="17" ry="7"/>
    <g class="claws">
      <path d="M70 226 l-3 7 l7 -3Z M80 228 l-1 7 l6 -4Z M90 228 l1 7 l4 -6Z"/>
      <path d="M170 226 l3 7 l-7 -3Z M160 228 l1 7 l-6 -4Z M150 228 l-1 7 l-4 -6Z"/>
    </g>
    <path class="scale arm" d="M100 150 Q88 166 96 186 Q102 190 106 184 Q100 168 108 156 Z"/>
    <path class="scale arm arm-r" d="M140 150 Q152 166 144 186 Q138 190 134 184 Q140 168 132 156 Z"/>
    <path class="texture" d="M92 176 l4 -4 l4 4 M146 176 l-4 -4 l-4 4 M84 200 l4 -4 l4 4 M156 200 l-4 -4 l-4 4"/>

    <g class="frills">
      <path d="M86 60 Q62 34 38 26 Q56 50 84 74 Z"/>
      <path d="M82 72 Q52 66 26 72 Q52 84 82 86 Z"/>
      <path d="M84 86 Q58 98 40 118 Q66 108 90 98 Z"/>
      <path d="M154 60 Q178 34 202 26 Q184 50 156 74 Z"/>
      <path d="M158 72 Q188 66 214 72 Q188 84 158 86 Z"/>
      <path d="M156 86 Q182 98 200 118 Q174 108 150 98 Z"/>
    </g>
    <g class="horns">
      <path d="M96 48 Q86 24 70 12 Q90 22 106 44 Z"/>
      <path d="M144 48 Q154 24 170 12 Q150 22 134 44 Z"/>
    </g>

    <path class="scale" d="M80 70 Q80 38 120 36 Q160 38 160 70 Q160 90 146 102 Q138 120 120 122 Q102 120 94 102 Q80 90 80 70 Z"/>
    <path class="snout" d="M100 96 Q120 88 140 96 Q138 116 120 120 Q102 116 100 96 Z"/>
    <path class="texture" d="M112 46 l8 6 l8 -6 M114 56 l6 4 l6 -4"/>
    <ellipse class="scale-dark" cx="113" cy="101" rx="2.5" ry="1.5" transform="rotate(-20 113 101)"/>
    <ellipse class="scale-dark" cx="127" cy="101" rx="2.5" ry="1.5" transform="rotate(20 127 101)"/>

    <g class="eyes-open">
      <path class="eye-white" d="M88 76 Q98 64 114 72 Q104 84 88 76 Z"/>
      <path class="eye-white" d="M152 76 Q142 64 126 72 Q136 84 152 76 Z"/>
      <ellipse class="iris" cx="102" cy="73.5" rx="5.5" ry="6"/>
      <ellipse class="iris" cx="138" cy="73.5" rx="5.5" ry="6"/>
      <ellipse class="pupil" cx="102" cy="73.5" rx="1.6" ry="5"/>
      <ellipse class="pupil" cx="138" cy="73.5" rx="1.6" ry="5"/>
      <circle class="glint" cx="104" cy="71" r="1.4"/>
      <circle class="glint" cx="140" cy="71" r="1.4"/>
    </g>
    <g class="eyes-closed line">
      <path d="M88 76 Q100 80 114 74"/>
      <path d="M152 76 Q140 80 126 74"/>
    </g>
    <g class="eyes-happy line">
      <path d="M88 78 Q100 68 114 76"/>
      <path d="M152 78 Q140 68 126 76"/>
    </g>
    <path class="brow" d="M86 66 Q100 58 115 63 M154 66 Q140 58 125 63"/>

    <g class="mouth">
      <path class="line thin" d="M104 110 Q122 116 137 107"/>
      <path class="fang" d="M107 111 l2 5 l2 -4 Z"/>
      <path class="fang" d="M129 112 l2 4 l2 -5 Z"/>
    </g>
    <g class="mouth-grin">
      <path class="mouth-inside" d="M100 106 Q120 124 140 106 Q120 112 100 106 Z"/>
      <path class="fang" d="M103 107 Q120 112 137 107 L135 111 Q120 116 105 111 Z"/>
    </g>
    <g class="mouth-spit">
      <ellipse class="mouth-inside" cx="120" cy="112" rx="9" ry="7"/>
      <path class="fang" d="M113 106 l2 6 l2 -5 Z"/>
      <path class="fang" d="M123 107 l2 5 l2 -6 Z"/>
      <circle cx="120" cy="114" r="3.5" fill="#1b1426" stroke="#5dd94a" stroke-width="1.5"/>
    </g>

    <g class="foam">
      <circle cx="102" cy="118" r="4"/>
      <circle cx="95"  cy="112" r="3"/>
      <circle cx="108" cy="124" r="2.5"/>
      <circle cx="140" cy="119" r="3.5"/>
    </g>
    <g class="brush">
      <path class="arm-brush" d="M138 154 Q170 150 182 118"/>
      <rect class="brush-handle" x="126" y="106" width="60" height="8" rx="4"/>
      <rect class="brush-head" x="110" y="103" width="22" height="13" rx="4"/>
      <rect class="bristles" x="112" y="97" width="18" height="7" rx="2"/>
      <circle class="scale" cx="184" cy="112" r="8"/>
    </g>
  </g>

  <g class="zzz">
    <text x="182" y="40" font-size="20">z</text>
    <text x="200" y="22" font-size="26">Z</text>
  </g>
</svg>`;

  // Jungle bugs from Glory's rainforest: mosquito, beetle, caterpillar, spider.
  // Scared: wide eyes, no frown, "o" mouth.
  const INK = '#1e1a33';
  // Small glowing eyes, angled and narrowed; scared: wide and round, tiny pupils.
  function face(x, y, s, scared) {
    const f = scared
      ? `<circle cx="-5" cy="-2" r="4" fill="#ffe066"/><circle cx="5" cy="-2" r="4" fill="#ffe066"/>
         <circle cx="-5" cy="-2" r="1.2" fill="${INK}"/><circle cx="5" cy="-2" r="1.2" fill="${INK}"/>
         <ellipse cy="6" rx="2" ry="2.5" fill="${INK}"/>`
      : `<path d="M-10 -5 L-1 -1 Q-5 3 -9 0 Z M10 -5 L1 -1 Q5 3 9 0 Z" fill="#ffe066" stroke="${INK}" stroke-width="1.5" stroke-linejoin="round"/>
         <path d="M-5.5 -2.5 V1.5 M5.5 -2.5 V1.5" stroke="${INK}" stroke-width="1.5" stroke-linecap="round"/>
         <path d="M-4 7 l1.5 -3 l1.5 3 M1 7 l1.5 -3 l1.5 3" fill="none" stroke="${INK}" stroke-width="1.5" stroke-linejoin="round"/>`;
    return `<g transform="translate(${x} ${y}) scale(${s})">${f}</g>`;
  }
  const leg = (d, w = 2) => `<path d="${d}" fill="none" stroke="${INK}" stroke-width="${w}" stroke-linecap="round"/>`;

  const BUGS = [
    { color: '#8a5cff', draw: (c, scared) => `
      <ellipse cx="-10" cy="-11" rx="5" ry="10" transform="rotate(-35 -10 -11)" fill="#e6f6ff" opacity=".85" stroke="${INK}" stroke-width="1.2"/>
      <ellipse cx="10" cy="-11" rx="5" ry="10" transform="rotate(35 10 -11)" fill="#e6f6ff" opacity=".85" stroke="${INK}" stroke-width="1.2"/>
      ${leg('M-7 4 L-15 10 L-19 19 M-9 0 L-19 2 L-21 8 M7 4 L15 10 L19 19 M9 0 L19 2 L21 8', 1.5)}
      ${leg('M-4 -12 Q-6 -18 -11 -21 M4 -12 Q6 -18 11 -21', 1.5)}
      ${leg('M0 9 L0 21', 2.5)}
      <circle cy="-1" r="12" fill="${c}"/>${face(0, -1, 0.95, scared)}` },
    { color: '#ff5fa2', draw: (c, scared) => `
      ${leg('M-13 -4 L-20 -8 M-15 4 L-21 5 M-12 11 L-18 17 M13 -4 L20 -8 M15 4 L21 5 M12 11 L18 17', 2.5)}
      ${leg('M-4 -14 Q-6 -20 -12 -21 M4 -14 Q6 -20 12 -21')}
      <circle cy="1" r="16" fill="${c}"/>
      <path d="M0 10 L0 17" stroke="#c23a7c" stroke-width="2" stroke-linecap="round"/>
      <circle cx="-8" cy="10" r="2.5" fill="#c23a7c"/><circle cx="8" cy="10" r="2.5" fill="#c23a7c"/>
      <ellipse cx="-9" cy="-9" rx="3" ry="2" fill="#fff" opacity=".5" transform="rotate(-40 -9 -9)"/>
      ${face(0, -1, 1, scared)}` },
    { color: '#9be15d', draw: (c, scared) => `
      <g fill="${INK}"><circle cx="-16" cy="16" r="2"/><circle cx="-9" cy="18" r="2"/><circle cx="-2" cy="18" r="2"/></g>
      <g fill="#ff9f43"><path d="M-19 4 l2 -6 l3 5Z"/><path d="M-12 5 l2 -6 l3 5Z"/><path d="M-5 5 l2 -6 l3 5Z"/></g>
      <circle cx="-16" cy="10" r="6" fill="${c}"/><circle cx="-9" cy="12" r="6.5" fill="${c}"/><circle cx="-2" cy="12" r="6.5" fill="${c}"/>
      <g fill="#d8f5c4"><circle cx="-16" cy="10" r="2"/><circle cx="-9" cy="12" r="2"/><circle cx="-2" cy="12" r="2"/></g>
      ${leg('M4 -11 Q2 -18 -2 -20 M12 -11 Q14 -18 18 -20')}
      <circle cx="8" cy="-1" r="12" fill="${c}"/>${face(8, -1, 0.95, scared)}` },
    { color: '#ff9f43', draw: (c, scared) => `
      ${leg('M-8 -6 Q-16 -18 -21 -12 M-10 -1 Q-20 -6 -22 2 M-10 4 Q-19 6 -20 14 M-8 8 Q-14 16 -12 21', 2.5)}
      ${leg('M8 -6 Q16 -18 21 -12 M10 -1 Q20 -6 22 2 M10 4 Q19 6 20 14 M8 8 Q14 16 12 21', 2.5)}
      <circle r="14" fill="${c}"/>
      <circle cx="-3" cy="-11" r="1.5" fill="${INK}"/><circle cx="3" cy="-11" r="1.5" fill="${INK}"/>
      <path d="M-4 10 l1.5 4 l1.5 -4Z M1 10 l1.5 4 l1.5 -4Z" fill="#fff"/>
      ${face(0, 0, 0.95, scared)}` },
  ];
  const bug = (b, scared = false) => `<g>${b.draw(b.color, scared)}</g>`;

  const sprites = {
    sparkle: { w: 36, h: 36, svg: `<svg viewBox="-12 -12 24 24">
      <path d="M0-11Q1.5-1.5 11 0Q1.5 1.5 0 11Q-1.5 1.5-11 0Q-1.5-1.5 0-11Z" fill="#fff6c2"/>
      <circle r="2.5" fill="#fff"/></svg>` },
    // RainWing venom: a black glob with an acid-green glow
    venom: { w: 40, h: 40, svg: `<svg viewBox="-20 -20 40 40">
      <path d="M-15 0 Q-15 -11 -4 -14 Q2 -18 8 -12 Q16 -8 15 1 Q17 10 8 14 Q0 18 -7 13 Q-16 10 -15 0Z" fill="#5dd94a" opacity=".55"/>
      <path d="M-10 0 Q-10 -9 0 -10 Q10 -10 10 0 Q11 9 1 10 Q-10 10 -10 0Z" fill="#1b1426"/>
      <circle cx="-4" cy="-4" r="2.5" fill="#9dff8a"/></svg>` },
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
    germs: BUGS.map((b) => bug(b)),
    germsScared: BUGS.map((b) => bug(b, true)),
    germParticles: BUGS.map((b) => [b.color, '#1b1426', '#5dd94a']), // melted by venom
    pop: 'venom',                // Glory spits venom at each bug (RainWings don't breathe fire)
    tricks: ['rainbow', 'fly'],  // theme-only tricks, start screen only
    teeth: [125, 116],           // in the 250x250 buddy box
    mouth: [125, 118],
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
      spit:    { mood: 'spit' },
      fly_a:   { mood: 'idle', cls: 'trick-fly' },
      fly_b:   { mood: 'idle', cls: 'trick-fly flap' },
      rainbow_1: { mood: 'idle', cls: 'rainbow-1' },
      rainbow_2: { mood: 'idle', cls: 'rainbow-2' },
      rainbow_3: { mood: 'idle', cls: 'rainbow-3' },
      rainbow_4: { mood: 'idle', cls: 'rainbow-4' },
      rainbow_5: { mood: 'idle', cls: 'rainbow-5' },
    },

    css: `
${T} .t-clock { font-family: 'Metamorphous', serif; font-size: 84px; }
${T} .t-bravo {
  font-family: 'Cinzel Decorative', serif; font-weight: 700; font-size: 40px;
  background: linear-gradient(#fff1a8, #ffc23d 45%, #ff7b1c);
  -webkit-background-clip: text; background-clip: text; color: transparent;
  filter: drop-shadow(0 3px 0 #7a1f00);
}

/* Scale colours by mood (RainWing!) */
${T} .character {
  --scale: #3fbfa0; --scale-dark: #2a9a80; --snout: #6fd6bd; --belly: #d8f5c4; --belly-line: #b5e3a0;
  --frill: #ffcf4a; --frill-2: #ff9f43; --wing: #2f8f78; --iris: #f0b429; --ink: #1e1a33;
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
${T} .tail       { fill: none; stroke: var(--scale-dark); stroke-width: 11; stroke-linecap: round; }
${T} .wing       { fill: var(--wing); stroke: var(--scale-dark); stroke-width: 3; stroke-linejoin: round; }
${T} .wing-bone  { fill: none; stroke: var(--scale-dark); stroke-width: 5; stroke-linecap: round; }
${T} .frills path { fill: var(--frill); stroke: var(--frill-2); stroke-width: 2.5; stroke-linejoin: round; }
${T} .horns      { fill: #efe2c4; stroke: #b9a57c; stroke-width: 1.5; stroke-linejoin: round; }
${T} .texture    { fill: none; stroke: var(--scale-dark); stroke-width: 2; stroke-linecap: round; stroke-linejoin: round; opacity: .7; }
${T} .brow       { fill: none; stroke: var(--scale-dark); stroke-width: 4; stroke-linecap: round; }
${T} .glint      { fill: #fff; }
${T} .claws      { fill: #efe2c4; }
${T} .eye-white  { fill: #fff; }
${T} .iris       { fill: var(--iris); }
${T} .pupil      { fill: var(--ink); }
${T} .line       { fill: none; stroke: var(--ink); stroke-width: 3.5; stroke-linecap: round; }
${T} .line.thin  { stroke-width: 2.5; }
${T} .fang       { fill: #fff; }
${T} .mouth-inside { fill: #4a1f3d; }
${T} .foam       { fill: #fff; opacity: .9; }
${T} .arm-brush  { fill: none; stroke: var(--scale); stroke-width: 11; stroke-linecap: round; }
${T} .brush-handle { fill: #4fc3f7; }
${T} .brush-head   { fill: #fff; }
${T} .bristles     { fill: #c8f0ff; }
${T} .sparkles   { fill: #ffd166; }
${T} .zzz        { fill: #c3b5ff; font-family: 'Cinzel Decorative', serif; font-weight: 700; }

/* Hidden unless a mood shows them */
${T} .eyes-closed, ${T} .eyes-happy, ${T} .brush, ${T} .foam, ${T} .zzz,
${T} .sparkles, ${T} .wings-open, ${T} .mouth-grin, ${T} .mouth-spit { display: none; }

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

/* Spitting venom at a bug */
${T} .mood-spit .mouth { display: none; }
${T} .mood-spit .mouth-spit { display: inline; }

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
