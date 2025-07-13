# ViOS Standard Library (libc)

This directory contains the locally installed ViOS standard library (`libViOSlibc.a`) and its header files.

## Priority System

The build system checks for ViOS libc in the following order:
1. **System-wide installation** (`/opt/ViOS`) - if manually installed
2. **Local installation** (`src/libc`) - if not found system-wide

## Structure

```
src/libc/
├── lib/
│   └── libViOSlibc.a    # The compiled standard library
├── include/             # Header files
│   └── *.h             # Various header files
└── README.md           # This file
```

## Installation

The ViOS libc is automatically installed when you run the build script:

```bash
./build.sh
```

The build script will:
1. First check for existing system-wide installation at `/opt/ViOS`
2. If not found, clone the ViOS-Libc repository from GitHub
3. Build the library
4. Install it locally in this directory

## Usage

Programs automatically detect the libc location:
- **System-wide**: `/opt/ViOS/lib/libViOSlibc.a` and `/opt/ViOS/include`
- **Local**: `../../src/libc/lib/libViOSlibc.a` and `../../src/libc/include`

The Makefiles automatically choose the correct path based on availability.

## Cleaning

To clean the libc installation:

```bash
./build.sh clean-libc
```

Or to clean everything including the main project:

```bash
./build.sh clean
```

## Manual Installation

If you need to install the libc manually:

1. Clone the repository:
   ```bash
   git clone https://github.com/PinkQween/ViOS-Libc.git
   cd ViOS-Libc
   ```

2. Build the library:
   ```bash
   make all
   ```

3. Copy to the local directory:
   ```bash
   mkdir -p ../../src/libc/lib
   mkdir -p ../../src/libc/include
   cp libViOSlibc.a ../../src/libc/lib/
   # Copy headers as needed
   ```

Or install system-wide (requires sudo):
   ```bash
   sudo make install
   ```

## Notes

- This is a local installation, not system-wide
- The library is specific to the ViOS operating system
- Programs built for ViOS should link against this library 