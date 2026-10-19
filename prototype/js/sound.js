// Tiny synth for UI sounds (no audio files needed).
// Browsers only allow audio after a user gesture, so call Sound.unlock() on tap.
window.Sound = (function () {
  'use strict';

  let ctx = null;
  let enabled = true;

  function ensure() {
    if (!ctx) {
      const AC = window.AudioContext || window.webkitAudioContext;
      if (!AC) return null;
      ctx = new AC();
    }
    if (ctx.state === 'suspended') ctx.resume();
    return ctx;
  }

  function tone(freq, start, dur, type = 'sine', gain = 0.15, freqEnd = null) {
    const c = ensure();
    if (!c || !enabled) return;
    const t = c.currentTime + start;
    const osc = c.createOscillator();
    const amp = c.createGain();
    osc.type = type;
    osc.frequency.setValueAtTime(freq, t);
    if (freqEnd) osc.frequency.exponentialRampToValueAtTime(freqEnd, t + dur);
    amp.gain.setValueAtTime(0.0001, t);
    amp.gain.exponentialRampToValueAtTime(gain, t + 0.015);
    amp.gain.exponentialRampToValueAtTime(0.0001, t + dur);
    osc.connect(amp).connect(c.destination);
    osc.start(t);
    osc.stop(t + dur + 0.05);
  }

  // notes: [[freqHz, seconds], ...]; freq 0 = rest
  function melody(notes, type = 'triangle', gain = 0.15) {
    let t = 0;
    for (const [f, d] of notes) {
      if (f) tone(f, t, d * 1.3, type, gain);
      t += d;
    }
  }

  return {
    unlock: ensure,
    setEnabled(v) { enabled = v; },

    start()  { melody([[523, 0.1], [659, 0.1], [784, 0.2]]); },
    zone()   { melody([[784, 0.12], [1047, 0.22]]); },
    pop()    { tone(500, 0, 0.12, 'sine', 0.2, 1400); },
    giggle() { melody([[880, 0.06], [988, 0.06], [880, 0.06], [1175, 0.1]], 'square', 0.05); },
    finish() {
      melody([
        [523, 0.12], [659, 0.12], [784, 0.12], [1047, 0.24],
        [0, 0.06], [784, 0.12], [1047, 0.4],
      ]);
    },
  };
})();
