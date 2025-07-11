#!/bin/bash

# ViOS Alternative Runner Script for macOS
# This script tries different QEMU options for better compatibility

echo "Starting ViOS on macOS (Alternative mode)..."
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
    echo "Error: os.bin not found."
    exit 1
fi

echo "Trying different QEMU configurations..."
echo ""

# Try option 1: Basic VGA with serial console output
echo "=== Trying Option 1: Basic VGA with serial ==="
qemu-system-i386 \
    -hda ./bin/os.bin \
    -m 512M \
    -vga std \
    -no-reboot \
    -no-shutdown \
    -display cocoa \
    -serial stdio &

QEMU_PID=$!
sleep 3

# Check if QEMU is still running
if ! kill -0 $QEMU_PID 2>/dev/null; then
    echo "Option 1 failed, trying Option 2..."
    echo ""
    
    # Try option 2: SDL display with VNC fallback
    echo "=== Trying Option 2: SDL display ==="
    qemu-system-i386 \
        -hda ./bin/os.bin \
        -m 512M \
        -vga std \
        -display sdl \
        -no-reboot \
        -no-shutdown \
        -serial stdio &
    
    QEMU_PID=$!
    sleep 3
    
    if ! kill -0 $QEMU_PID 2>/dev/null; then
        echo "Option 2 failed, trying Option 3..."
        echo ""
        
        # Try option 3: VNC display (most compatible)
        echo "=== Trying Option 3: VNC display ==="
        echo "Connect with VNC viewer to localhost:5901"
        qemu-system-i386 \
            -hda ./bin/os.bin \
            -m 512M \
            -vga std \
            -vnc :1 \
            -no-reboot \
            -no-shutdown \
            -serial stdio
    else
        echo "Option 2 is running. Wait for it to finish or press Ctrl+C to stop."
        wait $QEMU_PID
    fi
else
    echo "Option 1 is running. Wait for it to finish or press Ctrl+C to stop."
    wait $QEMU_PID
fi

echo ""
echo "ViOS session ended."
