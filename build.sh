#!/bin/bash
CURRENT_DIR=$(pwd)

cd ../../../
set -e

export EDK_TOOLS_PATH=$HOME/edk2/BaseTools
export GCC5_X64_PREFIX=x86_64-elf-
. edksetup.sh BaseTools
build -a X64 -t GCC5 -p MdeModulePkg/MdeModulePkg.dsc -m MdeModulePkg/Application/ViOS64BitDev/ViOS.inf

cd "$CURRENT_DIR"

mkdir -p ./bin ./mnt

# Build the kernel FIRST (before disk operations)
echo "Building kernel..."
cd ./ViOS64Bit
make clean || true
make build
cd ..

# Create the final disk image with GPT structure
dd if=/dev/zero bs=1048576 count=700 of=./bin/os.img

# Create GPT structure
python3 - <<'PYEOF'
import struct
import os
import binascii
import uuid

def crc32(data):
    return binascii.crc32(data) & 0xffffffff

img_path = './bin/os.img'
sector_size = 512
img_size = os.path.getsize(img_path)
total_sectors = img_size // sector_size

# Open the image file
with open(img_path, 'r+b') as f:
    # Write protective MBR (sector 0)
    mbr = bytearray(512)
    mbr[446:446+16] = struct.pack('<B3sB3sLL',
        0x00,           # Status
        b'\x00\x02\x00', # CHS first
        0xEE,           # Type (GPT protective)
        b'\xFF\xFF\xFF', # CHS last
        1,              # LBA first sector
        total_sectors - 1  # LBA number of sectors
    )
    mbr[510:512] = b'\x55\xAA'
    f.write(mbr)

    # Create partition entries first (to calculate CRC)
    partition_entries = bytearray(128 * 128)  # 128 entries of 128 bytes each

    # Partition 1: EFI System Partition
    partition_entries[0:16] = bytes.fromhex('28732ac11ff8d211ba4b00a0c93ec93b')
    partition_entries[16:32] = uuid.uuid4().bytes
    partition_entries[32:40] = struct.pack('<Q', 2048)  # First LBA
    partition_entries[40:48] = struct.pack('<Q', 2048 + 716800 - 1)  # Last LBA
    partition_entries[48:56] = struct.pack('<Q', 0)  # Attributes
    partition_entries[56:56+72] = 'ABC'.encode('utf-16le').ljust(72, b'\x00')

    # Partition 2: Another EFI partition
    offset = 128
    part2_first_lba = 718848
    part2_last_lba = total_sectors - 34  # Last usable LBA
    partition_entries[offset:offset+16] = bytes.fromhex('28732ac11ff8d211ba4b00a0c93ec93b')
    partition_entries[offset+16:offset+32] = uuid.uuid4().bytes
    partition_entries[offset+32:offset+40] = struct.pack('<Q', part2_first_lba)  # First LBA
    partition_entries[offset+40:offset+48] = struct.pack('<Q', part2_last_lba)  # Last LBA
    partition_entries[offset+48:offset+56] = struct.pack('<Q', 0)  # Attributes
    partition_entries[offset+56:offset+56+72] = 'ViOS'.encode('utf-16le').ljust(72, b'\x00')

    # Calculate partition array CRC
    partition_array_crc = crc32(partition_entries)

    # Write GPT header (sector 1)
    f.seek(512)
    gpt_header = bytearray(512)
    gpt_header[0:8] = b'EFI PART'
    gpt_header[8:12] = struct.pack('<I', 0x00010000)  # Revision 1.0
    gpt_header[12:16] = struct.pack('<I', 92)  # Header size
    gpt_header[20:24] = struct.pack('<I', 0)  # Reserved
    gpt_header[24:32] = struct.pack('<Q', 1)  # Current LBA
    gpt_header[32:40] = struct.pack('<Q', total_sectors - 1)  # Backup LBA
    gpt_header[40:48] = struct.pack('<Q', 34)  # First usable LBA
    gpt_header[48:56] = struct.pack('<Q', total_sectors - 34)  # Last usable LBA
    gpt_header[56:72] = uuid.uuid4().bytes  # Disk GUID
    gpt_header[72:80] = struct.pack('<Q', 2)  # Partition entries starting LBA
    gpt_header[80:84] = struct.pack('<I', 128)  # Number of partition entries
    gpt_header[84:88] = struct.pack('<I', 128)  # Size of partition entry
    gpt_header[88:92] = struct.pack('<I', partition_array_crc)  # Partition array CRC

    # Calculate header CRC
    header_crc = crc32(gpt_header[0:92])
    gpt_header[16:20] = struct.pack('<I', header_crc)

    f.write(gpt_header)

    # Write partition entries (starting at sector 2)
    f.seek(1024)
    f.write(partition_entries)

    # Write backup GPT partition entries (at end - 33 sectors)
    backup_entries_lba = total_sectors - 33
    f.seek(backup_entries_lba * 512)
    f.write(partition_entries)

    # Write backup GPT header (at end - 1 sector)
    backup_header = bytearray(512)
    backup_header[0:8] = b'EFI PART'
    backup_header[8:12] = struct.pack('<I', 0x00010000)
    backup_header[12:16] = struct.pack('<I', 92)
    backup_header[20:24] = struct.pack('<I', 0)
    backup_header[24:32] = struct.pack('<Q', total_sectors - 1)  # Current LBA (backup location)
    backup_header[32:40] = struct.pack('<Q', 1)  # Backup LBA (primary location)
    backup_header[40:48] = struct.pack('<Q', 34)
    backup_header[48:56] = struct.pack('<Q', total_sectors - 34)
    backup_header[56:72] = gpt_header[56:72]  # Same disk GUID
    backup_header[72:80] = struct.pack('<Q', backup_entries_lba)  # Backup partition entries LBA
    backup_header[80:84] = struct.pack('<I', 128)
    backup_header[84:88] = struct.pack('<I', 128)
    backup_header[88:92] = struct.pack('<I', partition_array_crc)

    backup_header_crc = crc32(backup_header[0:92])
    backup_header[16:20] = struct.pack('<I', backup_header_crc)

    f.seek((total_sectors - 1) * 512)
    f.write(backup_header)

