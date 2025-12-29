#!/bin/bash
set -euo pipefail

# === Config ===
echo "=================================="
echo "        ViOS Build Script         "
echo "=================================="

# Add Homebrew paths to PATH for cross-compilers
export PATH="/opt/homebrew/bin:/opt/homebrew/sbin:/usr/local/bin:$PATH"


check_toolchain() {
    echo "[*] Checking for required toolchain..."

    # Check for x86_64-elf-gcc (required for this project)
    if ! command -v x86_64-elf-gcc &>/dev/null; then
        echo "[!] x86_64-elf-gcc not found"
        echo "[*] Installing via Homebrew..."

        if ! command -v brew &>/dev/null; then
            echo "[!] Homebrew not found. Please install from https://brew.sh/"
            exit 1
        fi

        brew install x86_64-elf-gcc || {
            echo "[!] Failed to install x86_64-elf-gcc"
            exit 1
        }
    fi

    echo "[✓] x86_64-elf-gcc found at: $(which x86_64-elf-gcc)"

    # Check for x86_64-elf-ld (should come with binutils)
    if ! command -v x86_64-elf-ld &>/dev/null; then
        echo "[!] x86_64-elf-ld not found"
        echo "[*] Installing x86_64-elf-binutils via Homebrew..."
        brew install x86_64-elf-binutils || {
            echo "[!] Failed to install x86_64-elf-binutils"
            exit 1
        }
    fi

    echo "[✓] x86_64-elf-ld found at: $(which x86_64-elf-ld)"
}


check_required_tools() {
    echo "[*] Checking for required tools..."

    # Check required tools
    missing_tools=()
    for tool in nasm make; do
        if ! command -v "$tool" &>/dev/null; then
            missing_tools+=("$tool")
        fi
    done

    if [ ${#missing_tools[@]} -gt 0 ]; then
        echo "[!] Missing tools: ${missing_tools[*]}"
        echo "[*] Installing via Homebrew..."

        if ! command -v brew &>/dev/null; then
            echo "[!] Homebrew not found. Please install from https://brew.sh/"
            exit 1
        fi

        for tool in "${missing_tools[@]}"; do
            brew install "$tool" || {
                echo "[!] Failed to install $tool"
                exit 1
            }
        done
    fi

    echo "[✓] All required tools are installed."
}

run_make() {
    echo "[*] Running your project Makefile..."

    make clean || echo "[*] make clean failed, continuing anyway..."
    make all
}


show_help() {
    echo "Usage: $0 [OPTION]"
    echo ""
    echo "Options:"
    echo "  build     Build the project (default)"
    echo "  clean     Clean the project"
    echo "  help      Show this help message"
    echo ""
    echo "Examples:"
    echo "  $0          # Build the project"
    echo "  $0 clean    # Clean everything"
}

# === Main ===
case "${1:-build}" in
    "build")
        check_toolchain
        check_required_tools
        run_make
        ;;
    "clean")
        echo "[*] Cleaning project..."
        make clean
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
