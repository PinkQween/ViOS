#!/bin/bash
set -euo pipefail

# === Config ===
# Use system toolchain instead of custom one
# export PREFIX="$HOME/opt/cross"
# export TARGET=i686-elf
# export PATH="$PREFIX/bin:$PATH"
# Set platform-appropriate virtual environment directory
export VENV_DIR="$HOME/ViOS-venv"

echo "=================================="
echo "  Using ViOS i386-ViOS-elf toolchain"
echo "=================================="

install_deps() {
    echo "[*] Installing required system packages..."
    sudo apt update
    sudo apt install -y unzip build-essential bison flex libgmp-dev libmpc-dev libmpfr-dev texinfo \
        wget curl git gawk xz-utils nasm python3 python3-venv libfreetype6-dev mtools
}

# Remove the custom GCC installation function since we're using system toolchain
# install_gcc() {
#     # ... removed ...
# }

check_and_install_vios_binutils() {
    echo "[*] Checking for ViOS binutils..."

    # Check if i386-vios-elf-ld is installed (binutils doesn't include GCC)
    if command -v "i386-vios-elf-ld" &>/dev/null; then
        echo "[✓] ViOS binutils found at: $(which i386-vios-elf-ld)"
        return 0
    fi

    echo "[!] ViOS binutils not found. Attempting to install..."

    if [[ "$(uname -s)" == "Darwin" ]]; then
        # macOS - ensure Homebrew is installed
        if ! command -v brew &>/dev/null; then
            echo "[!] Homebrew not found. Installing Homebrew..."
            /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)" || {
                echo "[!] Homebrew installation failed. Please install manually:"
                echo "[*] https://brew.sh/"
                exit 1
            }

            # Add Homebrew to PATH for current session
            if [[ -d "/opt/homebrew/bin" ]]; then
                export PATH="/opt/homebrew/bin:$PATH"
            elif [[ -d "/usr/local/bin" ]]; then
                export PATH="/usr/local/bin:$PATH"
            fi

            echo "[✓] Homebrew installed successfully."
        fi

        echo "[*] Installing ViOS binutils via Homebrew..."
        brew install --HEAD "../ViOS binutils/Formula/vios-binutils.rb" || {
            echo "[!] Failed to install via Homebrew. Trying alternative path..."
            brew install ../vios-binutils || {
                echo "[!] Failed to install via Homebrew. Please install manually from:"
                echo "[*] https://github.com/PinkQween/ViOS-binutils/releases"
                exit 1
            }
        }

    else
        # Linux - try to download and install from releases
        echo "[*] Downloading ViOS binutils from GitHub releases..."
        
        # Create temporary directory
        TEMP_DIR=$(mktemp -d)
        cd "$TEMP_DIR"
        
        # Try to download latest release - first try .tar.gz, then .deb as fallback
        echo "[*] Attempting to download tar.gz archive..."
        if curl -L -f -o vios-binutils.tar.gz "https://github.com/PinkQween/ViOS-binutils/releases/latest/download/vios-binutils-binaries-0.0.1.tar.gz"; then
            INSTALL_TYPE="tar"
        else
            echo "[*] tar.gz download failed, trying .deb package..."
            if curl -L -f -o vios-binutils.deb "https://github.com/PinkQween/ViOS-binutils/releases/latest/download/vios-binutils_0.0.1-1_amd64.deb"; then
                INSTALL_TYPE="deb"
            else
                echo "[!] Both tar.gz and deb downloads failed. Trying to get latest release info..."
                # Try to get the actual latest release tag and construct URLs
                LATEST_TAG=$(curl -s https://api.github.com/repos/PinkQween/ViOS-binutils/releases/latest | grep '"tag_name"' | cut -d'"' -f4)
                if [[ -n "$LATEST_TAG" ]]; then
                    echo "[*] Found latest tag: $LATEST_TAG, trying versioned URLs..."
                    # Remove 'v' prefix if present to match asset naming
                    VERSION_NUM=${LATEST_TAG#v}
                    echo "[*] Using version number: $VERSION_NUM"
                    if curl -L -f -o vios-binutils.tar.gz "https://github.com/PinkQween/ViOS-binutils/releases/download/$LATEST_TAG/vios-binutils-binaries-$VERSION_NUM.tar.gz"; then
                        INSTALL_TYPE="tar"
                    elif curl -L -f -o vios-binutils.deb "https://github.com/PinkQween/ViOS-binutils/releases/download/$LATEST_TAG/vios-binutils_$VERSION_NUM-1_amd64.deb"; then
                        INSTALL_TYPE="deb"
                    else
                        echo "[!] Failed to download ViOS binutils. Please install manually from:"
                        echo "[*] https://github.com/PinkQween/ViOS-binutils/releases"
                        rm -rf "$TEMP_DIR"
                        exit 1
                    fi
                else
                    echo "[!] Failed to get latest release info. Please install manually from:"
                    echo "[*] https://github.com/PinkQween/ViOS-binutils/releases"
                    rm -rf "$TEMP_DIR"
                    exit 1
                fi
            fi
        fi
        
        # Extract and install based on file type
        if [[ "$INSTALL_TYPE" == "tar" ]]; then
            echo "[*] Extracting and installing ViOS binutils from tar.gz..."
            tar -xzf vios-binutils.tar.gz
            
            # Check for different possible archive structures
            echo "[*] Checking archive structure..."
            ls -la .
            
            # Install to /usr/local (requires sudo)
            if [[ -d "opt/vios-binutils" ]]; then
                echo "[*] Found opt/vios-binutils structure"
                sudo cp -r opt/vios-binutils/* /usr/local/
                
                # Create uppercase symbolic links for compatibility
                echo "[*] Creating uppercase symbolic links for compatibility..."
                for tool in ar as g++ gcc ld nm objcopy objdump readelf size strings strip; do
                    if [[ -f "/usr/local/bin/i386-vios-elf-$tool" ]]; then
                        sudo ln -sf "i386-vios-elf-$tool" "/usr/local/bin/i386-ViOS-elf-$tool"
                    fi
                done
                
                echo "[✓] ViOS binutils installed to /usr/local"
            elif [[ -d "bin" ]]; then
                echo "[*] Found bin/ structure"
                sudo cp -r bin/* /usr/local/bin/
                
                # Create uppercase symbolic links for compatibility
                echo "[*] Creating uppercase symbolic links for compatibility..."
                for tool in ar as g++ gcc ld nm objcopy objdump readelf size strings strip; do
                    if [[ -f "/usr/local/bin/i386-vios-elf-$tool" ]]; then
                        sudo ln -sf "i386-vios-elf-$tool" "/usr/local/bin/i386-ViOS-elf-$tool"
                    fi
                done
                
                echo "[✓] ViOS binutils installed to /usr/local/bin"
            elif [[ -d "usr/local" ]]; then
                echo "[*] Found usr/local structure"
                sudo cp -r usr/local/* /usr/local/
                
                # Create uppercase symbolic links for compatibility
                echo "[*] Creating uppercase symbolic links for compatibility..."
                for tool in ar as g++ gcc ld nm objcopy objdump readelf size strings strip; do
                    if [[ -f "/usr/local/bin/i386-vios-elf-$tool" ]]; then
                        sudo ln -sf "i386-vios-elf-$tool" "/usr/local/bin/i386-ViOS-elf-$tool"
                    fi
                done
                
                echo "[✓] ViOS binutils installed to /usr/local"
            elif [[ -f "i386-vios-elf-ld" ]]; then
                echo "[*] Found binaries in root directory"
                sudo cp i386-vios-elf-* /usr/local/bin/
                if [[ -d "ldscripts" ]]; then
                    sudo mkdir -p /usr/local/lib/ldscripts
                    sudo cp -r ldscripts/* /usr/local/lib/ldscripts/
                fi
                
                # Create uppercase symbolic links for compatibility
                echo "[*] Creating uppercase symbolic links for compatibility..."
                for tool in ar as g++ gcc ld nm objcopy objdump readelf size strings strip; do
                    if [[ -f "/usr/local/bin/i386-vios-elf-$tool" ]]; then
                        sudo ln -sf "i386-vios-elf-$tool" "/usr/local/bin/i386-ViOS-elf-$tool"
                    fi
                done
                
                echo "[✓] ViOS binutils installed to /usr/local/bin"
            else
                echo "[!] Unexpected archive structure. Contents:"
                find . -type f -name "*ViOS*" -o -name "*elf*" | head -10
                echo "[*] Please install manually from: https://github.com/PinkQween/ViOS-binutils/releases"
                rm -rf "$TEMP_DIR"
                exit 1
            fi
        elif [[ "$INSTALL_TYPE" == "deb" ]]; then
            echo "[*] Installing ViOS binutils from .deb package..."
            if command -v dpkg >/dev/null 2>&1; then
                sudo dpkg -i vios-binutils.deb
                echo "[✓] ViOS binutils installed via dpkg"
            else
                echo "[!] dpkg not found. Cannot install .deb package."
                echo "[*] Please install manually from: https://github.com/PinkQween/ViOS-binutils/releases"
                rm -rf "$TEMP_DIR"
                exit 1
            fi
        fi
        
        # Clean up
        cd /
        rm -rf "$TEMP_DIR"
        
        # Update PATH if needed
        if ! echo "$PATH" | grep -q "/usr/local/bin"; then
            echo "[*] Adding /usr/local/bin to PATH"
            export PATH="/usr/local/bin:$PATH"
        fi
    fi
    
    # Verify installation
    if command -v "i386-vios-elf-ld" &>/dev/null; then
        echo "[✓] ViOS binutils successfully installed!"
    else
        echo "[!] ViOS binutils installation failed. Please install manually."
        exit 1
    fi
}

check_and_install_vios_libc() {
    echo "[*] Checking for ViOS standard library..."
    
    # Store the original directory
    ORIGINAL_DIR="$(pwd)"
    
    # Set the ViOS library path to be in external/ViOS-libc
    VIOS_LIB_PATH="$ORIGINAL_DIR/external/ViOS-libc"
    
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
        cd "$ORIGINAL_DIR"
        rm -rf "$TEMP_DIR"
        
        echo "[✓] ViOS standard library installed successfully at $VIOS_LIB_PATH"
    fi
}

check_and_install() {
    # Check if target binutils are installed
    if ! command -v "i386-vios-elf-ld" &>/dev/null; then
        echo "[!] i386-vios-elf-ld not found. Please install ViOS binutils:"
        echo "[*] On macOS: brew install ViOS-binutils"
        echo "[*] On Linux: Follow your distribution's instructions"
        exit 1
    else
        echo "[✓] i386-vios-elf-ld found at: $(which i386-vios-elf-ld)"
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

    make clean || echo "[*] make clean failed, continuing anyway..."
    
    # Debug: Check ViOS libc status before building
    echo "[*] Debug: Checking ViOS libc status..."
    if [[ -f "external/ViOS-libc/lib/libViOSlibc.a" ]]; then
        echo "[✓] ViOS libc found at external/ViOS-libc/lib/libViOSlibc.a"
        ls -la external/ViOS-libc/include/
    else
        echo "[!] ViOS libc not found at external/ViOS-libc/lib/libViOSlibc.a"
    fi
    
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
        check_and_install_vios_binutils
        check_and_install_vios_libc
        check_and_install
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
