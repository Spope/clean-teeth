#!/usr/bin/env python3
"""Convert RGBA PNGs into LVGL v9 C images (RGB565A8), using only the standard library.

Usage: png2lvgl.py OUT_DIR file1.png [file2.png ...]

Each `name.png` becomes `OUT_DIR/img_name.c` defining `const lv_image_dsc_t img_name`,
and `OUT_DIR/assets.h` declares them all.

Text images (`*_clock_digit_*`, `*_bravo`) are trimmed to their visible columns so that
proportional digits sit next to each other naturally. Countdown digits (`*_clock_*`) are
stored as A8 (alpha only): the device tints them with the current zone's colour.

Images that would otherwise be rotated on every frame (see TILTS) also get pre-rotated
copies, `img_name_tilt_0` ... `img_name_tilt_<n-1>`, in the same C file: rotating
in software costs the ESP32-S3 several milliseconds per image and frame, swapping is free.
The middle frame is upright, so it is the image itself and not repeated.

`*_peek` images are the poses drawn at the peek-a-boo's zoom. They become only the
frames the trick shows (see PEEK): `img_name_<i>` per tilt, cropped to the part that can
reach the screen, LZ4-compressed (LVGL decompresses each once into its image cache), with
`img_name_<i>_pivot`, where the buddy's feet are in that frame.
"""
import math
import re
import struct
import sys
import zlib
from pathlib import Path


def read_png_rgba(path):
    data = Path(path).read_bytes()
    if data[:8] != b"\x89PNG\r\n\x1a\n":
        raise ValueError(f"{path}: not a PNG")
    pos, idat, w = 8, b"", None
    while pos < len(data):
        length, ctype = struct.unpack(">I4s", data[pos:pos + 8])
        chunk = data[pos + 8:pos + 8 + length]
        pos += 12 + length
        if ctype == b"IHDR":
            w, h, depth, color, _, _, interlace = struct.unpack(">IIBBBBB", chunk)
            if depth != 8 or color not in (2, 6) or interlace:
                raise ValueError(f"{path}: need 8-bit RGB/RGBA, non-interlaced PNG")
            bpp = 4 if color == 6 else 3
        elif ctype == b"IDAT":
            idat += chunk
        elif ctype == b"IEND":
            break

    raw = zlib.decompress(idat)
    stride = w * bpp
    rows, prev = [], bytearray(stride)
    for y in range(h):
        f = raw[y * (stride + 1)]
        line = bytearray(raw[y * (stride + 1) + 1:(y + 1) * (stride + 1)])
        for i in range(stride):
            a = line[i - bpp] if i >= bpp else 0
            b = prev[i]
            c = prev[i - bpp] if i >= bpp else 0
            if f == 1:
                line[i] = (line[i] + a) & 0xFF
            elif f == 2:
                line[i] = (line[i] + b) & 0xFF
            elif f == 3:
                line[i] = (line[i] + ((a + b) >> 1)) & 0xFF
            elif f == 4:
                p = a + b - c
                pa, pb, pc = abs(p - a), abs(p - b), abs(p - c)
                pred = a if pa <= pb and pa <= pc else (b if pb <= pc else c)
                line[i] = (line[i] + pred) & 0xFF
        rows.append(line)
        prev = line

    pixels = []
    for line in rows:
        for x in range(w):
            px = line[x * bpp:(x + 1) * bpp]
            pixels.append((px[0], px[1], px[2], px[3] if bpp == 4 else 255))
    return w, h, pixels


# (stem pattern, max angle in degrees, frame count, pivot as a fraction of width and height)
# Frame i is tilted by -max + i * 2 * max / (count - 1) degrees, clockwise.
TILTS = [
    (r"_germ_\d$", 10, 9, (0.5, 0.5)),          # germ wobble, BRUSHY_GERM_TILTS
    (r"^dragon_brush_[ab]$", 2, 5, (0.5, 1.0)),  # brushing sway, BRUSHY_SWAY_TILTS
]


