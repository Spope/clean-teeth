(function () {
  'use strict';

  const CONFIG = {
    totalSeconds: 120,     // 2 minutes, as dentists recommend
    zones: 4,              // top-left, top-right, bottom-right, bottom-left (clockwise)
    germsPerZone: 3,
    sleepAfterSeconds: 30, // idle → sleepy
    doneTapGuardMs: 1500,  // ignore taps right after finishing
    celebrateSeconds: 12,  // done → idle automatically
    longPressMs: 900,      // hold on the start screen to switch theme
    trickEveryMs: [8000, 15000],  // random pause between tricks
    lowBattery: 25,        // % at boot at or below which the warning shows
    batteryWarnMs: 5000,
  };

  const SIZE = 466;
  const CENTER = SIZE / 2;
  const RING_R = 215;
  const RING_C = 2 * Math.PI * RING_R;
  const BLOCKS_PER_ZONE = 8; // blocky ring style
  const GERM_R = 168;
  const BUDDY = 108;         // top-left corner of the 250x250 buddy box on screen
  const SVG_NS = 'http://www.w3.org/2000/svg';

  const $ = (id) => document.getElementById(id);
  const screen = $('screen');
  const charHost = $('character');
  const rand = (min, max) => min + Math.random() * (max - min);

  const store = {
    get(key, fallback) {
      try {
        const v = localStorage.getItem(key);
        return v === null ? fallback : JSON.parse(v);
      } catch { return fallback; }
    },
    set(key, value) {
      try { localStorage.setItem(key, JSON.stringify(value)); } catch { /* ignore */ }
    },
  };

  const state = {
    theme: null,
    phase: 'idle',   // idle | brushing | done
    mood: 'idle',
    elapsed: 0,      // seconds of brushing (simulated time)
    zone: -1,
    speed: 1,
    phaseSince: performance.now(),
    lastTs: performance.now(),
  };

  // Timers that belong to the current phase; cleared whenever the phase changes
  let phaseTimers = [];
  const later = (ms, fn) => phaseTimers.push(setTimeout(fn, ms));

  function svgEl(tag, attrs) {
    const el = document.createElementNS(SVG_NS, tag);
    for (const [k, v] of Object.entries(attrs)) el.setAttribute(k, v);
    return el;
  }

  // ---------- Ring ----------
  let segments = [];
  function buildRing(theme) {
    const svg = $('ring');
    svg.innerHTML = '';
    segments = [];
    const blocky = theme.style === 'blocky';
    const gap = blocky ? 12 : 30; // round caps need a bigger gap
    const segLen = RING_C / CONFIG.zones - gap;
    const gapDeg = (gap / 2 / RING_C) * 360;
    for (let z = 0; z < CONFIG.zones; z++) {
      const base = { cx: CENTER, cy: CENTER, r: RING_R, transform: `rotate(${180 + z * (360 / CONFIG.zones) + gapDeg} ${CENTER} ${CENTER})` };
      const track = svgEl('circle', { ...base, class: 'seg-track', 'stroke-dasharray': `${segLen} ${RING_C}` });
      const fill = svgEl('circle', { ...base, class: 'seg-fill', 'stroke-dasharray': `0 ${RING_C}` });
      fill.style.stroke = theme.ring[z % theme.ring.length];
      fill.style.opacity = 0;
      svg.append(track, fill);
      if (blocky) {
        // Black notches cut each zone into blocks, like the XP bar
        const block = segLen / BLOCKS_PER_ZONE;
        const dashes = [0, block - 2];
        for (let k = 1; k < BLOCKS_PER_ZONE; k++) dashes.push(4, k < BLOCKS_PER_ZONE - 1 ? block - 4 : RING_C);
        svg.append(svgEl('circle', { ...base, class: 'seg-gaps', 'stroke-dasharray': dashes.join(' ') }));
      }
      segments.push({ track, fill, len: segLen });
    }
  }

  function setRingProgress(t) {
    const zoneLen = CONFIG.totalSeconds / CONFIG.zones;
    segments.forEach((s, z) => {
      const p = Math.min(1, Math.max(0, (t - z * zoneLen) / zoneLen));
      s.fill.setAttribute('stroke-dasharray', `${s.len * p} ${RING_C}`);
      s.fill.style.opacity = p > 0 ? 1 : 0;
    });
  }

  function setActiveZone(z) {
    segments.forEach((s, i) => s.track.classList.toggle('active', i === z));
    if (z >= 0) $('clock').style.color = state.theme.ring[z];
  }

  // Big faint countdown behind the buddy
  function renderClock(t) {
    const text = fmt(Math.ceil(CONFIG.totalSeconds - t));
    const el = $('clock-text');
    if (el.textContent !== text) el.textContent = text;
  }

  // ---------- Sprites and particles ----------
  function sprite(svg, w, h, layer = 'fx') {
    const el = document.createElement('div');
    el.className = 'sprite';
    el.style.width = `${w}px`;
    el.style.height = `${h}px`;
    el.innerHTML = svg;
    el.dataset.w = w;
    el.dataset.h = h;
    $(layer).append(el);
    return el;
  }
  const themeSprite = (name, layer) => {
    const s = state.theme.sprites[name];
    return sprite(s.svg, s.w, s.h, layer);
  };

  // Transform that centres a sprite on (x, y)
  const at = (el, x, y, scale = 1, rot = 0) =>
    `translate(${x - el.dataset.w / 2}px, ${y - el.dataset.h / 2}px) rotate(${rot}deg) scale(${scale})`;

  // Pieces flying out of (x, y): pixels for blocky themes, round embers otherwise
  function burst(x, y, colors, count = 12, spread = 1) {
    const blocky = state.theme.style === 'blocky';
    for (let i = 0; i < count; i++) {
      const p = document.createElement('div');
      const size = blocky ? 6 : 7;
      p.className = 'particle';
      p.style.width = p.style.height = `${size}px`;
      p.style.background = colors[i % colors.length];
      p.style.borderRadius = blocky ? '0' : '50%';
      $('fx').append(p);
      const a = (i / count) * Math.PI * 2 + rand(-0.3, 0.3);
      const d = rand(35, 70) * spread;
      const x0 = x - size / 2, y0 = y - size / 2;
      p.animate([
        { transform: `translate(${x0}px, ${y0}px)`, opacity: 1 },
        { transform: `translate(${x0 + Math.cos(a) * d}px, ${y0 + Math.sin(a) * d + 25}px)`, opacity: 0 },
      ], { duration: rand(450, 650), easing: blocky ? 'steps(6)' : 'cubic-bezier(.2, .7, .4, 1)' })
        .onfinish = () => p.remove();
    }
  }

  function clearFx() {
    $('fx').innerHTML = '';
    $('fx-back').innerHTML = '';
  }

  // ---------- Germs ----------
  const germs = [];
  function buildGerms() {
    const svg = $('germs');
    const zoneDeg = 360 / CONFIG.zones;
    const zoneLen = CONFIG.totalSeconds / CONFIG.zones;
    for (let z = 0; z < CONFIG.zones; z++) {
      for (let i = 0; i < CONFIG.germsPerZone; i++) {
        const deg = 180 + z * zoneDeg + (zoneDeg * (i + 1)) / (CONFIG.germsPerZone + 1);
        const rad = (deg * Math.PI) / 180;
        const x = CENTER + GERM_R * Math.cos(rad);
        const y = CENTER + GERM_R * Math.sin(rad);
        const g = svgEl('g', { class: 'germ', transform: `translate(${x.toFixed(1)} ${y.toFixed(1)})` });
        const inner = svgEl('g', { class: 'germ-inner', style: `--d:-${((z * 0.37 + i * 0.23) % 1.4).toFixed(2)}s` });
        g.append(inner);
        svg.append(g);
        germs.push({ el: g, inner, x, y, kind: (z + i) % 4, popAt: z * zoneLen + ((i + 1) * zoneLen) / CONFIG.germsPerZone, popped: false });
      }
    }
  }

  function drawGerm(g, scared = false) {
    const set = scared ? state.theme.germsScared : state.theme.germs;
    g.inner.innerHTML = `${set[g.kind]}<rect class="flash" x="-20" y="-20" width="40" height="40" fill="#fff"/>`;
  }

  function resetGerms() {
    germs.forEach((g) => {
      clearTimeout(g.calmTimer);
      g.popped = false;
      g.el.classList.remove('popped', 'scared', 'priming');
      drawGerm(g);
    });
  }

  // Quiet pop (end of brushing, previews)
  function popGerm(g) {
    g.popped = true;
    g.el.classList.remove('priming', 'scared');
    g.el.classList.add('popped');
  }

  // The others shiver with a scared face for a moment
  function scareGerms() {
    germs.forEach((g) => {
      if (g.popped) return;
      drawGerm(g, true);
      g.el.classList.add('scared');
      clearTimeout(g.calmTimer);
      g.calmTimer = setTimeout(() => {
        g.el.classList.remove('scared');
        if (!g.popped) drawGerm(g);
      }, 900);
    });
  }

  function explode(g) {
    popGerm(g);
    burst(g.x, g.y, state.theme.germParticles[g.kind]);
    Sound.pop();
    shake();
    scareGerms();
  }

  // Each theme defeats germs its own way
  function defeatGerm(g) {
    g.popped = true; // claimed: won't be triggered again
    const theme = state.theme;
    if (theme.pop === 'creeper' && g.kind === 0) {
      g.el.classList.add('priming');
      Sound.hiss();
      later(700, () => explode(g));
    } else if (theme.pop === 'venom' && !trick && state.mood === 'brushing') {
      spitVenom(g); // only when the buddy isn't busy with another animation
    } else {
      explode(g);
    }
  }

  function spitVenom(g) {
    setMood('spit');
    const [mx, my] = state.theme.mouth;
    const glob = themeSprite('venom');
    glob.animate([
      { transform: at(glob, BUDDY + mx, BUDDY + my, 0.4) },
      { transform: at(glob, g.x, g.y, 1.1) },
    ], { duration: 380, easing: 'ease-in', fill: 'forwards' }).onfinish = () => {
      glob.remove();
      if (state.phase !== 'brushing') return; // brushing ended meanwhile
      explode(g);
      if (state.phase === 'brushing' && state.mood === 'spit') setMood('brushing');
    };
  }

  // ---------- Tricks ----------
  // Keyframes [ms, x, y, scale, rotation°, easing of the next segment], applied to the buddy
  // around its feet (bottom centre). Mirrored in device/ui/brushy_ui.c.
  // A trick may also have `cls` (classes on the buddy meanwhile) and timed `events`.
  const EASE = { in: 'ease-in', out: 'ease-out', back: 'cubic-bezier(.34, 1.56, .64, 1)', linear: 'linear' };
  const OFF = 420; // fully outside the 466px screen
  const XP_ORBS = 7;
  const XP_CHEST = [125, 170]; // where orbs fly to, in the 250x250 buddy box

  const TRICKS = {
    // Duck out of view, pop up big and close from the bottom edge (side: -1 left, 0 middle,
    // 1 right, leaning in), duck again, then bounce back into place.
    peek: (side) => {
      const x = side * 90, r = -side * 15;
      return { frames: [
        [0, 0, 0, 1, 0, 'in'],
        [350, 0, 400, 1, 0, 'linear'],
        [360, x, 700, 1.9, r, 'back'],
        [850, x, 287, 1.9, r, 'linear'],
        [2000, x, 287, 1.9, r, 'in'],
        [2400, x, 700, 1.9, r, 'linear'],
        [2410, 0, 400, 1, 0, 'back'],
        [2900, 0, 0, 1, 0],
      ] };
    },

    // Slip off one side, then lean in from that edge to spy, twice, and walk back.
    peekSide: (d) => {
      const x = d * 255, r = -d * 22;
      return { frames: [
        [0, 0, 0, 1, 0, 'in'],
        [400, d * OFF, 0, 1, 0, 'linear'],
        [410, d * OFF, 20, 1, r, 'out'],
        [900, x, 20, 1, r, 'linear'],
        [1800, x, 20, 1, r, 'in'],
        [2100, d * OFF, 20, 1, r, 'linear'],
        [2500, d * OFF, 20, 1, r, 'out'],
        [2800, x, 20, 1, r, 'linear'],
        [3500, x, 20, 1, r, 'in'],
        [3800, d * OFF, 20, 1, r, 'linear'],
        [3810, d * OFF, 0, 1, 0, 'out'],
        [4400, 0, 0, 1, 0],
      ] };
    },

    // Walk off one side, come back in from that side and hop all the way across until gone
    // on the other side, then walk back in to the middle.
    slide: (d) => {
      const frames = [
        [0, 0, 0, 1, 0, 'in'],
        [150, d * 15, -6, 1, d * 6, 'in'],
        [700, d * OFF, 0, 1, d * 6, 'linear'],
      ];
      for (let k = 0; k <= 8; k++) { // crossing: 8 little hops in 2 s
        frames.push([1100 + k * 250, d * (OFF - k * 2 * OFF / 8), k % 2 ? -10 : 0, 1, -d * 6, 'linear']);
      }
      frames.push([3500, -d * OFF, 0, 1, d * 6, 'out'], [4200, -d * 15, -6, 1, d * 6, 'out'], [4400, 0, 0, 1, 0]);
      return { frames };
    },

    // Minecraft: a grass block appears by Steve's left foot, he hits it three times with the
    // pickaxe in his left hand (the right one may be brushing) and breaks it.
    mine: () => ({
      cls: 'trick-mine',
      frames: [[0, 0, 0, 1, 0, 'linear'], [3000, 0, 0, 1, 0]],
      events: [[0, 'block'], [500, 'hit'], [1100, 'hit'], [1700, 'break']],
    }),

    // Minecraft: XP orbs appear around the ring one after another and zip into Steve, then
    // he hops as he levels up. Hands stay free, so it also plays while brushing.
    xp: () => {
      const events = [];
      for (let k = 0; k < XP_ORBS; k++) events.push([k * 220, 'orb']);
      events.push([2250, 'levelup']);
      return { frames: [[0, 0, 0, 1, 0, 'linear'], [2250, 0, 0, 1, 0, 'out'], [2450, 0, -14, 1, 0, 'in'], [2650, 0, 0, 1, 0]], events };
    },

    // Dragon: Glory's scales cycle through the rainbow while she hops on the spot.
    rainbow: () => {
      const frames = [];
      for (let k = 0; k <= 8; k++) frames.push([k * 400, 0, k % 2 ? -12 : 0, 1, 0, k % 2 ? 'in' : 'out']);
      const events = [[0, 'magic']];
      for (let k = 0; k < 10; k++) events.push([k * 320, `rainbow-${(k % 5) + 1}`]);
      return { frames, events };
    },

    // Dragon: take off, fly across the top of the screen flapping, glide back down.
    fly: (d) => {
      const frames = [
        [0, 0, 0, 1, 0, 'in'],
        [400, d * 40, -50, 1, d * 8, 'in'],
        [900, d * OFF, -120, 1, d * 10, 'linear'],
      ];
      for (let k = 0; k <= 8; k++) {
        frames.push([1300 + k * 225, d * (OFF - k * 2 * OFF / 8), -110 + (k % 2 ? -25 : 0), 1, -d * 10, 'linear']);
      }
      frames.push([3500, -d * OFF, -120, 1, d * 10, 'out'], [4300, -d * 40, -40, 1, d * 6, 'out'], [4700, 0, 0, 1, 0]);
      return { cls: 'trick-fly', frames, events: [[0, 'whoosh'], [3500, 'whoosh']] };
    },
  };
  const GENERIC_TRICKS = ['peek', 'peekSide', 'slide'];

  let trick = null; // { anim, cls, timers, block }
  let nextTrickAt = 0;
  let lastTrick = null; // never picked twice in a row

  function scheduleTrick() {
    const [min, max] = CONFIG.trickEveryMs;
    nextTrickAt = performance.now() + rand(min, max);
  }

  function hasTrick(name) {
    return GENERIC_TRICKS.includes(name) || (state.theme.tricks || []).includes(name);
  }

  function pickTrick() {
    const list = [...GENERIC_TRICKS, ...(state.theme.tricks || [])].filter((t) => t !== lastTrick);
    return list[Math.floor(Math.random() * list.length)];
  }

  // Timed things that happen during a trick
  function trickEvent(t, name) {
    if (name === 'block') {
      t.block = themeSprite('block', 'fx-back');
      const [x, y] = [BUDDY + 26, BUDDY + 226];
      t.block.animate([{ transform: at(t.block, x, y, 0) }, { transform: at(t.block, x, y, 1) }],
        { duration: 200, easing: 'steps(3)', fill: 'forwards' });
      t.blockAt = [x, y];
    } else if (name === 'hit' || name === 'break') {
      charHost.classList.add('hit');
      t.timers.push(setTimeout(() => charHost.classList.remove('hit'), 180));
      Sound.hit();
      const [x, y] = t.blockAt;
      const dirt = ['#8b5a2b', '#6e4520', '#5dbb3a'];
      if (name === 'hit') {
        burst(x, y - 20, dirt, 4, 0.5);
        t.hits = (t.hits || 0) + 1;
        if (t.hits === 2) t.block.innerHTML = state.theme.sprites.block_cracked.svg;
      } else {
        t.block.remove();
        t.block = null;
        burst(x, y, dirt, 16, 1);
        Sound.crumble();
        shake();
      }
    } else if (name.startsWith('rainbow-')) {
      charHost.classList.remove('rainbow-1', 'rainbow-2', 'rainbow-3', 'rainbow-4', 'rainbow-5');
      charHost.classList.add(name);
      t.cls = name;
    } else if (name === 'orb') {
      // Appears on a random spot near the ring, drifts a little, then zips into Steve
      const orb = themeSprite('xp_orb');
      const a = rand(0, Math.PI * 2);
      const [x0, y0] = [CENTER + Math.cos(a) * 190, CENTER + Math.sin(a) * 190];
      const [x1, y1] = [BUDDY + XP_CHEST[0], BUDDY + XP_CHEST[1]];
      orb.animate([
        { transform: at(orb, x0, y0, 0), easing: 'ease-out' },
        { transform: at(orb, x0 + (x1 - x0) * 0.08, y0 + (y1 - y0) * 0.08, 1), offset: 0.35, easing: 'ease-in' },
        { transform: at(orb, x1, y1, 0.5) },
      ], { duration: 900, fill: 'forwards' }).onfinish = () => {
        orb.remove();
        t.orbs.delete(orb);
        Sound.xp();
        burst(x1, y1, ['#c8f542', '#7fd42a'], 3, 0.4);
      };
      t.orbs.add(orb);
    } else if (name === 'levelup') {
      Sound.levelUp();
      burst(BUDDY + XP_CHEST[0], BUDDY + XP_CHEST[1] - 40, ['#c8f542', '#7fd42a', '#fbffd0'], 12, 1.2);
    } else if (name === 'magic') {
      Sound.magic();
    } else if (name === 'whoosh') {
      Sound.pop();
    }
  }

  function runTrick(name) {
    if (!hasTrick(name)) return;
    stopTrick();
    lastTrick = name;
    const side = name === 'peek' ? Math.floor(Math.random() * 3) - 1 : (Math.random() < 0.5 ? -1 : 1);
    const def = TRICKS[name](side);
    const total = def.frames[def.frames.length - 1][0];
    const t = { cls: def.cls, timers: [], orbs: new Set() };
    if (def.cls) charHost.classList.add(...def.cls.split(' '));
    t.anim = $('buddy').animate(def.frames.map(([ms, x, y, sc, r, e]) => ({
      offset: ms / total,
      transform: `translate(${x}px, ${y}px) rotate(${r}deg) scale(${sc})`,
      easing: EASE[e] || 'linear',
    })), { duration: total });
    for (const [ms, ev] of def.events || []) t.timers.push(setTimeout(() => trickEvent(t, ev), ms));
    t.anim.onfinish = () => { stopTrick(); scheduleTrick(); };
    trick = t;
  }

  function stopTrick() {
    if (!trick) return;
    const t = trick;
    trick = null;
    t.anim.cancel();
    t.timers.forEach(clearTimeout);
    if (t.cls) charHost.classList.remove(...t.cls.split(' '));
    charHost.classList.remove('hit');
    if (t.block) t.block.remove();
    t.orbs.forEach((o) => o.remove());
  }

  function canTrick() {
    return state.phase === 'brushing' || (state.phase === 'idle' && state.mood === 'idle');
  }

  // Quick damped wobble of everything on screen (germ popped)
  function shake() {
    const frames = [];
    for (let i = 0; i <= 12; i++) {
      const p = i / 12;
      const x = 7 * (1 - p) * Math.sin(p * Math.PI * 8);
      const y = 3.5 * (1 - p) * Math.sin(p * Math.PI * 6);
      frames.push({ transform: `translate(${x.toFixed(1)}px, ${y.toFixed(1)}px)` });
    }
    $('stage').animate(frames, { duration: 350 });
  }

  // ---------- Mirror check (dev panel only: mid-brushing it looked like the end) ----------
  function mirrorCheck() {
    stopTrick();
    setMood('grin');
    Sound.bling();
    const [tx, ty] = state.theme.teeth;
    const glint = (dx, dy, delay) => {
      const s = themeSprite('sparkle');
      const x = BUDDY + tx + dx, y = BUDDY + ty + dy;
      s.animate([
        { transform: at(s, x, y, 0, 0) },
        { transform: at(s, x, y, 1.2, 90), offset: 0.5 },
        { transform: at(s, x, y, 0, 180) },
      ], { duration: 800, delay, fill: 'both', easing: 'ease-in-out' }).onfinish = () => s.remove();
    };
    glint(-22, -6, 0);
    glint(24, 4, 300);
    glint(-4, -14, 700);
    later(1700, () => { if (state.phase === 'brushing' && state.mood === 'grin') setMood('brushing'); });
  }

  // ---------- Reward drop: the reward pops out of the buddy, bounces, and is collected ----------
  function rewardDrop() {
    const el = sprite(state.theme.reward, 32, 32);
    const d = Math.random() < 0.5 ? -1 : 1;
    const blocky = state.theme.style === 'blocky';
    const path = [ // [ms, x, y, scale, easing]
      [0, CENTER, 260, 0.3, 'out'],
      [300, CENTER + d * 30, 165, 1.6, 'in'],
      [650, CENTER + d * 55, 330, 1.6, 'out'],
      [800, CENTER + d * 60, 305, 1.6, 'in'],
      [950, CENTER + d * 62, 330, 1.6, 'linear'],
      [1500, CENTER + d * 62, 330, 1.6, 'in'],
      [1900, CENTER + d * 62, 330, 0],
    ];
    const total = path[path.length - 1][0];
    el.animate(path.map(([ms, x, y, sc, e]) => ({
      offset: ms / total,
      transform: at(el, x, y, sc),
      easing: blocky ? 'steps(4)' : (EASE[e] || 'linear'), // Minecraft: pixel steps
    })), { duration: total, fill: 'forwards' }).onfinish = () => {
      el.remove();
      if (state.phase !== 'done') return;
      Sound.collect();
    };
  }

  // ---------- Theme ----------
  function loadTheme(id) {
    const theme = Themes.get(id) || Themes.list()[0];
    stopTrick();
    clearFx();
    state.theme = theme;

    let style = $('theme-style');
    if (!style) {
      style = document.createElement('style');
      style.id = 'theme-style';
      document.head.append(style);
    }
    style.textContent = theme.css;

    let font = $('theme-font');
    if (!font) {
      font = document.createElement('link');
      font.id = 'theme-font';
      font.rel = 'stylesheet';
      document.head.append(font);
    }
    if (theme.fontUrl) font.href = theme.fontUrl;

    const root = document.documentElement.style;
    root.setProperty('--accent', theme.accent);
    root.setProperty('--glow', theme.glow);

    charHost.innerHTML = theme.character;
    germs.forEach((g) => drawGerm(g));
    $('bravo-text').textContent = theme.bravo;
    buildRing(theme);
    if (state.phase === 'brushing' || state.phase === 'done') {
      setRingProgress(Math.min(state.elapsed, CONFIG.totalSeconds));
      setActiveZone(state.phase === 'brushing' ? state.zone : -1);
    }
    Sound.setTheme(theme.sound);
    applyScreenClasses();
    setMood(state.mood);
    store.set('theme', theme.id);
    const sel = $('dev-theme');
    if (sel) sel.value = theme.id;
    document.querySelectorAll('[data-trick]').forEach((b) => {
      b.disabled = !['shake', 'mirror'].includes(b.dataset.trick) && !hasTrick(b.dataset.trick);
    });
  }

  function nextTheme() {
    const list = Themes.list();
    const i = list.findIndex((t) => t.id === state.theme.id);
    loadTheme(list[(i + 1) % list.length].id);
    state.phaseSince = performance.now(); // don't fall asleep right away
    setMood('idle');
    giggle();
  }

  function applyScreenClasses() {
    screen.className = `screen phase-${state.phase} theme-${state.theme.id} style-${state.theme.style}`;
  }

  function setMood(mood) {
    if (mood === 'sleepy' || mood === 'cheer') stopTrick();
    state.mood = mood;
    const keep = trick && trick.cls ? ` ${trick.cls}` : '';
    charHost.className = `character mood-${mood}${keep}`;
  }

  function giggle() {
    charHost.classList.remove('giggle');
    void charHost.offsetWidth; // restart the animation
    charHost.classList.add('giggle');
    Sound.giggle();
  }
  charHost.addEventListener('animationend', (e) => {
    if (e.target === charHost) charHost.classList.remove('giggle');
  });

  // ---------- Phases ----------
  function setPhase(phase) {
    stopTrick();
    scheduleTrick();
    phaseTimers.forEach(clearTimeout);
    phaseTimers = [];
    state.phase = phase;
    state.phaseSince = performance.now();
    applyScreenClasses();
    $('dev-phase').textContent = phase;
  }

  function toIdle() {
    setPhase('idle');
    clearFx();
    state.elapsed = 0;
    state.zone = -1;
    resetGerms();
    setRingProgress(0);
    setActiveZone(-1);
    setMood('idle');
  }

  function start() {
    Sound.start();
    setPhase('brushing');
    clearFx();
    state.elapsed = 0;
    state.zone = -1;
    resetGerms();
    renderClock(0);
    setMood('brushing');
    giggle();
  }

  function finish() {
    state.elapsed = CONFIG.totalSeconds;
    setPhase('done');
    germs.forEach((g) => { if (!g.el.classList.contains('popped')) popGerm(g); });
    setRingProgress(CONFIG.totalSeconds);
    setActiveZone(-1);
    setMood('cheer');
    Sound.finish();
    rewardDrop();
    confetti();
  }

  function updateBrushing() {
    const t = Math.min(state.elapsed, CONFIG.totalSeconds);
    setRingProgress(t);
    renderClock(t);

    const zone = Math.min(CONFIG.zones - 1, Math.floor(t / (CONFIG.totalSeconds / CONFIG.zones)));
    if (zone !== state.zone) {
      if (state.zone !== -1) { Sound.zone(); giggle(); }
      state.zone = zone;
      setActiveZone(zone);
    }

    germs.forEach((g) => { if (!g.popped && t >= g.popAt) defeatGerm(g); });

    if (state.elapsed >= CONFIG.totalSeconds) finish();
  }

  // ---------- Effects ----------
  function confetti() {
    const host = $('confetti');
    const colors = state.theme.ring.concat(['#ffffff']);
    for (let i = 0; i < 50; i++) {
      const s = document.createElement('span');
      s.style.left = `${Math.random() * 100}%`;
      s.style.background = colors[i % colors.length];
      s.style.animationDuration = `${1.8 + Math.random() * 1.6}s`;
      s.style.animationDelay = `${Math.random() * 0.8}s`;
      host.append(s);
    }
    setTimeout(() => { host.innerHTML = ''; }, 4500);
  }

  // ---------- Low battery warning ----------
  // Shown at boot (device: brushy_ui_battery). Covers the stage: a tap only dismisses it.
  let batteryTimer = null;
  function batteryWarning(percent) {
    if (!(percent >= 0 && percent <= CONFIG.lowBattery)) return;
    const el = $('battery');
    $('battery-fill').style.width = `${Math.max(percent, 8)}%`;
    $('battery-text').textContent = `${percent}%`;
    el.hidden = false;
    el.classList.remove('closing');
    Sound.lowBattery();
    clearTimeout(batteryTimer);
    batteryTimer = setTimeout(closeBatteryWarning, CONFIG.batteryWarnMs);
  }
  function closeBatteryWarning() {
    const el = $('battery');
    if (el.hidden || el.classList.contains('closing')) return;
    clearTimeout(batteryTimer);
    el.classList.add('closing');
    setTimeout(() => { if (el.classList.contains('closing')) el.hidden = true; }, 400);
  }
  $('battery').addEventListener('pointerdown', (e) => {
    e.stopPropagation(); // not a tap on the buddy
    Sound.unlock();
    closeBatteryWarning();
  });

  // ---------- Input ----------
  // Start screen: tap to start, press and hold to switch theme.
  let holdTimer = null;
  screen.addEventListener('pointerdown', () => {
    Sound.unlock();
    if (state.phase === 'idle') {
      holdTimer = setTimeout(() => { holdTimer = null; nextTheme(); }, CONFIG.longPressMs);
    } else if (state.phase === 'brushing') {
      giggle();
    } else if (performance.now() - state.phaseSince > CONFIG.doneTapGuardMs) {
      toIdle();
    }
  });
  function release(isTap) {
    if (!holdTimer) return;
    clearTimeout(holdTimer);
    holdTimer = null;
    if (isTap && state.phase === 'idle') start();
  }
  screen.addEventListener('pointerup', () => release(true));
  screen.addEventListener('pointerleave', () => release(false));
  screen.addEventListener('pointercancel', () => release(false));

  // ---------- Main loop ----------
  function fmt(s) {
    s = Math.floor(s);
    return `${Math.floor(s / 60)}:${String(s % 60).padStart(2, '0')}`;
  }

  function tick(ts) {
    const dt = (ts - state.lastTs) / 1000;
    state.lastTs = ts;
    const inPhase = ((ts - state.phaseSince) / 1000) * state.speed;

    if (state.phase === 'brushing') {
      state.elapsed += dt * state.speed;
      updateBrushing();
    } else if (state.phase === 'idle') {
      if (state.mood === 'idle' && inPhase > CONFIG.sleepAfterSeconds) setMood('sleepy');
    } else if (state.phase === 'done' && inPhase > CONFIG.celebrateSeconds) {
      toIdle();
    }
    const calm = state.mood === 'idle' || state.mood === 'brushing'; // not mid-grin or mid-spit
    if (!trick && ts >= nextTrickAt && canTrick() && calm) runTrick(pickTrick());

    $('dev-time').textContent = `${fmt(state.elapsed)} / ${fmt(CONFIG.totalSeconds)}`
      + (state.phase === 'brushing' ? ` · zone ${state.zone + 1}` : '');
    requestAnimationFrame(tick);
  }

  // ---------- Dev panel ----------
  function setupDev() {
    const sel = $('dev-theme');
    Themes.list().forEach((t) => sel.add(new Option(t.name, t.id)));
    sel.value = state.theme.id;
    sel.addEventListener('change', () => loadTheme(sel.value));

    $('dev-speed').addEventListener('click', (e) => {
      const b = e.target.closest('button');
      if (!b) return;
      state.speed = Number(b.dataset.speed);
      $('dev-speed').querySelectorAll('button').forEach((x) => x.classList.toggle('on', x === b));
    });

    document.querySelectorAll('[data-trick]').forEach((b) => b.addEventListener('click', () => playTrick(b.dataset.trick)));
    $('dev-sound').addEventListener('change', (e) => Sound.setEnabled(e.target.checked));
    $('dev-skip').addEventListener('click', () => {
      if (state.phase !== 'brushing') start();
      state.elapsed = CONFIG.totalSeconds - 3 / state.speed;
    });
    $('dev-stop').addEventListener('click', toIdle);
    $('dev-battery').addEventListener('click', () => batteryWarning(18));
  }

  function playTrick(name) {
    if (name === 'shake') shake();
    else if (name === 'mirror') mirrorCheck();
    else runTrick(name);
  }

  // Jump straight to a state, e.g. index.html?theme=dragon&preview=brushing&t=50&trick=peek
  // (preview = idle | sleepy | brushing | done; trick = peek | peekSide | slide | mine |
  // rainbow | fly | mirror | shake; battery = charge at boot, warns at 25 or less).
  // Handy for design reviews.
  function applyPreview() {
    const params = new URLSearchParams(location.search);
    const preview = params.get('preview');
    if (preview === 'sleepy') {
      setMood('sleepy');
    } else if (preview === 'brushing') {
      start();
      state.elapsed = Number(params.get('t')) || 0;
      const t = Math.min(state.elapsed, CONFIG.totalSeconds);
      germs.forEach((g) => { if (t >= g.popAt) popGerm(g); }); // skip the effects
      updateBrushing();
    } else if (preview === 'done') {
      start();
      finish();
    }
    const trickName = params.get('trick');
    if (trickName) playTrick(trickName);
    if (params.has('battery')) batteryWarning(Number(params.get('battery')));
  }

  // Scale the simulated device to fit small windows (never above 1:1).
  function fit() {
    const slot = $('slot');
    const full = 520;
    const devH = document.querySelector('.dev').offsetHeight;
    const s = Math.min(1, (window.innerWidth - 32) / full, (window.innerHeight - devH - 72) / full);
    const scale = Math.max(0.4, s);
    slot.style.width = slot.style.height = `${full * scale}px`;
    $('device').style.transform = `scale(${scale})`;
  }

  // ---------- Boot ----------
  buildGerms();
  loadTheme(new URLSearchParams(location.search).get('theme') || store.get('theme', 'minecraft'));
  setupDev();
  toIdle();
  applyPreview();
  fit();
  window.addEventListener('resize', fit);
  requestAnimationFrame(tick);
})();
