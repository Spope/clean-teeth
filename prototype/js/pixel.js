// Pixel-art helper: turns a grid of characters into SVG rects.
//   Pixel.grid(['..KK..', '.KLLK.'], { K: '#123', L: '#9ef' }, x, y, size)
// '.' or ' ' is transparent. Horizontal runs of the same colour become one rect.
window.Pixel = (function () {
  'use strict';

  function grid(rows, palette, x0, y0, size, cls = '') {
    const c = cls ? ` class="${cls}"` : '';
    let out = '';
    rows.forEach((row, y) => {
      let x = 0;
      while (x < row.length) {
        const ch = row[x];
        let end = x + 1;
        while (end < row.length && row[end] === ch) end++;
        if (ch !== '.' && ch !== ' ') {
          out += `<rect${c} x="${x0 + x * size}" y="${y0 + y * size}" width="${(end - x) * size}" height="${size}" fill="${palette[ch]}"/>`;
        }
        x = end;
      }
    });
    return out;
  }

  function rect(x, y, w, h, fill, cls = '') {
    return `<rect${cls ? ` class="${cls}"` : ''} x="${x}" y="${y}" width="${w}" height="${h}" fill="${fill}"/>`;
  }

  return { grid, rect };
})();
