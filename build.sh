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

check_and_install_vios_libc() {
    echo "[*] Checking for ViOS standard library..."
    
    # Set the ViOS library path to be in external/ViOS-libc
    VIOS_LIB_PATH="$(pwd)/external/ViOS-libc"
    
    # Check if the library directory exists and contains the expected files
    if [[ -d "$VIOS_LIB_PATH" ]] && [[ -f "$VIOS_LIB_PATH/lib/libViOSlibc.a" ]] && [[ -d "$VIOS_LIB_PATH/include" ]]; then
        echo "[✓] ViOS standard library found at $VIOS_LIB_PATH"
        return 0
    else
        echo "[!] ViOS standard library not found at $VIOS_LIB_PATH"
        echo "[*] Installing ViOS standard library locally..."
        
        # Create temporary directory for cloning
        TEMP_DIR=$(mktemp -d)
        cd "$TEMP_DIR"
        
        # Clone the repository
        echo "[*] Cloning ViOS-Libc repository..."
        if ! git clone https://github.com/PinkQween/ViOS-Libc.git; then
            echo "[!] Failed to clone ViOS-Libc repository"
            rm -rf "$TEMP_DIR"
            exit 1
        fi
        
        cd ViOS-Libc
        
        # Build the library
        echo "[*] Building ViOS standard library..."
        if ! make all; then
            echo "[!] Failed to build ViOS standard library"
            rm -rf "$TEMP_DIR"
            exit 1
        fi
        
        # Create the external libc directory structure
        echo "[*] Installing ViOS standard library locally..."
        mkdir -p "$VIOS_LIB_PATH/lib"
        mkdir -p "$VIOS_LIB_PATH/include"
        
        # Copy the built library and headers to the external directory
        if [[ -f "build/libViOSlibc.a" ]]; then
            cp build/libViOSlibc.a "$VIOS_LIB_PATH/lib/"
        else
            echo "[!] Built library file not found"
            rm -rf "$TEMP_DIR"
            exit 1
        fi
        
        # Copy headers if they exist
        if [[ -d "include" ]]; then
            cp -r include/* "$VIOS_LIB_PATH/include/"
        elif [[ -d "src" ]]; then
            # Look for header files in src directory
            find src -name "*.h" -exec cp --parents {} "$VIOS_LIB_PATH/include/" \;
        fi
        
        # Clean up
        cd /
        rm -rf "$TEMP_DIR"
        
        echo "[✓] ViOS standard library installed successfully at $VIOS_LIB_PATH"
    fi
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

clean_vios_libc() {
    echo "[*] Cleaning ViOS libc installation..."
    if [[ -d "external/ViOS-libc" ]]; then
        rm -rf external/ViOS-libc
        echo "[✓] ViOS libc cleaned"
    else
        echo "[*] No ViOS libc installation found to clean"
    fi
}

show_help() {
    echo "Usage: $0 [OPTION]"
    echo ""
    echo "Options:"
    echo "  build     Build the project (default)"
    echo "  clean     Clean the project and ViOS libc"
    echo "  clean-libc Clean only ViOS libc installation"
    echo "  help      Show this help message"
    echo ""
    echo "Examples:"
    echo "  $0          # Build the project"
    echo "  $0 clean    # Clean everything"
    echo "  $0 clean-libc # Clean only libc"
}

# === Main ===
case "${1:-build}" in
    "build")
        check_and_install
        check_and_install_vios_libc
        run_make
        ;;
    "clean")
        echo "[*] Cleaning project..."
        make clean
        clean_vios_libc
        ;;
    "clean-libc")
        clean_vios_libc
        ;;
    "help"|"-h"|"--help")
        show_help
        ;;
    *)
        echo "[!] Unknown option: $1"
        show_help
        exit 1
        ;;
esac
