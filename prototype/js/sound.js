// Tiny synth for UI sounds (no audio files needed).
// Browsers only allow audio after a user gesture, so call Sound.unlock() on tap.
// Each theme picks a voice (waveform for melodies) and a pop sound.
window.Sound = (function () {
  'use strict';

  let ctx = null;
  let enabled = true;
  let voice = 'triangle';
  let popKind = 'blip';

  function ensure() {
    if (!ctx) {
      const AC = window.AudioContext || window.webkitAudioContext;
      if (!AC) return null;
      ctx = new AC();
    }
    if (ctx.state === 'suspended') ctx.resume();
    return ctx;
  }

  function envelope(c, t, dur, gain) {
    const amp = c.createGain();
    amp.gain.setValueAtTime(0.0001, t);
    amp.gain.exponentialRampToValueAtTime(gain, t + 0.015);
    amp.gain.exponentialRampToValueAtTime(0.0001, t + dur);
    amp.connect(c.destination);
    return amp;
  }

  function tone(freq, start, dur, type = 'sine', gain = 0.15, freqEnd = null) {
    const c = ensure();
    if (!c || !enabled) return;
    const t = c.currentTime + start;
    const osc = c.createOscillator();
    osc.type = type;
    osc.frequency.setValueAtTime(freq, t);
    if (freqEnd) osc.frequency.exponentialRampToValueAtTime(freqEnd, t + dur);
    osc.connect(envelope(c, t, dur, type === 'square' ? gain * 0.4 : gain));
    osc.start(t);
    osc.stop(t + dur + 0.05);
  }

  // Filtered noise sweep: a "fwoosh" of dragon venom, a fuse hiss, crumbling dirt...
  function noise(dur = 0.3, gain = 0.25, from = 600, to = 2400) {
    const c = ensure();
    if (!c || !enabled) return;
    const t = c.currentTime;
    const buf = c.createBuffer(1, Math.ceil(c.sampleRate * dur), c.sampleRate);
    const data = buf.getChannelData(0);
    for (let i = 0; i < data.length; i++) data[i] = Math.random() * 2 - 1;
    const src = c.createBufferSource();
    src.buffer = buf;
    const filter = c.createBiquadFilter();
    filter.type = 'bandpass';
    filter.Q.value = 1.2;
    filter.frequency.setValueAtTime(from, t);
    filter.frequency.exponentialRampToValueAtTime(to, t + dur);
    src.connect(filter).connect(envelope(c, t, dur, gain));
    src.start(t);
  }

  // notes: [[freqHz, seconds], ...]; freq 0 = rest
  function melody(notes, type = voice, gain = 0.15) {
    let t = 0;
    for (const [f, d] of notes) {
      if (f) tone(f, t, d * 1.3, type, gain);
      t += d;
    }
  }

  return {
    unlock: ensure,
    setEnabled(v) { enabled = v; },
    setTheme(sound) {
      voice = sound.voice || 'triangle';
      popKind = sound.pop || 'blip';
    },

    start()  { melody([[523, 0.1], [659, 0.1], [784, 0.2]]); },
    zone()   { melody([[784, 0.12], [1047, 0.22]]); },
    pop()    { if (popKind === 'whoosh') noise(); else tone(500, 0, 0.12, voice === 'square' ? 'square' : 'sine', 0.2, 1400); },
    hiss()   { noise(0.7, 0.12, 3000, 5000); },                 // creeper fuse
    hit()    { tone(160, 0, 0.06, 'square', 0.25, 90); },       // pickaxe on a block
    crumble() { noise(0.25, 0.3, 400, 150); },                  // block breaks
    bling()  { melody([[1568, 0.06], [2093, 0.06], [2637, 0.16]], 'sine', 0.12); },
    collect() { melody([[988, 0.06], [1319, 0.14]], voice, 0.15); },
    magic()  { melody([[784, 0.07], [988, 0.07], [1175, 0.07], [1568, 0.07], [1976, 0.16]], 'sine', 0.1); },
    giggle() { melody([[880, 0.06], [988, 0.06], [880, 0.06], [1175, 0.1]], 'square', 0.05); },
    finish() {
      melody([
        [523, 0.12], [659, 0.12], [784, 0.12], [1047, 0.24],
        [0, 0.06], [784, 0.12], [1047, 0.4],
      ]);
    },
  };
})();
