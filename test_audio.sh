#!/bin/bash

# Test script for ViOS with audio support
echo "Starting ViOS with audio support..."

# Try different audio configurations for macOS
echo "Testing with coreaudio..."
qemu-system-i386 \
  -drive file=bin/os.bin,format=raw,if=floppy \
  -audiodev coreaudio,id=audio0 \
  -device sb16,audiodev=audio0 \
  -machine pcspk-audiodev=audio0 \
  -monitor stdio \
  -no-reboot -no-shutdown

echo "Audio test completed."
