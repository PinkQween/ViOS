#!/bin/bash

# Exit on any error, undefined variables, and pipe failures
set -euo pipefail

# === Config ===
export PREFIX="$HOME/opt/cross"
export TARGET=i686-elf
export PATH="$PREFIX/bin:$PATH"

# === Banner ===
echo "=================================="
echo "  Setting up $TARGET toolchain"
echo "  Prefix: $PREFIX"
echo "=================================="

# === Dependency Installer ===
install_deps() {
    echo "[*] Installing required packages..."
    sudo apt update
    sudo apt install -y \
        unzip \
        build-essential \
        bison \
        flex \
        libgmp-dev \
        libmpc-dev \
        libmpfr-dev \
        texinfo \
        wget \
        curl \
        git \
        gawk \
        xz-utils \
        nasm \
        python3 \
        python3-pip \
        python3-dev \
        libfreetype6-dev

    # Install Python FreeType bindings for Python3
    pip3 install --user freetype-py
}

# === Download + Build Binutils ===
install_binutils() {
    echo "[*] Installing Binutils..."
    mkdir -p "$HOME/src"
    cd "$HOME/src"

    if [ ! -f binutils-2.42.tar.xz ]; then
        wget https://ftp.gnu.org/gnu/binutils/binutils-2.42.tar.xz
    fi

    tar -xf binutils-2.42.tar.xz
    mkdir -p build-binutils
    cd build-binutils
    ../binutils-2.42/configure \
        --target="$TARGET" \
        --prefix="$PREFIX" \
        --with-sysroot \
        --disable-nls \
        --disable-werror
    make -j"$(nproc)"
    make install
}

# === Download + Build GCC ===
install_gcc() {
    echo "[*] Installing GCC"
    sudo dpkg --add-architecture i386
    sudo apt-get update
    # Download prebuilt i686-elf toolchain (lordmilko)
    echo "[!] WARNING: Downloading prebuilt toolchain. Verify source integrity."
    echo "[*] Source: https://github.com/lordmilko/i686-elf-tools"
    read -p "Continue? (y/N): " -n 1 -r
    echo
    if [[ ! $REPLY =~ ^[Yy]$ ]]; then
        echo "Aborted by user."
        exit 1
    fi

    wget https://github.com/lordmilko/i686-elf-tools/releases/download/13.2.0/i686-elf-tools-linux.zip
    unzip i686-elf-tools-linux.zip -d i686-elf-tools
    sudo mv i686-elf-tools /usr/local/i686-elf-tools
    sudo ln -sf /usr/local/i686-elf-tools/bin/* /usr/local/bin/
}

# === Main ===

# Check if necessary tools are available
missing_tools=()
for tool in nasm make gcc g++ bison flex makeinfo python3 python3-pip; do
    if ! command -v "$tool" &>/dev/null; then
        missing_tools+=("$tool")
    fi
done

if [ ${#missing_tools[@]} -gt 0 ]; then
    echo "[!] Missing tools: ${missing_tools[*]}"
    install_deps
fi

# Check and install cross-compiler
if ! command -v "${TARGET}-gcc" &>/dev/null; then
    install_gcc
else
    echo "[✓] ${TARGET}-gcc already installed."
fi

if [[ "$(uname -s)" == "Linux" ]]; then
    # Check for mformat (mtools)
    if ! command -v mformat &>/dev/null; then
        echo "[*] mformat (from mtools) is required but not installed. Attempting to install..."
        if command -v apt &>/dev/null; then
            sudo apt update && sudo apt install -y mtools
        elif command -v dnf &>/dev/null; then
            sudo dnf install -y mtools
        elif command -v pacman &>/dev/null; then
            sudo pacman -Sy --noconfirm mtools
        else
            echo "[!] Could not detect package manager. Please install mtools manually."
            exit 1
        fi
    fi
else
    echo "[*] Not running on Linux, skipping mtools installation check."
fi

# === Build Your Kernel ===
echo "[*] Running your project Makefile..."
make clean
make all