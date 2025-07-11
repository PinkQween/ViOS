#!/bin/bash

# Default options
AUDIO=true
DEBUG=false

# Parse arguments
for arg in "$@"; do
    case $arg in
        -a|--no-audio)
            AUDIO=false
            ;;
        -d|--debug)
            DEBUG=true
            ;;
        *)
            echo "Unknown option: $arg"
            exit 1
            ;;
    esac
done

# Base QEMU command
QEMU_CMD="qemu-system-i386 -m 512M -drive file=bin/os.bin,if=ide,index=0,media=disk,format=raw"

# Audio option
if [ "$AUDIO" = true ]; then
    QEMU_CMD="$QEMU_CMD -audiodev coreaudio,id=audio0 -machine pcspk-audiodev=audio0"
fi

# Debug option
if [ "$DEBUG" = true ]; then
    QEMU_CMD="$QEMU_CMD -s -S"
    echo "Run this in another terminal: gdb bin/kernel.elf -ex 'target remote localhost:1234'"
fi

# Run QEMU
eval "$QEMU_CMD"