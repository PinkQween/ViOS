from PIL import Image
import os
import struct

def load_and_save_rgb(filepath: str = "../logo.png", output_path: str = "../assets/sys/assets/logo.rgb"):
    print(f"Starting with {filepath} -> {output_path}")

    if not os.path.exists(filepath):
        raise FileNotFoundError(f"Input file not found: {filepath}")

    with Image.open(filepath) as img:
        img = img.convert('RGB')
        width, height = img.size
        pixels = img.tobytes()

    dir_name = os.path.dirname(output_path)
    if dir_name:
        os.makedirs(dir_name, exist_ok=True)

    with open(output_path, 'wb') as f:
        f.write(struct.pack('<I', width))
        f.write(struct.pack('<I', height))
        f.write(pixels)

    print(f"Saved raw RGB data ({width}x{height}) to {output_path}")
    return width, height

if __name__ == "__main__":
    load_and_save_rgb()
