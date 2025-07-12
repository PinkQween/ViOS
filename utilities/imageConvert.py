#!/usr/bin/env python3

from PIL import Image

TARGET_WIDTH, TARGET_HEIGHT = 320, 200

# Load image
img = Image.open("ViOS_LOGO_PNG.png").convert("RGBA")  # Load with alpha

# Maintain aspect ratio and resize with high-quality Lanczos filter
img_ratio = img.width / img.height
target_ratio = TARGET_WIDTH / TARGET_HEIGHT

if img_ratio > target_ratio:
    new_width = TARGET_WIDTH
    new_height = round(TARGET_WIDTH / img_ratio)
else:
    new_height = TARGET_HEIGHT
    new_width = round(TARGET_HEIGHT * img_ratio)

resized_img = img.resize((new_width, new_height), Image.LANCZOS)

# Create new black background image
final_img = Image.new("RGBA", (TARGET_WIDTH, TARGET_HEIGHT), (0, 0, 0, 255))

# Paste resized image centered, respecting alpha transparency
paste_x = (TARGET_WIDTH - new_width) // 2
paste_y = (TARGET_HEIGHT - new_height) // 2
final_img.paste(resized_img, (paste_x, paste_y), resized_img)

# Convert to P mode (256 colors) using adaptive palette
final_img_p = final_img.convert("RGB").convert("P", palette=Image.ADAPTIVE, colors=256)

# Save raw pixel data (palette indices)
with open("assets/logo.bin", "wb") as f:
    f.write(final_img_p.tobytes())

# Extract palette and convert from 0–255 to VGA 0–63 scale
palette = final_img_p.getpalette()[:256 * 3]  # First 256 RGB triplets
vga_palette = bytearray(round(c * 63 / 255) for c in palette)

# Save VGA-compatible palette (768 bytes)
with open("assets/logo.pal", "wb") as f:
    f.write(vga_palette)