print("GPT structure created")
PYEOF

# Attach the disk image to format partitions
echo "Attaching disk image to format partitions..."
ATTACH_OUTPUT=$(hdiutil attach -nomount ./bin/os.img)
echo "$ATTACH_OUTPUT"

# Get the base disk device and partition 2
BASE_DISK=$(echo "$ATTACH_OUTPUT" | grep "GUID_partition_scheme" | awk '{print $1}')
PART1_DEV="${BASE_DISK}s1"
PART2_DEV="${BASE_DISK}s2"

echo "Base disk: $BASE_DISK"
echo "Partition 1: $PART1_DEV"
echo "Partition 2: $PART2_DEV"

# Wait for partitions to be recognized and force rescan
sleep 1

# Force macOS to re-read the partition table
echo "Forcing partition table rescan..."
diskutil unmountDisk "$BASE_DISK" 2>/dev/null || true
sleep 1

# Show what diskutil sees
diskutil list "$BASE_DISK"

# Use gpt to verify
echo "Verifying with gpt:"
sudo gpt -r show "$BASE_DISK" | grep "GPT part"

# Check what partitions are visible
echo "Checking available partitions..."
ls -l "${BASE_DISK}"*

# Format the partitions
echo "Formatting partition 1..."
newfs_msdos -F 16 -v ABC "$PART1_DEV"

echo "Formatting partition 2..."
if [ -e "$PART2_DEV" ]; then
    newfs_msdos -F 16 -v ViOS "$PART2_DEV"
else
    echo "Warning: $PART2_DEV does not exist, retrying..."
    sleep 2
    newfs_msdos -F 16 -v ViOS "$PART2_DEV"
fi

# Mount partition 1 immediately after formatting (while disk is still attached)
echo "Mounting partition 1..."
mkdir -p ./mnt
sudo mount -t msdos "$PART1_DEV" ./mnt
MOUNT_POINT="./mnt"

echo "Partition 1 mounted at: $MOUNT_POINT"

# Copy the UEFI bootloader
cp ../../../Build/MdeModule/DEBUG_GCC5/X64/ViOS.efi ./bin/ViOS.efi

