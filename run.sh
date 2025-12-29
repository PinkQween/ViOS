#!/bin/bash

# Check if debug mode is requested
DEBUG=false
if [[ "$1" == "--debug" || "$1" == "-d" ]]; then
    DEBUG=true
fi

if [ "$DEBUG" = true ]; then
    echo "Starting QEMU in debug mode..."
    echo "In another terminal, run: gdb ./ViOS64Bit/bin/kernel.elf"
    echo "Then in GDB, type: target remote localhost:1234"
    echo "Then: continue"
    echo ""

    qemu-system-x86_64 \
      -machine pc \
      -drive file=./bin/os.img,format=raw,if=ide \
      -m 512M \
      -cpu qemu64 \
      -bios /opt/homebrew/share/qemu/RELEASEX64_OVMF.fd \
      -serial stdio \
      -s -S
else
    qemu-system-x86_64 \
      -machine pc \
      -drive file=./bin/os.img,format=raw,if=ide \
      -m 512M \
      -cpu qemu64 \
      -bios /opt/homebrew/share/qemu/RELEASEX64_OVMF.fd \
      -serial stdio
fi