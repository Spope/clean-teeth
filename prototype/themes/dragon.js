// "Les Royaumes de feu" theme: Glory the RainWing. RainWing scales change colour with
// mood (teal when calm, lavender when sleepy, pink when happy), and so does she.
(function () {
  'use strict';

  const sparkle = 'M0-11Q2-2 11 0Q2 2 0 11Q-2 2-11 0Q-2-2 0-11Z';

  // Drawn after the book art rather than as a baby dragon, facing us: long neck, wedge head
  // with a blue face, stern slanted eyes under heavy brows, swept-back horns, orange ruff and
  // wings, yellow speckles, and a curled prehensile tail.
  const spots = [
    [100, 68], [104, 78], [140, 68], [136, 78], [106, 110], [105, 120], [104, 130], [134, 110],
    [135, 120], [136, 130], [96, 148], [91, 158], [144, 148], [149, 158], [80, 192], [86, 204],
    [160, 192], [154, 204],
  ].map(([x, y]) => `<circle cx="${x}" cy="${y}" r="1.8"/>`).join('');

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
      <path class="membrane" d="M106 138 L70 90 Q62 124 50 150 Q58 176 82 194 L100 176 Z"/>
      <path class="membrane" d="M134 138 L170 90 Q178 124 190 150 Q182 176 158 194 L140 176 Z"/>
      <path class="wing-bone" d="M106 138 L70 90 L50 150 M70 90 L82 194 M134 138 L170 90 L190 150 M170 90 L158 194"/>
    </g>
    <g class="wings-open">
      <path class="membrane" d="M104 140 L28 58 Q36 92 14 108 Q36 114 26 140 Q50 140 46 164 Q68 156 96 174 Z"/>
      <path class="membrane" d="M136 140 L212 58 Q204 92 226 108 Q204 114 214 140 Q190 140 194 164 Q172 156 144 174 Z"/>
      <path class="wing-bone" d="M104 140 L28 58 M28 58 L14 108 M28 58 L26 140 M28 58 L46 164
        M136 140 L212 58 M212 58 L226 108 M212 58 L214 140 M212 58 L194 164"/>
    </g>

    <path class="tail" d="M146 216 Q194 228 211 202"/>
    <path class="tail tip" d="M211 202 Q226 174 208 160 Q194 152 188 166 Q186 176 198 174 Q204 172 202 166"/>

    <ellipse class="scale" cx="84" cy="202" rx="15" ry="22"/>
    <ellipse class="scale" cx="156" cy="202" rx="15" ry="22"/>
    <ellipse class="scale-dark" cx="80" cy="227" rx="15" ry="6"/>
    <ellipse class="scale-dark" cx="160" cy="227" rx="15" ry="6"/>
    <path class="scale" d="M104 128 Q84 150 86 180 Q88 206 104 216 L136 216 Q152 206 154 180 Q156 150 136 128 Z"/>
    <path class="scale" d="M110 90 L106 136 L134 136 L130 90 Z"/>
    <path class="belly" d="M113 96 L127 96 L132 140 Q144 176 131 212 L109 212 Q96 176 108 140 Z"/>
    <path class="belly-lines" d="M113 108 H127 M112 118 H128 M111 128 H129 M108 142 H132 M105 156 H135 M104 170 H136 M104 184 H136 M106 198 H134"/>
    <g class="spikes">
      <path d="M104 112 l-8 -2 l7 8 Z M103 124 l-8 0 l7 7 Z M136 112 l8 -2 l-7 8 Z M137 124 l8 0 l-7 7 Z"/>
    </g>
    <g class="spots">${spots}</g>

    <path class="scale arm" d="M104 146 Q94 178 100 220 L92 230 L114 232 L112 220 Q110 182 116 152 Z"/>
    <path class="scale arm arm-r" d="M136 146 Q146 178 140 220 L148 230 L126 232 L128 220 Q130 182 124 152 Z"/>
    <path class="texture" d="M100 170 l4 -3 l4 3 M140 170 l-4 -3 l-4 3 M78 196 l5 -4 l5 4 M162 196 l-5 -4 l-5 4"/>
    <g class="claws">
      <path d="M68 228 l-4 6 l7 -2 Z M92 230 l-3 5 l6 -2 Z M98 231 l-1 5 l5 -3 Z"/>
      <path d="M172 228 l4 6 l-7 -2 Z"/>
      <path class="arm-r" d="M148 230 l3 5 l-6 -2 Z M142 231 l1 5 l-5 -3 Z"/>
    </g>

    <g class="frills">
      <path d="M94 42 L64 22 L90 56 Z"/>
      <path d="M92 56 L58 56 L94 66 Z"/>
      <path d="M98 68 L68 86 L104 78 Z"/>
      <path d="M146 42 L176 22 L150 56 Z"/>
      <path d="M148 56 L182 56 L146 66 Z"/>
      <path d="M142 68 L172 86 L136 78 Z"/>
    </g>
    <g class="horns">
      <path d="M105 32 Q94 12 76 4 Q96 9 114 28 Z"/>
      <path d="M135 32 Q146 12 164 4 Q144 9 126 28 Z"/>
    </g>

    <path class="scale" d="M120 24 Q142 24 148 42 Q152 56 144 66 Q136 76 134 90 Q132 102 120 104 Q108 102 106 90 Q104 76 96 66 Q88 56 92 42 Q98 24 120 24 Z"/>
    <path class="face" d="M120 24 Q142 24 148 42 Q150 52 145 60 Q134 58 128 68 L128 92 Q120 96 112 92 L112 68 Q106 58 95 60 Q90 52 92 42 Q98 24 120 24 Z"/>
    <path class="texture" d="M112 34 l8 5 l8 -5"/>
    <path class="line thin" d="M114 88 l2 2 M126 88 l-2 2"/>

    <g class="eyes-open">
      <path class="iris" d="M95 49 Q105 45 115 55 Q103 59 95 49 Z"/>
      <path class="iris" d="M145 49 Q135 45 125 55 Q137 59 145 49 Z"/>
      <ellipse class="pupil" cx="105.5" cy="52" rx="1.3" ry="3.6"/>
      <ellipse class="pupil" cx="134.5" cy="52" rx="1.3" ry="3.6"/>
      <circle class="glint" cx="108" cy="51" r="1.1"/>
      <circle class="glint" cx="137" cy="51" r="1.1"/>
    </g>
    <g class="eyes-closed line thin">
      <path d="M95 50 Q105 56 115 55"/>
      <path d="M145 50 Q135 56 125 55"/>
    </g>
    <g class="eyes-happy line thin">
      <path d="M95 52 Q105 46 115 54"/>
      <path d="M145 52 Q135 46 125 54"/>
    </g>
    <path class="brow" d="M91 45 Q105 40 117 51 M149 45 Q135 40 123 51"/>

    <g class="mouth">
      <path class="line thin" d="M109 96 Q120 100 131 96"/>
      <path class="fang" d="M111 97 l2 6 l2 -5 Z"/>
      <path class="fang" d="M125 98 l2 5 l2 -6 Z"/>
    </g>
    <g class="mouth-grin">
      <path class="mouth-inside" d="M106 93 Q120 110 134 93 Q120 99 106 93 Z"/>
      <path class="fang" d="M108 94 Q120 99 132 94 L130 98 Q120 103 110 98 Z"/>
    </g>
    <g class="mouth-spit">
      <ellipse class="mouth-inside" cx="120" cy="99" rx="9" ry="7"/>
      <path class="fang" d="M113 93 l2 6 l2 -5 Z"/>
      <path class="fang" d="M123 94 l2 5 l2 -6 Z"/>
      <circle cx="120" cy="101" r="3.5" fill="#1b1426" stroke="#5dd94a" stroke-width="1.5"/>
    </g>

    <g class="foam">
      <circle cx="104" cy="102" r="4"/>
      <circle cx="98" cy="95" r="3"/>
      <circle cx="110" cy="108" r="2.5"/>
      <circle cx="137" cy="103" r="3.5"/>
    </g>
    <g class="brush">
      <path class="arm-brush-edge" d="M134 152 Q164 140 156 104"/>
      <path class="arm-brush" d="M134 152 Q164 140 156 104"/>
      <rect class="brush-handle" x="126" y="93" width="30" height="7" rx="3.5"/>
      <rect class="brush-head" x="106" y="91" width="22" height="11" rx="3.5"/>
      <rect class="bristles" x="108" y="86" width="18" height="6" rx="2"/>
      <circle class="hand" cx="156" cy="97" r="7"/>
      <path class="claws" d="M150 92 l-4 -1 l3 4 Z M149 99 l-4 1 l4 3 Z"/>
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
  const RAINBOW = [ // scale, dark, face, belly, belly line, frill, frill edge, wing membrane
    ['#ff5a6e', '#d93a50', '#b82a5a', '#ffe3e7', '#ffc2ca', '#ffd23f', '#ff9f43', '#ffb03f'],
    ['#ff9a3c', '#e07a1f', '#d9462a', '#fff0dc', '#ffd9b0', '#ff5a6e', '#d93a50', '#ffd23f'],
    ['#ffd23f', '#e0b21f', '#ff8a2e', '#fffbe0', '#fff0a8', '#3fbfa0', '#2a9a80', '#ff9a3c'],
    ['#4aa8ff', '#2f86dd', '#2f55c8', '#e3f1ff', '#bcdcff', '#ffd23f', '#ff9f43', '#a66bff'],
    ['#a66bff', '#844ddd', '#5e3cc8', '#f0e6ff', '#dcc8ff', '#7ce3ff', '#3fb0d8', '#ff7ac2'],
  ];
  const rainbowCss = RAINBOW.map(([sc, dark, face, belly, line, frill, frill2, membrane], i) =>
    `.theme-dragon .character.rainbow-${i + 1} { --scale: ${sc}; --scale-dark: ${dark}; --face: ${face}; --belly: ${belly};
      --belly-line: ${line}; --frill: ${frill}; --frill-2: ${frill2}; --membrane: ${membrane}; --spots: ${belly}; }`).join('\n');

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
    tricks: ['rainbow', 'fly'],  // theme-only tricks
    teeth: [125, 101],           // in the 250x250 buddy box
    mouth: [125, 104],
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
  --scale: #2fbf9f; --scale-dark: #1f8a74; --face: #2f7fd8; --belly: #e6f4e2; --belly-line: #bcdcc4;
  --frill: #ff7a2e; --frill-2: #d9461f; --membrane: #ff8a3c; --spots: #ffe14d;
  --horn: #6b6385; --claw: #efe2c4; --iris: #c6f04a; --ink: #1e1a33;
}
${T} .character.mood-sleepy {
  --scale: #8a8fe0; --scale-dark: #6c70c4; --face: #6a5fc8; --belly: #e4e2ff; --belly-line: #c9c6f5;
  --frill: #c3b5ff; --frill-2: #9a8cf0; --membrane: #b7a6ff; --spots: #e6e0ff;
}
${T} .character.mood-cheer {
  --scale: #ff7ac2; --scale-dark: #e0559f; --face: #a65be0; --belly: #ffe3f1; --belly-line: #ffc2df;
  --frill: #ffe066; --frill-2: #ffb020; --membrane: #ffd23f; --spots: #fff3a0;
}

${T} .scale      { fill: var(--scale); }
${T} .scale-dark { fill: var(--scale-dark); }
${T} .face       { fill: var(--face); }
${T} .belly      { fill: var(--belly); }
${T} .belly-lines { fill: none; stroke: var(--belly-line); stroke-width: 2.5; stroke-linecap: round; }
${T} .spots      { fill: var(--spots); }
${T} .tail       { fill: none; stroke: var(--scale-dark); stroke-width: 12; stroke-linecap: round; }
${T} .tail.tip   { stroke-width: 7; }
${T} .membrane   { fill: var(--membrane); stroke: var(--frill-2); stroke-width: 2; stroke-linejoin: round; }
${T} .wing-bone  { fill: none; stroke: var(--scale-dark); stroke-width: 3.5; stroke-linecap: round; stroke-linejoin: round; }
${T} .frills path, ${T} .spikes { fill: var(--frill); stroke: var(--frill-2); stroke-width: 1.5; stroke-linejoin: round; }
${T} .horns      { fill: var(--horn); stroke: #9a93b5; stroke-width: 1.2; stroke-linejoin: round; }
${T} .texture    { fill: none; stroke: var(--scale-dark); stroke-width: 2; stroke-linecap: round; stroke-linejoin: round; opacity: .7; }
${T} .brow       { fill: none; stroke: var(--ink); stroke-width: 3.5; stroke-linecap: round; }
${T} .glint      { fill: #fff; }
${T} .claws      { fill: var(--claw); }
${T} .hand       { fill: var(--scale); stroke: var(--scale-dark); stroke-width: 2; }
${T} .iris       { fill: var(--iris); stroke: var(--ink); stroke-width: 1; }
${T} .pupil      { fill: var(--ink); }
${T} .line       { fill: none; stroke: var(--ink); stroke-width: 3.5; stroke-linecap: round; }
${T} .line.thin  { stroke-width: 2.2; }
${T} .fang       { fill: #fff; }
${T} .mouth-inside { fill: #4a1f3d; }
${T} .foam       { fill: #fff; opacity: .9; }
${T} .arm-brush-edge { fill: none; stroke: var(--scale-dark); stroke-width: 13; stroke-linecap: round; }
${T} .arm-brush  { fill: none; stroke: var(--scale); stroke-width: 9; stroke-linecap: round; }
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

${T} .mood-idle .body-all  { animation: rw-bob 3s ease-in-out infinite; }
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

${T} .mood-cheer .wings-folded { display: none; }
${T} .mood-cheer .sparkles, ${T} .mood-cheer .wings-open { display: inline; }
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

@keyframes rw-bob     { 50% { transform: translateY(-3px); } }
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