# Copy the kernel binary
cp ./ViOS64Bit/bin/kernel.bin ./bin/kernel.bin

# Copy the EFI file and kernel into the filesystem of partition one
sudo mkdir -p "$MOUNT_POINT/EFI/BOOT"
sudo cp ./bin/ViOS.efi "$MOUNT_POINT/EFI/Boot/BOOTX64.efi"
sudo cp ./bin/kernel.bin "$MOUNT_POINT/kernel.bin"

echo "Copied bootloader to $MOUNT_POINT/EFI/Boot/BOOTX64.efi"

# Unmount partition 1
sudo umount ./mnt

# Now mount partition 2 and copy kernel data there
echo "Mounting partition 2 for kernel filesystem..."
if sudo mount -t msdos "$PART2_DEV" ./mnt; then
    echo "✓ Partition 2 mounted successfully at ./mnt"
    echo "Contents before copy:"
    ls -la ./mnt/

    # Copy background image and other kernel files to partition 2
    if [ -f ./ViOS64Bit/data/images/bkground.bmp ]; then
        echo "Found source file: ./ViOS64Bit/data/images/bkground.bmp"
        ls -lh ./ViOS64Bit/data/images/bkground.bmp
        if sudo cp -v ./ViOS64Bit/data/images/bkground.bmp ./mnt/; then
            echo "✓ BMP copied successfully"
            echo "Contents after copy:"
            ls -lh ./mnt/bkground.bmp
        else
            echo "✗ Failed to copy BMP (error code: $?)"
            sudo umount ./mnt
            hdiutil detach "$BASE_DISK"
            exit 1
        fi
    else
        echo "✗ Source BMP file not found at ./ViOS64Bit/data/images/bkground.bmp"
    fi

    # Copy sysfont.bmp to partition 2
    if [ -f ./ViOS64Bit/data/images/fonts/sysfont.bmp ]; then
        echo "Found source file: ./ViOS64Bit/data/images/fonts/sysfont.bmp"
        ls -lh ./ViOS64Bit/data/images/fonts/sysfont.bmp
        if sudo cp -v ./ViOS64Bit/data/images/fonts/sysfont.bmp ./mnt/; then
            echo "✓ sysfont.bmp copied successfully"
            echo "Contents after copy:"
            ls -lh ./mnt/sysfont.bmp
        else
            echo "✗ Failed to copy sysfont.bmp (error code: $?)"
        fi
    else
        echo "✗ Source sysfont.bmp file not found at ./ViOS64Bit/data/images/fonts/sysfont.bmp"
    fi

    # Copy shell.elf if it exists
    if [ -f ./ViOS64Bit/assets/shell/shell.elf ]; then
        echo "Found source file: ./ViOS64Bit/assets/shell/shell.elf"
        if sudo cp -v ./ViOS64Bit/assets/shell/shell.elf ./mnt/; then
            echo "✓ shell.elf copied successfully"
        else
            echo "✗ Failed to copy shell.elf (error code: $?)"
        fi
    else
        echo "Note: shell.elf not found (optional)"
    fi

    if [ -f ./ViOS64Bit/assets/blank/blank.elf ]; then
        echo "Found source file: ./ViOS64Bit/assets/blank/blank.elf"
        if sudo cp -v ./ViOS64Bit/assets/blank/blank.elf ./mnt/; then
            echo "✓ blank.elf copied successfully"
        else
            echo "✗ Failed to copy blank.elf (error code: $?)"
        fi
    else
        echo "Note: blank.elf not found (optional)"
    fi

    # Show final contents
    echo "Final contents of partition 2:"
    ls -lah ./mnt/

    # Force sync before unmounting
    echo "Syncing filesystem..."
    sync
    sleep 1

    # Cleanup
    echo "Unmounting partition 2..."
    if sudo umount ./mnt; then
        echo "✓ Partition 2 unmounted successfully"
    else
        echo "✗ Failed to unmount partition 2 (error code: $?)"
    fi
else
    echo "✗ Failed to mount partition 2 at $PART2_DEV"
    hdiutil detach "$BASE_DISK"
    exit 1
fi

echo "Detaching disk..."
hdiutil detach "$BASE_DISK"

echo "Build completed"
