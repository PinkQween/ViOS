#!/usr/bin/env python3
import sys
import struct
import os

# Constants
SECTOR_SIZE = 512
DEFAULT_PARTITION_START = 2048
DEFAULT_PARTITION_TYPE = 0x0C  # FAT32 LBA
DEFAULT_IMAGE_SIZE_MB = 128


def write_blank_image(path, size_mb):
    size_bytes = size_mb * 1024 * 1024
    with open(path, 'wb') as f:
        f.truncate(size_bytes)
    print(f"Created blank image: {path} ({size_mb} MB)")

def create_mbr(partition_start, partition_size, partition_type):
    mbr = bytearray([0x90] * SECTOR_SIZE)  # Fill with NOPs for minimal boot code
    part_offset = 446
    # CHS start: head=0, sector=2, cylinder=0 => 0x00 0x02 0x00
    chs_start = b'\x00\x02\x00'
    # CHS end: head=254, sector=63, cylinder=1023 => 0xFE 0xFF 0xFF
    chs_end = b'\xFE\xFF\xFF'
    entry = struct.pack('<B3sB3sII',
        0x80,                # bootable
        chs_start,           # CHS start
        partition_type,      # type
        chs_end,             # CHS end
        partition_start,     # LBA start
        partition_size       # total sectors
    )
    mbr[part_offset:part_offset+16] = entry
    mbr[510:512] = b'\x55\xAA'
    return mbr

def write_mbr(image_path, mbr_bytes):
    with open(image_path, 'r+b') as f:
        f.seek(0)
        f.write(mbr_bytes)
    print(f"Wrote MBR to {image_path}")

def write_vbr(image_path, vbr_path, partition_start):
    with open(image_path, 'r+b') as img, open(vbr_path, 'rb') as vbr:
        img.seek(partition_start * SECTOR_SIZE)
        img.write(vbr.read(SECTOR_SIZE))
    print(f"Wrote VBR from {vbr_path} to sector {partition_start}")

def main():
    import argparse
    parser = argparse.ArgumentParser(description="Create a raw image with MBR and a single FAT32 partition.")
    parser.add_argument('image', help='Path to image file')
    parser.add_argument('--size', type=int, default=128, help='Image size in MB (default: 128)')
    parser.add_argument('--start', type=int, default=2048, help='Partition start sector (default: 2048)')
    parser.add_argument('--type', type=lambda x: int(x,0), default=0x0C, help='Partition type (default: 0x0C FAT32 LBA)')
    args = parser.parse_args()

    if not os.path.exists(args.image):
        write_blank_image(args.image, args.size)
    total_sectors = (args.size * 1024 * 1024) // 512
    partition_size = total_sectors - args.start
    mbr = create_mbr(args.start, partition_size, args.type)
    write_mbr(args.image, mbr)

if __name__ == '__main__':
    main() 