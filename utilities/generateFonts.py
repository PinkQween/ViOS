#!/usr/bin/env python3

import os
import sys
import site
sys.path.insert(0, site.getusersitepackages())
import freetype

output_dir = 'src/fonts'
try:
    os.makedirs(output_dir, exist_ok=True)
    print(f"[DEBUG] Ensured directory exists: {output_dir}")
except Exception as e:
    print(f"[ERROR] Could not create directory {output_dir}: {e}")
    raise

font_dir = 'utilities/fonts'

if not os.path.exists(font_dir):
    print(f"❌ Font directory '{font_dir}' does not exist.")
    sys.exit(1)

font_files = [f for f in os.listdir(font_dir) if f.lower().endswith(('.ttf', '.otf'))]
if not font_files:
    print(f"⚠️ No .ttf or .otf fonts found in '{font_dir}'. Nothing to process.")
    sys.exit(0)  # Consider 0 if this is a non-error skip

if not os.path.exists(font_dir):
    raise RuntimeError(f"Font directory '{font_dir}' does not exist")

font_files = [f for f in os.listdir(font_dir) if f.lower().endswith(('.ttf', '.otf'))]
if not font_files:
    raise RuntimeError(f"No .ttf or .otf fonts found in {font_dir}")

for font_filename in font_files:
    font_path = os.path.join(font_dir, font_filename)
    font_name = os.path.splitext(font_filename)[0]
    try:
        face = freetype.Face(font_path)
        face.set_pixel_sizes(0, 16)
    except freetype.FT_Exception as e:
        print(f"Error loading font {font_filename}: {e}")
        continue

    face = freetype.Face(font_path)
    face.set_pixel_sizes(0, 16)

    max_width = 0
    max_ascender = 0
    max_descender = 0

    for i in range(32, 127):
        face.load_char(chr(i), freetype.FT_LOAD_RENDER | freetype.FT_LOAD_TARGET_MONO)
        bitmap = face.glyph.bitmap
        top = face.glyph.bitmap_top
        bottom = bitmap.rows - top
        max_width = max(max_width, bitmap.width)
        max_ascender = max(max_ascender, top)
        max_descender = max(max_descender, bottom)

    max_height = max_ascender + max_descender

    out_c_path = os.path.join(output_dir, f'characters_{font_name}.c')
    out_h_path = os.path.join(output_dir, f'characters_{font_name}.h')

    glyph_data = []
    kerning_pairs = {}

    for i in range(32, 127):
        ch = chr(i)
        face.load_char(ch, freetype.FT_LOAD_RENDER | freetype.FT_LOAD_TARGET_MONO)
        bitmap = face.glyph.bitmap
        top = face.glyph.bitmap_top
        left = face.glyph.bitmap_left
        advance = face.glyph.advance.x >> 6

        offset_y = max_ascender - top

        buffer = [[0] * max_width for _ in range(max_height)]
        for y in range(bitmap.rows):
            for b in range(bitmap.pitch):
                byte_val = bitmap.buffer[y * bitmap.pitch + b]
                for bit_index in range(min(8, bitmap.width - b * 8)):
                    if byte_val & (1 << (7 - bit_index)):
                        x = b * 8 + bit_index
                        buffer[y + offset_y][x] = 1

        glyph_data.append((ch, buffer, advance, left, top))

        for j in range(32, 127):
            kern = face.get_kerning(i, j, freetype.FT_KERNING_DEFAULT)
            kern_val = kern.x >> 6
            if kern_val != 0:
                kerning_pairs[(chr(i), chr(j))] = kern_val

    # Write header
    with open(out_h_path, 'w') as h:
        guard = f'CHARACTERS_{font_name.upper()}_H'
        h.write(f'#ifndef {guard}\n#define {guard}\n\n')
        h.write(f'int get{font_name}Character(int index, int y);\n')
        h.write(f'int get{font_name}Advance(int index);\n')
        h.write(f'int get{font_name}Kerning(int left, int right);\n')
        h.write(f'#define FONT_{font_name.upper()}_WIDTH {max_width}\n')
        h.write(f'#define FONT_{font_name.upper()}_HEIGHT {max_height}\n\n')
        h.write('#endif\n')

    # Write source
    with open(out_c_path, 'w') as c:
        c.write(f'#include "characters_{font_name}.h"\n\n')

        c.write(f'static const unsigned int font_bitmap[95][FONT_{font_name.upper()}_HEIGHT] = {{\n')
        for ch, bitmap, *_ in glyph_data:
            c.write(f'  /* {repr(ch)} */\n  {{\n')
            for row in bitmap:
                c.write('    0b' + ''.join(str(b) for b in row) + ',\n')
            c.write('  },\n')
        c.write('};\n\n')

        c.write(f'static const unsigned char font_advance[] __attribute__((unused)) = {{\n')
        for _, _, adv, *_ in glyph_data:
            c.write(f'  {adv},\n')
        c.write('};\n\n')

        c.write(f'int get{font_name}Character(int index, int y) {{\n')
        c.write(f'  if (index < 32 || index >= 127 || y >= FONT_{font_name.upper()}_HEIGHT) return 0;\n')
        c.write(f'  return font_bitmap[index - 32][y];\n')
        c.write('}\n\n')

        c.write(f'int get{font_name}Advance(int index) {{\n')
        c.write(f'  if (index < 32 || index >= 127) return 0;\n')
        c.write(f'  return font_advance[index - 32];\n')
        c.write('}\n\n')

        c.write(f'int get{font_name}Kerning(int left, int right) {{\n')
        for (l, r), v in kerning_pairs.items():
            l_escaped = repr(l)[1:-1]
            r_escaped = repr(r)[1:-1]
            c.write(f'  if (left == \'{l_escaped}\' && right == \'{r_escaped}\') return {v};\n')
        c.write('  return 0;\n')
        c.write('}\n')

    print(f"✅ Output saved to {out_c_path} and {out_h_path}")
