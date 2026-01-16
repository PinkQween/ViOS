#!/bin/bash
set -e

DISK_IMAGE="$1"
OUTPUT_DIR="$2"
VIOS64BIT_DIR="$3"

# Create loop device
LOOP_DEVICE=$(sudo losetup -f --show "$DISK_IMAGE")
sudo partprobe "$LOOP_DEVICE"
sleep 1

# Format partitions
sudo mkfs.vfat -F 16 -n ABC "${LOOP_DEVICE}p1" >/dev/null 2>&1
sudo mkfs.vfat -F 16 -n VIOS "${LOOP_DEVICE}p2" >/dev/null 2>&1

# Mount and populate partition 1
MNT_DIR=$(mktemp -d)
sudo mount "${LOOP_DEVICE}p1" "$MNT_DIR"
sudo mkdir -p "$MNT_DIR/EFI/BOOT"
sudo cp "$OUTPUT_DIR/ViOS.efi" "$MNT_DIR/EFI/BOOT/BOOTX64.efi"
sudo cp "$OUTPUT_DIR/kernel.bin" "$MNT_DIR/kernel.bin"
sudo umount "$MNT_DIR"

# Mount and populate partition 2
sudo mount "${LOOP_DEVICE}p2" "$MNT_DIR"
[ -f "$VIOS64BIT_DIR/data/images/bkground.bmp" ] && sudo cp "$VIOS64BIT_DIR/data/images/bkground.bmp" "$MNT_DIR/"
[ -f "$VIOS64BIT_DIR/data/images/clsicon.bmp" ] && sudo cp "$VIOS64BIT_DIR/data/images/clsicon.bmp" "$MNT_DIR/"
[ -f "$VIOS64BIT_DIR/data/images/fonts/sysfont.bmp" ] && sudo cp "$VIOS64BIT_DIR/data/images/fonts/sysfont.bmp" "$MNT_DIR/"
find "$VIOS64BIT_DIR/assets" -name "*.elf" -exec sudo cp {} "$MNT_DIR/" \; 2>/dev/null || true
sudo umount "$MNT_DIR"

# Cleanup
sudo losetup -d "$LOOP_DEVICE"
rmdir "$MNT_DIR"
