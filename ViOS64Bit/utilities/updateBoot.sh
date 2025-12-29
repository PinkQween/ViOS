#!/bin/bash
# Update boot sector with kernel size

# Get kernel size in bytes
KERNEL_SIZE=$(stat -f%z ./bin/kernel.bin 2>/dev/null || stat -c%s ./bin/kernel.bin 2>/dev/null)

if [ -z "$KERNEL_SIZE" ]; then
    echo "Error: Could not determine kernel size"
    exit 1
fi

# Calculate size in sectors (512 bytes per sector, round up)
SECTORS=$(( (KERNEL_SIZE + 511) / 512 ))

echo "Kernel size: $KERNEL_SIZE bytes ($SECTORS sectors)"

# Copy boot.bin to boot_with_size.bin
cp ./bin/boot.bin ./bin/boot_with_size.bin

# Write the sector count at offset 510 (2 bytes before boot signature)
# Using printf and dd to write the 16-bit value in little-endian format
printf "\\x$(printf '%02x' $((SECTORS & 0xFF)))\\x$(printf '%02x' $((SECTORS >> 8)))" | \
    dd of=./bin/boot_with_size.bin bs=1 seek=508 count=2 conv=notrunc 2>/dev/null

echo "Boot sector updated with kernel size"
