#!/usr/bin/env node
// Prints the assets a theme needs, one per line, for tools/export_assets.sh:
//   pose <name> | sprite <name> <w> <h>
// Usage: node tools/theme_assets.js <theme-id>
'use strict';
const path = require('path');

globalThis.window = globalThis;
const dir = path.join(__dirname, '..', 'prototype');
require(path.join(dir, 'js', 'pixel.js'));
require(path.join(dir, 'themes', 'registry.js'));
require(path.join(dir, 'themes', `${process.argv[2]}.js`));

const theme = Themes.get(process.argv[2]);
for (const name of Object.keys(theme.poses)) console.log(`pose ${name}`);
for (const [name, s] of Object.entries(theme.sprites || {})) console.log(`sprite ${name} ${s.w} ${s.h}`);
