#!/bin/bash

# ViOS Runner Script for macOS
# This script runs ViOS using QEMU emulation

echo "Starting ViOS on macOS..."
echo "Press Ctrl+A then X to quit QEMU"
echo ""

# Check if QEMU is installed
if ! command -v qemu-system-i386 &> /dev/null; then
    echo "Error: QEMU is not installed or not in PATH"
    echo "Install with: brew install qemu"
    exit 1
fi

# Check if os.bin exists
if [ ! -f "./bin/os.bin" ]; then
    echo "Error: os.bin not found. Run 'make all' first to build ViOS"
    exit 1
fi

# Run ViOS with QEMU
# Options explained:
# -m 512M         : Allocate 512MB RAM
# -no-reboot      : Exit QEMU instead of rebooting
# -no-shutdown    : Don't shutdown automatically
# -serial stdio   : Connect serial port to terminal (for debugging)
# -monitor telnet::45454,server,nowait : Allow monitor access via telnet
qemu-system-i386 \
    -hda ./bin/os.bin \
    -m 512M \
    -no-reboot \
    -no-shutdown \
    -serial stdio

echo ""
echo "ViOS session ended."
