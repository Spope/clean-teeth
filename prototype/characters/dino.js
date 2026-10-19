// Dino: a little green dinosaur who brushes along with the child.
// To recolour, change the CSS variables at the top of `css` below.
Characters.register({
  id: 'dino',
  name: 'Dino',
  theme: {
    accent: '#5bd48a',
    glow: 'rgba(91, 212, 138, 0.28)',
    ring: ['#7ce38b', '#ffd166', '#ff8fab', '#7cc8ff'],
  },

  svg: `
<svg viewBox="0 0 240 240" aria-label="Dino">
  <g class="sparkles">
    <path transform="translate(22 60)"  d="M0-11Q2-2 11 0Q2 2 0 11Q-2 2-11 0Q-2-2 0-11Z"/>
    <path transform="translate(218 52) scale(.8)" d="M0-11Q2-2 11 0Q2 2 0 11Q-2 2-11 0Q-2-2 0-11Z"/>
    <path transform="translate(14 170) scale(.7)" d="M0-11Q2-2 11 0Q2 2 0 11Q-2 2-11 0Q-2-2 0-11Z"/>
    <path transform="translate(228 160)" d="M0-11Q2-2 11 0Q2 2 0 11Q-2 2-11 0Q-2-2 0-11Z"/>
  </g>

  <g class="body-all">
    <path class="skin-dark tail" d="M172 196 Q226 192 226 140 Q206 176 168 170 Z"/>

    <g class="spikes">
      <path d="M80 54 L90 22 L106 46 Z"/>
      <path d="M104 44 L120 10 L136 44 Z"/>
      <path d="M134 46 L150 22 L160 54 Z"/>
    </g>

    <ellipse class="skin" cx="120" cy="180" rx="66" ry="50"/>
    <ellipse class="belly" cx="120" cy="190" rx="42" ry="32"/>
    <ellipse class="skin-dark" cx="88"  cy="228" rx="20" ry="9"/>
    <ellipse class="skin-dark" cx="152" cy="228" rx="20" ry="9"/>
    <ellipse class="skin arm" cx="62" cy="172" rx="12" ry="19" transform="rotate(25 62 172)"/>
    <ellipse class="skin arm arm-r" cx="178" cy="172" rx="12" ry="19" transform="rotate(-25 178 172)"/>

    <ellipse class="skin" cx="120" cy="95" rx="76" ry="62"/>

    <g class="eyes-open">
      <circle class="eye-white" cx="92"  cy="86" r="17"/>
      <circle class="eye-white" cx="148" cy="86" r="17"/>
      <circle class="pupil" cx="95"  cy="89" r="9"/>
      <circle class="pupil" cx="151" cy="89" r="9"/>
      <circle class="eye-white" cx="98"  cy="85" r="3.5"/>
      <circle class="eye-white" cx="154" cy="85" r="3.5"/>
    </g>
    <g class="eyes-closed line">
      <path d="M76 88 Q92 100 108 88"/>
      <path d="M132 88 Q148 100 164 88"/>
    </g>
    <g class="eyes-happy line">
      <path d="M76 94 Q92 74 108 94"/>
      <path d="M132 94 Q148 74 164 94"/>
    </g>

    <ellipse class="cheek" cx="70"  cy="114" rx="12" ry="7"/>
    <ellipse class="cheek" cx="170" cy="114" rx="12" ry="7"/>
    <circle class="nostril" cx="111" cy="106" r="2.5"/>
    <circle class="nostril" cx="129" cy="106" r="2.5"/>

    <g class="mouth">
      <path class="mouth-inside" d="M96 120 Q120 150 144 120 Q120 128 96 120 Z"/>
      <ellipse class="tongue" cx="120" cy="136" rx="9" ry="5"/>
      <rect class="tooth" x="104" y="120" width="8" height="7" rx="2"/>
      <rect class="tooth" x="128" y="120" width="8" height="7" rx="2"/>
    </g>

    <g class="foam">
      <circle cx="98"  cy="140" r="5"/>
      <circle cx="90"  cy="131" r="3.5"/>
      <circle cx="104" cy="148" r="3"/>
      <circle cx="146" cy="142" r="4"/>
    </g>

    <g class="brush">
      <path class="skin arm-brush" d="M178 172 L188 140" />
      <rect class="brush-handle" x="128" y="127" width="62" height="9" rx="4.5"/>
      <rect class="brush-head" x="112" y="123" width="22" height="14" rx="4"/>
      <rect class="bristles" x="114" y="116" width="18" height="8" rx="2"/>
      <circle class="skin" cx="188" cy="134" r="11"/>
    </g>
  </g>

  <g class="zzz">
    <text x="176" y="44" font-size="20">z</text>
    <text x="194" y="26" font-size="26">Z</text>
  </g>
</svg>`,

  // Still frames exported for the device (tools/export_assets.sh). Each pose is a
  // mood plus optional CSS; animations are disabled during export. The device
  // animates movement (bob, jump, wiggle) itself and swaps between these frames.
  poses: {
    idle:    { mood: 'idle' },
    blink:   { mood: 'idle', css: '.char-dino .eyes-open{display:none} .char-dino .eyes-closed{display:inline}' },
    sleepy:  { mood: 'sleepy' },
    brush_a: { mood: 'brushing', css: '.char-dino .brush{transform:translateX(-9px)}' },
    brush_b: { mood: 'brushing', css: '.char-dino .brush{transform:translateX(9px)}' },
    cheer:   { mood: 'cheer' },
  },

  css: `
.char-dino {
  --skin: #5bd48a;
  --skin-dark: #3bb26e;
  --belly: #d4f7c5;
  --spikes: #ffb347;
  --ink: #1e1a33;
}
.char-dino .skin      { fill: var(--skin); }
.char-dino .skin-dark { fill: var(--skin-dark); }
.char-dino .belly     { fill: var(--belly); }
.char-dino .spikes    { fill: var(--spikes); stroke: var(--spikes); stroke-width: 6; stroke-linejoin: round; }
.char-dino .eye-white { fill: #fff; }
.char-dino .pupil     { fill: var(--ink); }
.char-dino .line      { fill: none; stroke: var(--ink); stroke-width: 5; stroke-linecap: round; }
.char-dino .cheek     { fill: #ff6f9c; opacity: .8; }
.char-dino .nostril   { fill: var(--skin-dark); }
.char-dino .mouth-inside { fill: #4a1f3d; }
.char-dino .tongue    { fill: #ff7a9c; }
.char-dino .tooth     { fill: #fff; }
.char-dino .foam      { fill: #fff; opacity: .9; }
.char-dino .arm-brush { fill: none; stroke: var(--skin); stroke-width: 20; stroke-linecap: round; }
.char-dino .brush-handle { fill: #4fc3f7; }
.char-dino .brush-head   { fill: #fff; }
.char-dino .bristles     { fill: #c8f0ff; }
.char-dino .sparkles  { fill: #ffd166; }
.char-dino .zzz       { fill: #b9c3ff; font-family: inherit; font-weight: 700; }

/* Hidden unless a mood shows them */
.char-dino .eyes-closed, .char-dino .eyes-happy, .char-dino .brush,
.char-dino .foam, .char-dino .zzz, .char-dino .sparkles { display: none; }

.char-dino .body-all, .char-dino .eyes-open, .char-dino .brush,
.char-dino .foam, .char-dino .sparkles path, .char-dino .zzz text {
  transform-box: fill-box;
  transform-origin: center;
}
.char-dino .body-all { transform-origin: 50% 100%; }

/* idle: bob + blink */
.char-dino.mood-idle .body-all  { animation: dino-bob 2s ease-in-out infinite; }
.char-dino.mood-idle .eyes-open { animation: dino-blink 4s infinite; }

/* sleepy: eyes closed, slow breathing, floating Zzz */
.char-dino.mood-sleepy .eyes-open   { display: none; }
.char-dino.mood-sleepy .eyes-closed,
.char-dino.mood-sleepy .zzz         { display: inline; }
.char-dino.mood-sleepy .body-all    { animation: dino-breathe 3.2s ease-in-out infinite; }
.char-dino.mood-sleepy .zzz text    { animation: dino-zzz 2.4s ease-in-out infinite; }
.char-dino.mood-sleepy .zzz text:nth-child(2) { animation-delay: .6s; }

/* brushing: toothbrush scrubs, foam bubbles, body wiggles */
.char-dino.mood-brushing .arm-r { display: none; }
.char-dino.mood-brushing .brush,
.char-dino.mood-brushing .foam  { display: inline; }
.char-dino.mood-brushing .brush { animation: dino-scrub .28s ease-in-out infinite alternate; }
.char-dino.mood-brushing .foam  { animation: dino-foam .5s ease-in-out infinite alternate; }
.char-dino.mood-brushing .body-all  { animation: dino-wiggle .56s ease-in-out infinite alternate; }
.char-dino.mood-brushing .eyes-open { animation: dino-blink 3s infinite; }

/* cheer: happy eyes, jumping, sparkles */
.char-dino.mood-cheer .eyes-open  { display: none; }
.char-dino.mood-cheer .eyes-happy,
.char-dino.mood-cheer .sparkles   { display: inline; }
.char-dino.mood-cheer .body-all   { animation: dino-jump .7s ease-in-out infinite; }
.char-dino.mood-cheer .sparkles path { animation: dino-twinkle 1s ease-in-out infinite alternate; }
.char-dino.mood-cheer .sparkles path:nth-child(2) { animation-delay: .3s; }
.char-dino.mood-cheer .sparkles path:nth-child(3) { animation-delay: .6s; }
.char-dino.mood-cheer .sparkles path:nth-child(4) { animation-delay: .15s; }

@keyframes dino-bob     { 50% { transform: translateY(-5px); } }
@keyframes dino-blink   { 0%, 93%, 100% { transform: scaleY(1); } 96% { transform: scaleY(.1); } }
@keyframes dino-breathe { 50% { transform: scale(1.03, .98); } }
@keyframes dino-zzz     { 0% { opacity: 0; transform: translateY(6px); } 50% { opacity: 1; } 100% { opacity: 0; transform: translateY(-10px); } }
@keyframes dino-scrub   { from { transform: translateX(-9px); } to { transform: translateX(9px); } }
@keyframes dino-foam    { from { transform: scale(.85); } to { transform: scale(1.1); } }
@keyframes dino-wiggle  { from { transform: rotate(-2deg); } to { transform: rotate(2deg); } }
@keyframes dino-jump    { 0%, 100% { transform: translateY(0) scale(1.05, .95); } 45% { transform: translateY(-20px) scale(.97, 1.03); } }
@keyframes dino-twinkle { from { transform: scale(.4) rotate(0); opacity: .4; } to { transform: scale(1.1) rotate(45deg); opacity: 1; } }
`,
});