def transform(w, h, pixels, degrees, pivot, out_w, out_h, out_pivot):
    """Rotate clockwise by `degrees` with bilinear filtering: `pivot` (source pixels) lands on
    `out_pivot` of an `out_w` x `out_h` image."""
    t = math.radians(degrees)
    cos_t, sin_t = math.cos(t), math.sin(t)
    px, py = pivot
    qx, qy = out_pivot
    premul = [(r * a, g * a, b * a, a) for r, g, b, a in pixels]  # avoids dark fringes
    out = []
    for y in range(out_h):
        for x in range(out_w):
            dx, dy = x + 0.5 - qx, y + 0.5 - qy
            sx = px + dx * cos_t + dy * sin_t - 0.5
            sy = py - dx * sin_t + dy * cos_t - 0.5
            x0, y0 = math.floor(sx), math.floor(sy)
            fx, fy = sx - x0, sy - y0
            acc = [0.0, 0.0, 0.0, 0.0]
            for xx, yy, wgt in ((x0, y0, (1 - fx) * (1 - fy)), (x0 + 1, y0, fx * (1 - fy)),
                                (x0, y0 + 1, (1 - fx) * fy), (x0 + 1, y0 + 1, fx * fy)):
                if wgt and 0 <= xx < w and 0 <= yy < h:
                    p = premul[yy * w + xx]
                    for c in range(4):
                        acc[c] += p[c] * wgt
            a = acc[3]
            if a < 0.5:
                out.append((0, 0, 0, 0))
            else:
                out.append(tuple(min(255, round(acc[c] / a)) for c in range(3)) + (min(255, round(a)),))
    return out


def rotate(w, h, pixels, degrees, pivot):
    """Rotate clockwise around `pivot` (pixels), keeping the size."""
    return transform(w, h, pixels, degrees, pivot, w, h, pivot)


def tilts(stem):
    for pattern, max_deg, count, (fx, fy) in TILTS:
        if re.search(pattern, stem):
            return [(-max_deg + i * 2 * max_deg / (count - 1), (fx, fy), i)
                    for i in range(count) if i != count // 2]
    return []


# Peek-a-boo (brushy_tricks.c build_peek): the buddy pops up from the bottom edge, zoomed
# (the *_peek images are drawn at that size) and tilted by -15, 0 or +15 degrees. His feet
# (bottom centre) rise to y = 604 at most (287 below the screen centre, plus the ease-back
# overshoot), so only what is 138 px or more above them can reach the bottom edge (466).
PEEK_TILTS = (-15, 0, 15)  # BRUSHY_PEEK_TILT
PEEK_KEEP_ABOVE = 130      # px above the feet; a few px of margin


def peek_frames(w, h, pixels):
    """Yields (tilt index, width, height, pixels, (pivot x, pivot y)) for each peek tilt."""
    px, py = w / 2, h  # feet
    for i, degrees in enumerate(PEEK_TILTS):
        t = math.radians(degrees)
        corners = [(x - px, y - py) for x in (0, w) for y in (0, h)]
        xs = [dx * math.cos(t) - dy * math.sin(t) for dx, dy in corners]
        ys = [dx * math.sin(t) + dy * math.cos(t) for dx, dy in corners]
        # canvas corner relative to the feet; the feet land on a pixel corner
        ox, oy = math.floor(min(xs)), math.floor(min(ys))
        out_w, out_h = math.ceil(max(xs)) - ox, -PEEK_KEEP_ABOVE - oy
        out = transform(w, h, pixels, degrees, (px, py), out_w, out_h, (-ox, -oy))
        # trim transparent borders
        rows = [y for y in range(out_h) if any(out[y * out_w + x][3] for x in range(out_w))]
        cols = [x for x in range(out_w) if any(out[y * out_w + x][3] for y in rows)]
        x0, x1, y0, y1 = cols[0], cols[-1] + 1, rows[0], rows[-1] + 1
        out = [out[y * out_w + x] for y in range(y0, y1) for x in range(x0, x1)]
        yield i, x1 - x0, y1 - y0, out, (-ox - x0, -oy - y0)


def lz4_compress(src):
    """LZ4 block format (what LVGL's LZ4_decompress_safe reads), greedy matching."""
    n, out, anchor, i, table = len(src), bytearray(), 0, 0, {}

    def length(v):  # extra length bytes after a 15 in the token
        while v >= 255:
            out.append(255)
            v -= 255
        out.append(v)

    while i < n - 12:  # a match must start 12+ bytes before the end and end 5+ before it
        key = src[i:i + 4]
        cand = table.get(key)
        table[key] = i
        if cand is None or i - cand > 65535:
            i += 1
            continue
        m, max_m = 4, n - 5 - i
        while m < max_m:  # extend the match, a chunk at a time while it is long
            k = min(64, max_m - m)
            if src[cand + m:cand + m + k] == src[i + m:i + m + k]:
                m += k
            else:
                while src[cand + m] == src[i + m]:
                    m += 1
                break
        lit = i - anchor
        out.append((min(lit, 15) << 4) | min(m - 4, 15))
        if lit >= 15:
            length(lit - 15)
        out += src[anchor:i]
        out += struct.pack("<H", i - cand)
        if m - 4 >= 15:
            length(m - 4 - 15)
        i += m
        anchor = i
    lit = n - anchor
    out.append(min(lit, 15) << 4)
    if lit >= 15:
        length(lit - 15)
    out += src[anchor:]
    return bytes(out)


def trim_columns(w, h, pixels):
    used = [x for x in range(w) if any(pixels[y * w + x][3] for y in range(h))]
    if not used:
        return w, h, pixels
    x0, x1 = used[0], used[-1] + 1
    return x1 - x0, h, [pixels[y * w + x] for y in range(h) for x in range(x0, x1)]


def to_rgb565a8(pixels):
    color, alpha = bytearray(), bytearray()
    for r, g, b, a in pixels:
        v = ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3)
        color += struct.pack("<H", v)
        alpha.append(a)
    return bytes(color + alpha)


