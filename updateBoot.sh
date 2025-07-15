#!/bin/bash
set -e

VBR_BIN="$1"
KERNEL_BIN="$2"

if [[ ! -f "$VBR_BIN" || ! -f "$KERNEL_BIN" ]]; then
    echo "Usage: $0 <vbr.bin> <kernel.bin>"
    exit 1
fi

KERNEL_SIZE_BYTES=$(wc -c < "$KERNEL_BIN")
KERNEL_SIZE_SECTORS=$(( (KERNEL_SIZE_BYTES + 511) / 512 ))

SECTOR0=$((KERNEL_SIZE_SECTORS & 0xFF))        # low byte
SECTOR1=$(((KERNEL_SIZE_SECTORS >> 8) & 0xFF)) # high byte

echo "Kernel size: $KERNEL_SIZE_BYTES bytes = $KERNEL_SIZE_SECTORS sectors"
echo "Patch bytes: $SECTOR0 (0x$(printf '%02X' $SECTOR0)) $SECTOR1 (0x$(printf '%02X' $SECTOR1))"

# Patch offset 95 and 96 (zero-based)
printf "\\$(printf '%03o' $SECTOR0)" | dd of="$VBR_BIN" bs=1 seek=95 conv=notrunc 2>/dev/null
printf "\\$(printf '%03o' $SECTOR1)" | dd of="$VBR_BIN" bs=1 seek=96 conv=notrunc 2>/dev/null

echo "✅ VBR updated with kernel size at offset 0x5F (decimal 95)"
