#!/bin/bash

# Get kernel size in bytes
KERNEL_SIZE_BYTES=$(wc -c < ./bin/kernel.bin)
KERNEL_SIZE_SECTORS=$((KERNEL_SIZE_BYTES / 512))
if [ $((KERNEL_SIZE_BYTES % 512)) -ne 0 ]; then
    KERNEL_SIZE_SECTORS=$((KERNEL_SIZE_SECTORS + 1))
fi

echo "Kernel size: $KERNEL_SIZE_BYTES bytes ($KERNEL_SIZE_SECTORS sectors)"

# Copy boot.bin to boot_with_size.bin
cp ./bin/boot.bin ./bin/boot_with_size.bin

# Calculate individual bytes
BYTE0=$((KERNEL_SIZE_BYTES & 0xFF))
BYTE1=$(((KERNEL_SIZE_BYTES >> 8) & 0xFF))
BYTE2=$(((KERNEL_SIZE_BYTES >> 16) & 0xFF))
BYTE3=$(((KERNEL_SIZE_BYTES >> 24) & 0xFF))

SECTOR0=$((KERNEL_SIZE_SECTORS & 0xFF))
SECTOR1=$(((KERNEL_SIZE_SECTORS >> 8) & 0xFF))

echo "Bytes: $BYTE0 $BYTE1 $BYTE2 $BYTE3"
echo "Sectors: $SECTOR0 $SECTOR1"

# Write bytes using printf and dd
printf "\\$(printf '%03o' $BYTE0)" | dd of=./bin/boot_with_size.bin bs=1 seek=62 conv=notrunc 2>/dev/null
printf "\\$(printf '%03o' $BYTE1)" | dd of=./bin/boot_with_size.bin bs=1 seek=63 conv=notrunc 2>/dev/null
printf "\\$(printf '%03o' $BYTE2)" | dd of=./bin/boot_with_size.bin bs=1 seek=64 conv=notrunc 2>/dev/null
printf "\\$(printf '%03o' $BYTE3)" | dd of=./bin/boot_with_size.bin bs=1 seek=65 conv=notrunc 2>/dev/null

printf "\\$(printf '%03o' $SECTOR0)" | dd of=./bin/boot_with_size.bin bs=1 seek=66 conv=notrunc 2>/dev/null
printf "\\$(printf '%03o' $SECTOR1)" | dd of=./bin/boot_with_size.bin bs=1 seek=67 conv=notrunc 2>/dev/null

echo "Boot sector updated!" 