def to_a8(pixels):
    return bytes(a for _, _, _, a in pixels)


def image_c(name, w, h, blob, cf="RGB565A8", stride=None, compress=False):
    flags = 0
    if compress:  # LVGL's compressed image: method, compressed size, decompressed size, data
        packed = lz4_compress(blob)
        blob = struct.pack("<III", 2, len(packed), len(blob)) + packed  # 2: LV_IMAGE_COMPRESS_LZ4
        flags = 0x0008  # LV_IMAGE_FLAGS_COMPRESSED
    lines = []
    for i in range(0, len(blob), 24):
        lines.append("    " + ", ".join(f"0x{b:02x}" for b in blob[i:i + 24]) + ",")
    body = "\n".join(lines)
    return f"""
LV_ATTRIBUTE_MEM_ALIGN LV_ATTRIBUTE_LARGE_CONST static const uint8_t {name}_map[] = {{
{body}
}};

const lv_image_dsc_t {name} = {{
    .header.magic = LV_IMAGE_HEADER_MAGIC,
    .header.cf = LV_COLOR_FORMAT_{cf},
    .header.flags = {flags},
    .header.w = {w},
    .header.h = {h},
    .header.stride = {stride if stride is not None else w * 2},
    .data_size = sizeof({name}_map),
    .data = {name}_map,
}};
"""


def main():
    if len(sys.argv) < 3:
        sys.exit(__doc__)
    out_dir = Path(sys.argv[1])
    out_dir.mkdir(parents=True, exist_ok=True)
    decls = []
    for png in sys.argv[2:]:
        name = "img_" + Path(png).stem
        w, h, pixels = read_png_rgba(png)
        stem = Path(png).stem
        if stem.endswith("_peek"):
            text, sizes = "", []
            for i, fw, fh, frame, (fx, fy) in peek_frames(w, h, pixels):
                text += image_c(f"{name}_{i}", fw, fh, to_rgb565a8(frame), compress=True)
                text += f"\nconst lv_point_t {name}_{i}_pivot = {{{fx}, {fy}}};\n"
                decls += [f"extern const lv_image_dsc_t {name}_{i};", f"extern const lv_point_t {name}_{i}_pivot;"]
                sizes.append(f"{fw}x{fh}")
            (out_dir / f"{name}.c").write_text("/* Generated by tools/png2lvgl.py: do not edit. */\n"
                                               "#include \"lvgl.h\"\n" + text)
            print(f"{png} -> {name}.c (peek frames {', '.join(sizes)})")
            continue
        if "_digit_" in stem or stem.endswith("_bravo"):
            w, h, pixels = trim_columns(w, h, pixels)
        if "_clock_" in stem:
            text = image_c(name, w, h, to_a8(pixels), cf="A8", stride=w)
        else:
            text = image_c(name, w, h, to_rgb565a8(pixels))
        decls.append(f"extern const lv_image_dsc_t {name};")
        frames = tilts(stem)
        for degrees, (fx, fy), i in frames:
            tilted = rotate(w, h, pixels, degrees, (fx * w, fy * h))
            text += image_c(f"{name}_tilt_{i}", w, h, to_rgb565a8(tilted))
            decls.append(f"extern const lv_image_dsc_t {name}_tilt_{i};")
        (out_dir / f"{name}.c").write_text("/* Generated by tools/png2lvgl.py: do not edit. */\n"
                                           "#include \"lvgl.h\"\n" + text)
        print(f"{png} -> {name}.c ({w}x{h}" + (f", {len(frames)} tilted)" if frames else ")"))

    (out_dir / "assets.h").write_text(f"""/* Generated by tools/png2lvgl.py: do not edit. */
#pragma once
#include "lvgl.h"

{chr(10).join(sorted(decls))}
""")


if __name__ == "__main__":
    main()
