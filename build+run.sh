#!/bin/bash

# Default paths
DEFAULT_LIBC_PATH="../ViOS libc"
DEFAULT_VIOS_PATH="../ViOS"

if [ "$1" = "-d" ]; then
    LIBC_PATH="$DEFAULT_LIBC_PATH"
    VIOS_PATH="$DEFAULT_VIOS_PATH"
elif [ -n "$1" ]; then
    LIBC_PATH="$1"
    VIOS_PATH="${2:-$DEFAULT_VIOS_PATH}"  # Use second arg or default
else
    # No args, skip libc install
    cd "$DEFAULT_VIOS_PATH" || exit 1
    ./build.sh || exit 1
    ./run.sh || exit 1
    exit 0
fi

# If we're here, we need to install libc first
if [ -d "$LIBC_PATH" ]; then
    echo "Installing libc from: $LIBC_PATH"
    cd "$LIBC_PATH" || exit 1
    sudo make install || exit 1
else
    echo "Error: Libc path '$LIBC_PATH' does not exist."
    exit 1
fi

# Then go to ViOS path
if [ -d "$VIOS_PATH" ]; then
    echo "Changing to ViOS path: $VIOS_PATH"
    cd "$VIOS_PATH" || exit 1
else
    echo "Error: ViOS path '$VIOS_PATH' does not exist."
    exit 1
fi

# Build and run
echo "Building ViOS..."
./build.sh || exit 1

echo "Running ViOS..."
./run.sh || exit 1
