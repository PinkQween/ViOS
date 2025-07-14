#!/bin/bash

# Default options
AUDIO=true
DEBUG=false
SERIAL=true

# Parse arguments
for arg in "$@"; do
    case $arg in
        -a|--no-audio)
            AUDIO=false
            ;;
        -d|--debug)
            DEBUG=true
            ;;
        -s|--no-serial)
            SERIAL=false
            ;;
        *)
            echo "Unknown option: $arg"
            exit 1
            ;;
    esac
done

# Base QEMU command
QEMU_CMD="qemu-system-i386 -m 512M -drive file=bin/os.bin,if=ide,index=0,media=disk,format=raw"

# Serial option
if [ "$SERIAL" = true ]; then
    QEMU_CMD="$QEMU_CMD -serial stdio"
fi

# Audio option
if [ "$AUDIO" = true ]; then
    # Use working audio configuration with proper format settings
    QEMU_CMD="$QEMU_CMD -audiodev coreaudio,id=audio0,out.frequency=44100,out.channels=2,out.format=s16 -machine pcspk-audiodev=audio0"
fi

# Debug option
if [ "$DEBUG" = true ]; then
    QEMU_CMD="$QEMU_CMD -s -S"
    echo "Run this in another terminal: gdb bin/kernel.elf -ex 'target remote localhost:1234'"
fi

# Run QEMU
eval "$QEMU_CMD"