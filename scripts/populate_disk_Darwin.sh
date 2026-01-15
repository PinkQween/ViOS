#!/bin/bash
set -e

DISK_IMAGE="$1"
OUTPUT_DIR="$2"
VIOS64BIT_DIR="$3"

# Attach disk image
ATTACH_OUTPUT=$(hdiutil attach -nomount "$DISK_IMAGE" 2>/dev/null)
BASE_DISK=$(echo "$ATTACH_OUTPUT" | grep "GUID_partition_scheme" | awk '{print $1}')
PART1_DEV="${BASE_DISK}s1"
PART2_DEV="${BASE_DISK}s2"

# Format partitions
newfs_msdos -F 16 -v ABC "$PART1_DEV" >/dev/null 2>&1
newfs_msdos -F 16 -v ViOS "$PART2_DEV" >/dev/null 2>&1

# Mount and populate partition 1
MNT_DIR=$(mktemp -d)
sudo mount -t msdos "$PART1_DEV" "$MNT_DIR"
sudo mkdir -p "$MNT_DIR/EFI/BOOT"
sudo cp "$OUTPUT_DIR/ViOS.efi" "$MNT_DIR/EFI/BOOT/BOOTX64.efi"
sudo cp "$OUTPUT_DIR/kernel.bin" "$MNT_DIR/kernel.bin"
sudo umount "$MNT_DIR"

# Mount and populate partition 2
sudo mount -t msdos "$PART2_DEV" "$MNT_DIR"
[ -f "$VIOS64BIT_DIR/data/images/bkground.bmp" ] && sudo cp "$VIOS64BIT_DIR/data/images/bkground.bmp" "$MNT_DIR/"
[ -f "$VIOS64BIT_DIR/data/images/clsicon.bmp" ] && sudo cp "$VIOS64BIT_DIR/data/images/clsicon.bmp" "$MNT_DIR/"
[ -f "$VIOS64BIT_DIR/data/images/fonts/sysfont.bmp" ] && sudo cp "$VIOS64BIT_DIR/data/images/fonts/sysfont.bmp" "$MNT_DIR/"
find "$VIOS64BIT_DIR/assets" -name "*.elf" -exec sudo cp {} "$MNT_DIR/" \; 2>/dev/null || true
sync
sudo umount "$MNT_DIR"

# Cleanup
hdiutil detach "$BASE_DISK" >/dev/null 2>&1
rmdir "$MNT_DIR"
