#!/bin/bash
set -euo pipefail

# === Config ===
export PREFIX="$HOME/opt/cross"
export TARGET=i686-elf
export PATH="$PREFIX/bin:$PATH"
export VENV_DIR="/ubuntu/ViOS-venv"

echo "=================================="
echo "  Setting up $TARGET toolchain"
echo "  Prefix: $PREFIX"
echo "=================================="

install_deps() {
    echo "[*] Installing required system packages..."
    sudo apt update
    sudo apt install -y unzip build-essential bison flex libgmp-dev libmpc-dev libmpfr-dev texinfo \
        wget curl git gawk xz-utils nasm python3 python3-venv libfreetype6-dev mtools
}

install_gcc() {
    echo "[*] Installing GCC cross-toolchain from lordmilko prebuilt binaries..."

    sudo dpkg --add-architecture i386
    sudo apt-get update

    echo "[!] WARNING: Downloading prebuilt toolchain. Verify source integrity."
    echo "[*] Source: https://github.com/lordmilko/i686-elf-tools"
    read -p "Continue? (y/N): " -n 1 -r
    echo
    if [[ ! $REPLY =~ ^[Yy]$ ]]; then
        echo "Aborted by user."
        exit 1
    fi

    cd /tmp
    wget -N https://github.com/lordmilko/i686-elf-tools/releases/download/13.2.0/i686-elf-tools-linux.zip
    unzip -o i686-elf-tools-linux.zip -d i686-elf-tools
    sudo mv -f i686-elf-tools /usr/local/i686-elf-tools
    sudo ln -sf /usr/local/i686-elf-tools/bin/* /usr/local/bin/
}

check_and_install() {
    # Check if target gcc is installed
    if ! command -v "${TARGET}-gcc" &>/dev/null; then
        install_gcc
    else
        echo "[✓] ${TARGET}-gcc already installed."
    fi

    # Check required tools
    missing_tools=()
    for tool in nasm make gcc g++; do
        if ! command -v "$tool" &>/dev/null; then
            missing_tools+=("$tool")
        fi
    done

    if [ ${#missing_tools[@]} -gt 0 ]; then
        echo "[!] Missing tools: ${missing_tools[*]}"
        if [[ "$(uname -s)" == "Linux" ]]; then
            install_deps
        else
            echo "[!] Please install missing tools manually for your system."
            echo "[*] On macOS, you can use: brew install nasm make gcc python3"
            exit 1
        fi
    else
        echo "[✓] All required tools are installed."
    fi

    # Check mtools/mformat presence on Linux
    if [[ "$(uname -s)" == "Linux" ]]; then
        if ! command -v mformat &>/dev/null; then
            echo "[*] mformat (mtools) missing, installing..."
            if command -v apt &>/dev/null; then
                sudo apt update && sudo apt install -y mtools
            elif command -v dnf &>/dev/null; then
                sudo dnf install -y mtools
            elif command -v pacman &>/dev/null; then
                sudo pacman -Sy --noconfirm mtools
            else
                echo "[!] Could not detect package manager for mtools. Please install manually."
                exit 1
            fi
        else
            echo "[✓] mformat is installed."
        fi
    else
        echo "[*] Not running Linux, skipping mtools check."
    fi
}

run_make() {
    echo "[*] Running your project Makefile..."

    make clean
    make all
}

# === Main ===
check_and_install
run_make
