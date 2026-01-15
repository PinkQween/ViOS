# ViOS Build Guide

Professional cross-platform build system using CMake.

## Prerequisites

### Linux (Ubuntu/Debian)
```bash
sudo apt install cmake build-essential nasm python3
sudo apt install gcc-x86-64-linux-gnu binutils-x86-64-linux-gnu
```

### macOS (Homebrew)
```bash
brew install cmake nasm python3
brew install x86_64-elf-gcc x86_64-elf-binutils
```

## Building

### Quick Start
```bash
make
```

### Standard CMake
```bash
mkdir build && cd build
cmake ..
make
```

Output: `build/bin/os.img`

### Build Options

```bash
# Custom cross-compiler
make CROSS_COMPILE=x86_64-linux-gnu-

# Or with CMake directly
cmake .. -DCROSS_COMPILE=x86_64-linux-gnu-

# Clean build
make clean
```

## Running

```bash
./run.sh
```

## Project Structure

```
ViOS64BitDev/
├── Makefile               # Convenience wrapper
├── CMakeLists.txt         # CMake configuration
├── scripts/               # Build helpers
├── ViOS64Bit/            # Kernel source
├── ViOS.c                # UEFI bootloader
└── build/                # Build output
    └── bin/os.img
```

## Troubleshooting

**Cross-compiler not found**  
Install the toolchain or specify alternative:
```bash
make CROSS_COMPILE=x86_64-linux-gnu-
```

**EDK2 build fails**  
Initialize EDK2:
```bash
cd /path/to/edk2
make -C BaseTools
source edksetup.sh
```

**Permission errors**  
The build requires `sudo` for disk operations.

## Development Workflow

```bash
# Build
make

# Rebuild after changes
make

# Clean rebuild
make clean
make
```
