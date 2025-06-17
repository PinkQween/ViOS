from PIL import Image

# Load and resize the image
img = Image.open("ViOS_LOGO_PNG.png").resize((320, 200)).convert("P")

# Save the raw pixel indices (framebuffer data)
with open("assets/logo.bin", "wb") as f:
    f.write(img.tobytes())

# Extract the palette (768 bytes: 256 colors × 3 RGB channels)
palette = img.getpalette()[:256*3]  # Take only first 256 colors

# VGA palette uses 6-bit color, so scale 0-255 range to 0-63
vga_palette = bytearray(int(c / 4) for c in palette)

# Save the palette data
with open("assets/logo.pal", "wb") as f:
    f.write(vga_palette)
