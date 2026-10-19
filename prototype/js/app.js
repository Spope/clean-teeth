(function () {
  'use strict';

  const CONFIG = {
    totalSeconds: 120,     // 2 minutes, as dentists recommend
    zones: 4,              // top-left, top-right, bottom-right, bottom-left (clockwise)
    germsPerZone: 3,
    sleepAfterSeconds: 30, // idle → sleepy
    doneTapGuardMs: 1500,  // ignore taps right after finishing
    celebrateSeconds: 12,  // done → idle automatically
  };

  const SIZE = 466;
  const CENTER = SIZE / 2;
  const RING_R = 215;
  const RING_C = 2 * Math.PI * RING_R;
  const RING_GAP = 30; // includes the round caps (stroke width 18)
  const GERM_R = 168;

  const $ = (id) => document.getElementById(id);
  const screen = $('screen');
  const charHost = $('character');

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
    phase: 'idle',   // idle | brushing | done
    mood: 'idle',
    elapsed: 0,      // seconds of brushing (simulated time)
    zone: -1,
    speed: 1,
    phaseSince: performance.now(),
    lastTs: performance.now(),
    stars: store.get('stars', 0),
  };

  // ---------- Ring ----------
  const segments = [];
  function buildRing() {
    const svg = $('ring');
    const segLen = RING_C / CONFIG.zones - RING_GAP;
    const gapDeg = (RING_GAP / 2 / RING_C) * 360;
    for (let z = 0; z < CONFIG.zones; z++) {
      const rot = `rotate(${180 + z * (360 / CONFIG.zones) + gapDeg} ${CENTER} ${CENTER})`;
      const track = circle('seg-track', rot, `${segLen} ${RING_C}`);
      const fill = circle('seg-fill', rot, `0 ${RING_C}`);
      fill.style.opacity = 0;
      svg.append(track, fill);
      segments.push({ track, fill, len: segLen });
    }
    function circle(cls, transform, dash) {
      const c = document.createElementNS('http://www.w3.org/2000/svg', 'circle');
      c.setAttribute('class', cls);
      c.setAttribute('cx', CENTER);
      c.setAttribute('cy', CENTER);
      c.setAttribute('r', RING_R);
      c.setAttribute('transform', transform);
      c.setAttribute('stroke-dasharray', dash);
      return c;
    }
  }

  function applyRingColors(colors) {
    segments.forEach((s, i) => { s.fill.style.stroke = colors[i % colors.length]; });
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
        const color = Germs.colors[(z + i) % Germs.colors.length];
        const g = document.createElementNS('http://www.w3.org/2000/svg', 'g');
        g.setAttribute('class', 'germ');
        g.setAttribute('transform', `translate(${x.toFixed(1)} ${y.toFixed(1)})`);
        g.innerHTML = Germs.svg(color, (z * 0.37 + i * 0.23) % 1.4);
        svg.append(g);
        germs.push({ el: g, popAt: z * zoneLen + ((i + 1) * zoneLen) / CONFIG.germsPerZone, popped: false });
      }
    }
  }

  function resetGerms() {
    germs.forEach((g) => { g.popped = false; g.el.classList.remove('popped'); });
  }

  function popGerm(g) {
    g.popped = true;
    g.el.classList.add('popped');
    Sound.pop();
  }

  // ---------- Character ----------
  let character = null;
  function loadCharacter(id) {
    character = Characters.get(id) || Characters.list()[0];
    charHost.innerHTML = character.svg;
    charHost.className = `character char-${character.id}`;

    let style = $('char-style');
    if (!style) {
      style = document.createElement('style');
      style.id = 'char-style';
      document.head.append(style);
    }
    style.textContent = character.css;

    const root = document.documentElement.style;
    root.setProperty('--accent', character.theme.accent);
    root.setProperty('--glow', character.theme.glow);
    applyRingColors(character.theme.ring);
    setMood(state.mood);
    store.set('character', character.id);
  }

  function setMood(mood) {
    state.mood = mood;
    [...charHost.classList].filter((c) => c.startsWith('mood-')).forEach((c) => charHost.classList.remove(c));
    charHost.classList.add(`mood-${mood}`);
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
    state.phase = phase;
    state.phaseSince = performance.now();
    screen.classList.remove('phase-idle', 'phase-brushing', 'phase-done');
    screen.classList.add(`phase-${phase}`);
    $('dev-phase').textContent = phase;
  }

  function toIdle() {
    setPhase('idle');
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
    state.elapsed = 0;
    state.zone = -1;
    resetGerms();
    setMood('brushing');
    giggle();
  }

  function finish() {
    state.elapsed = CONFIG.totalSeconds;
    germs.forEach((g) => { if (!g.popped) popGerm(g); });
    setRingProgress(CONFIG.totalSeconds);
    setActiveZone(-1);
    setPhase('done');
    setMood('cheer');
    Sound.finish();
    state.stars += 1;
    store.set('stars', state.stars);
    renderStars(true);
    confetti();
  }

  function updateBrushing() {
    const t = Math.min(state.elapsed, CONFIG.totalSeconds);
    setRingProgress(t);

    const zone = Math.min(CONFIG.zones - 1, Math.floor(t / (CONFIG.totalSeconds / CONFIG.zones)));
    if (zone !== state.zone) {
      if (state.zone !== -1) { Sound.zone(); giggle(); }
      state.zone = zone;
      setActiveZone(zone);
    }

    germs.forEach((g) => { if (!g.popped && t >= g.popAt) popGerm(g); });

    if (state.elapsed >= CONFIG.totalSeconds) finish();
  }

  // ---------- Effects ----------
  function renderStars(bump) {
    $('star-count').textContent = state.stars;
    const el = $('stars');
    if (bump) {
      el.classList.remove('bump');
      void el.offsetWidth;
      el.classList.add('bump');
    }
  }

  function confetti() {
    const host = $('confetti');
    const colors = character.theme.ring.concat(['#ffffff']);
    for (let i = 0; i < 50; i++) {
      const s = document.createElement('span');
      s.style.left = `${Math.random() * 100}%`;
      s.style.background = colors[i % colors.length];
      s.style.animationDuration = `${1.8 + Math.random() * 1.6}s`;
      s.style.animationDelay = `${Math.random() * 0.8}s`;
      s.style.transform = `rotate(${Math.random() * 360}deg)`;
      host.append(s);
    }
    setTimeout(() => { host.innerHTML = ''; }, 4500);
  }

  // ---------- Input ----------
  screen.addEventListener('pointerdown', () => {
    Sound.unlock();
    if (state.phase === 'idle') {
      start();
    } else if (state.phase === 'brushing') {
      giggle();
    } else if (performance.now() - state.phaseSince > CONFIG.doneTapGuardMs) {
      toIdle();
    }
  });

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

    $('dev-time').textContent = `${fmt(state.elapsed)} / ${fmt(CONFIG.totalSeconds)}`
      + (state.phase === 'brushing' ? ` · zone ${state.zone + 1}` : '');
    requestAnimationFrame(tick);
  }

  // ---------- Dev panel ----------
  function setupDev() {
    const sel = $('dev-char');
    Characters.list().forEach((c) => sel.add(new Option(c.name, c.id)));
    sel.value = character.id;
    sel.addEventListener('change', () => loadCharacter(sel.value));

    $('dev-speed').addEventListener('click', (e) => {
      const b = e.target.closest('button');
      if (!b) return;
      state.speed = Number(b.dataset.speed);
      $('dev-speed').querySelectorAll('button').forEach((x) => x.classList.toggle('on', x === b));
    });

    $('dev-sound').addEventListener('change', (e) => Sound.setEnabled(e.target.checked));
    $('dev-skip').addEventListener('click', () => {
      if (state.phase !== 'brushing') start();
      state.elapsed = CONFIG.totalSeconds - 3 / state.speed;
    });
    $('dev-stop').addEventListener('click', toIdle);
    $('dev-reset').addEventListener('click', () => {
      state.stars = 0;
      store.set('stars', 0);
      renderStars(false);
    });
  }

  // Jump straight to a state, e.g. index.html?preview=brushing&t=50
  // (preview = idle | sleepy | brushing | done). Handy for design reviews.
  function applyPreview() {
    const params = new URLSearchParams(location.search);
    const preview = params.get('preview');
    if (preview === 'sleepy') {
      setMood('sleepy');
    } else if (preview === 'brushing') {
      start();
      state.elapsed = Number(params.get('t')) || 0;
      updateBrushing();
    } else if (preview === 'done') {
      start();
      finish();
    }
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
  buildRing();
  buildGerms();
  loadCharacter(store.get('character', 'dino'));
  setupDev();
  renderStars(false);
  toIdle();
  applyPreview();
  fit();
  window.addEventListener('resize', fit);
  requestAnimationFrame(tick);
})();
