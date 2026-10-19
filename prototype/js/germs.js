// Germ artwork, shared by the prototype and the device asset exporter (tools/render.html).
window.Germs = (function () {
  'use strict';

  const colors = ['#a66bff', '#ff6bd6', '#8fe36b', '#ff9f43'];

  // Germ centred on (0,0), about 42px across. `delay` desynchronises the wobble.
  function svg(color, delay = 0) {
    let bumps = '';
    for (let k = 0; k < 8; k++) {
      const a = (k / 8) * Math.PI * 2;
      bumps += `<circle cx="${(15 * Math.cos(a)).toFixed(1)}" cy="${(15 * Math.sin(a)).toFixed(1)}" r="5.5"/>`;
    }
    return `<g class="germ-inner" style="--d:-${delay.toFixed(2)}s" fill="${color}">
      ${bumps}<circle r="15"/>
      <circle cx="-5.5" cy="-3" r="4.5" fill="#fff"/><circle cx="5.5" cy="-3" r="4.5" fill="#fff"/>
      <circle cx="-4.5" cy="-2" r="2.2" fill="#1e1a33"/><circle cx="6.5" cy="-2" r="2.2" fill="#1e1a33"/>
      <path d="M-10 -10 L-2 -7 M10 -10 L2 -7" stroke="#1e1a33" stroke-width="2" stroke-linecap="round"/>
      <path d="M-5 7 Q0 4 5 7" fill="none" stroke="#1e1a33" stroke-width="2" stroke-linecap="round"/>
    </g>`;
  }

  return { colors, svg };
})();
