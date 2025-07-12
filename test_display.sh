#!/bin/bash

echo "Starting ViOS with screenshot capability..."

# Run QEMU in background with QMP monitor
qemu-system-i386 \
    -drive file=./bin/os.bin,format=raw,index=0,media=disk \
    -m 128M \
    -vga std \
    -serial file:debug.log \
    -qmp unix:/tmp/qemu-monitor.sock,server,nowait &

QEMU_PID=$!
echo "QEMU PID: $QEMU_PID"

# Wait for QEMU to start
sleep 3

# Try to take a screenshot using QMP
echo "Attempting to take screenshot..."
echo '{"execute": "qmp_capabilities"}' | socat - UNIX-CONNECT:/tmp/qemu-monitor.sock
echo '{"execute": "screendump", "arguments": {"filename": "screenshot.ppm"}}' | socat - UNIX-CONNECT:/tmp/qemu-monitor.sock

# Wait a bit more for the screenshot
sleep 2

# Kill QEMU
echo "Stopping QEMU..."
kill $QEMU_PID 2>/dev/null
wait $QEMU_PID 2>/dev/null

# Check if screenshot was created
if [ -f screenshot.ppm ]; then
    echo "Screenshot saved as screenshot.ppm"
    ls -la screenshot.ppm
else
    echo "No screenshot was created"
fi

# Clean up
rm -f /tmp/qemu-monitor.sock

echo "Debug log contents:"
cat debug.log
