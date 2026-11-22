#!/usr/bin/env node
// What tools/export_assets.sh needs from a theme (prototype/themes/<id>.js):
//   node tools/theme_assets.js <id>       the images to render, one per line:
//                                         pose <name> | sprite <name> <w> <h>
//   node tools/theme_assets.js <id> json  the theme's settings for the device build
//                                         (assets/themes/<id>.json, read by tools/png2lvgl.py)
// Fails if the theme breaks the contract in prototype/themes/registry.js.
'use strict';
const fs = require('fs');
const path = require('path');

const [id, mode] = process.argv.slice(2);
const dir = path.join(__dirname, '..', 'prototype');
const file = path.join(dir, 'themes', `${id}.js`);
if (!id || !fs.existsSync(file)) {
  console.error(`usage: theme_assets.js <id> [json]  (no prototype/themes/${id}.js)`);
  process.exit(1);
}

globalThis.window = globalThis;
require(path.join(dir, 'js', 'pixel.js'));
require(path.join(dir, 'themes', 'registry.js'));
require(file);

const theme = Themes.get(id);
if (!theme) {
  console.error(`prototype/themes/${id}.js doesn't register a theme with id '${id}'`);
  process.exit(1);
}
if (Themes.check(theme).length) { // the problems were printed by Themes.register
  console.error(`theme '${id}' not exported: fix the problems above first`);
  process.exit(1);
}

if (mode === 'json') {
  // Only what the device needs besides the images (see brushy_theme_t in device/ui/brushy_theme.h)
  const settings = {
    _note: `exported from prototype/themes/${id}.js by tools/export_assets.sh: edit the theme, not this file`,
    id: theme.id,
    name: theme.name,
    style: theme.style,
    accent: theme.accent,
    ring: theme.ring,
    sound: { voice: theme.sound.voice, pop: theme.sound.pop },
    pop: theme.pop || 'burst',
    tricks: theme.tricks || [],
    teeth: theme.teeth,
    mouth: theme.mouth || theme.teeth,
    germParticles: theme.germParticles,
    sway: !!theme.sway,
    poses: Object.keys(theme.poses),
    sprites: Object.keys(theme.sprites),
  };
  // One line per list of numbers or names, for readable diffs
  console.log(JSON.stringify(settings, null, 2)
    .replace(/\[\s+([^[\]{}]*?)\s+\]/g, (_, items) => `[${items.split(/,\s+/).join(', ')}]`));
} else {
  for (const name of Object.keys(theme.poses)) console.log(`pose ${name}`);
  for (const [name, s] of Object.entries(theme.sprites)) console.log(`sprite ${name} ${s.w} ${s.h}`);
}
