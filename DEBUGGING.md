# ViOS Debugging System

This document describes the comprehensive debugging system implemented for ViOS, providing external logging capabilities for QEMU-based development.

## 🎯 Overview

The debugging system provides three main components:
1. **Serial Port Logging** - External output via COM1/COM2 
2. **Enhanced Panic System** - Better crash analysis with register dumps
3. **Structured Logging** - Professional logging with levels and filtering

## 📁 File Structure

```
src/debug/
├── serial.h      # Serial port interface and definitions
├── serial.c      # Serial port implementation
├── klog.h        # Kernel logging system interface
└── klog.c        # Kernel logging implementation

src/panic/
├── panic.h       # Enhanced panic system (updated)
└── panic.c       # Panic implementation with debug support
```

## 🚀 Quick Start

### QEMU Testing

```bash
# Run with live serial output in terminal
qemu-system-i386 -drive file=./bin/os.bin,format=raw,index=0,media=disk -m 128M -serial stdio -display none

# Run with serial output to file
qemu-system-i386 -drive file=./bin/os.bin,format=raw,index=0,media=disk -m 128M -serial file:debug.log
```

### Basic Usage in Kernel Code

```c
#include "debug/klog.h"

// Basic logging
KLOG_INFO("MEMORY", "Allocated %d bytes at %p", size, ptr);
KLOG_ERROR("DISK", "Failed to read sector %d", sector);

// Debugging with asserts
ASSERT(ptr != NULL);
ASSERT_MSG(size > 0, "Invalid allocation size");

// Hex dumps for debugging
klog_hex_dump(KLOG_DEBUG, "PACKET", buffer, 64, 0x1000);
```

## 📊 Features

### Serial Port System (`debug/serial.h/c`)

- **Multi-port support**: COM1 (0x3F8) and COM2 (0x2F8)
- **Multiple baud rates**: 9600, 19200, 38400, 57600, 115200
- **Formatted output**: `serial_printf()` with full printf support
- **Debug levels**: TRACE, INFO, WARN, ERROR, FATAL
- **Hex dumps**: Memory inspection with ASCII display
- **Early boot logging**: Available before full kernel initialization

#### API Functions

```c
// Initialization
int serial_init(uint16_t port, uint32_t baud_rate);

// Basic I/O
void serial_putc(char c);
void serial_puts(const char *str);
void serial_printf(const char *format, ...);

// Debug output
void serial_debug(serial_debug_level_t level, const char *subsystem, const char *format, ...);
void serial_hex_dump(const void *data, size_t size, uint32_t base_address);

// Convenience macros
SERIAL_INFO("MEMORY", "Heap initialized");
SERIAL_ERROR("DISK", "Read failed");
```

### Enhanced Panic System (`panic/panic.h/c`)

- **Register dumps**: Complete CPU state at crash
- **Memory dumps**: Hex dump around crash location
- **Assert macros**: Debug and release variants
- **Stack traces**: Function call analysis
- **Serial output**: All panic info sent to serial port

#### API Functions

```c
// Basic panic (legacy)
void panic(const char *msg);

// Enhanced panic with context
void panic_with_frame(const char *msg, struct interrupt_frame *frame);
void panic_with_exception(const char *msg, struct exception_frame *frame);

// Assert macros (DEBUG builds only)
ASSERT(condition);
ASSERT_MSG(condition, "Error message");
```

### Structured Logging System (`debug/klog.h/c`)

- **Multiple destinations**: Serial, graphics, memory buffer
- **Subsystem filtering**: Enable/disable by component
- **Log levels**: Fine-grained control
- **Timestamped logs**: RTC-based timestamps
- **Memory buffering**: Circular buffer for log storage

#### API Functions

```c
// Initialization
void klog_init(klog_level_t min_level, klog_dest_t destinations);

// Main logging
void klog(klog_level_t level, const char *subsystem, const char *format, ...);

// Configuration
void klog_set_global_level(klog_level_t min_level);
void klog_set_destinations(klog_dest_t destinations);

// Convenience macros
KLOG_TRACE("SUBSYSTEM", "Detailed trace info");
KLOG_INFO("SUBSYSTEM", "General information");
KLOG_WARN("SUBSYSTEM", "Warning condition");
KLOG_ERROR("SUBSYSTEM", "Error condition");
KLOG_FATAL("SUBSYSTEM", "Fatal error");
```

## 🔧 Configuration

### Log Levels

```c
typedef enum {
    KLOG_TRACE = 0,  // Detailed debugging
    KLOG_DEBUG = 1,  // Debug information
    KLOG_INFO  = 2,  // General information
    KLOG_WARN  = 3,  // Warning conditions
    KLOG_ERROR = 4,  // Error conditions
    KLOG_FATAL = 5   // Fatal errors
} klog_level_t;
```

### Destinations

```c
typedef enum {
    KLOG_DEST_SERIAL   = 0x01,  // Serial port output
    KLOG_DEST_GRAPHICS = 0x02,  // Graphics output (future)
    KLOG_DEST_MEMORY   = 0x04,  // Memory buffer
    KLOG_DEST_ALL      = 0x07   // All destinations
} klog_dest_t;
```

## 🏗️ Integration

### Kernel Initialization

The debugging system is initialized early in `kernel_main()`:

```c
void kernel_main() {
    // Initialize serial debugging first
    serial_init(SERIAL_COM1_BASE, 115200);
    klog_init(KLOG_INFO, KLOG_DEST_SERIAL);
    klog_boot("Kernel booting...");
    
    // Rest of kernel initialization...
}
```

### Makefile Integration

The build system includes the debug modules:

```makefile
FILES = \
  # ... other files ...
  ./build/debug/serial.o \
  ./build/debug/klog.o
```

## 📈 Example Output

```
=== ViOS Serial Debug Log ===
Serial port initialized: COM1 at 115200 baud
[12:34:56] [INFO ] [BOOT] Kernel booting...
[12:34:56] [INFO ] [KERNEL] Setting up GDT and TSS
[12:34:56] [INFO ] [KERNEL] Initializing memory subsystem
[12:34:56] [INFO ] [KERNEL] Initializing devices
[12:34:57] [INFO ] [KERNEL] Registering system call handlers
[12:34:57] [INFO ] [KERNEL] Initializing graphics subsystem
[12:34:57] [INFO ] [KERNEL] Loading first process: 0:/systemd.elf
[12:34:57] [INFO ] [KERNEL] Starting first task
[12:34:57] [INFO ] [KERNEL] Kernel initialization complete
```

## 🐛 Debugging Tips

### Common Issues

1. **No serial output**: Check QEMU serial configuration
2. **Garbled output**: Verify baud rate settings
3. **Missing logs**: Check log level filtering

### Best Practices

1. **Use appropriate levels**: INFO for general, DEBUG for detailed
2. **Include subsystem names**: Makes filtering easier
3. **Add context**: Include relevant data in log messages
4. **Use asserts**: Catch bugs early in development

### Performance Considerations

- Serial output is relatively slow
- Use conditional logging for high-frequency events
- Consider log level filtering for production builds

## 🔍 Advanced Features

### Conditional Logging

```c
#ifdef DEBUG
    KLOG_TRACE_IF(condition, "MEMORY", "Debug trace");
    KLOG_DEBUG_IF(verbose, "DISK", "Verbose debug");
#endif
```

### Function Tracing

```c
void my_function() {
    KLOG_FUNC_ENTRY("SUBSYSTEM");
    
    // Function implementation
    
    KLOG_FUNC_EXIT_VAL("SUBSYSTEM", return_value);
}
```

### Memory Analysis

```c
// Hex dump with context
klog_hex_dump(KLOG_DEBUG, "NETWORK", packet_buffer, packet_size, 0x1000);

// Register dumps on crash
panic_with_frame("Division by zero", current_frame);
```

## 🧪 Testing

The system has been tested with:

- ✅ QEMU i386 emulation
- ✅ Serial output to file
- ✅ Serial output to console
- ✅ Multiple baud rates
- ✅ All log levels
- ✅ Panic conditions
- ✅ Assert macros

## 📚 Related Files

- `src/kernel.c` - Main kernel with debug initialization
- `src/panic/` - Enhanced panic system
- `Makefile` - Build system integration
- `src/idt/idt.h` - Interrupt frame definitions

## 🎯 Future Enhancements

- [ ] Graphics output destination
- [ ] Network logging support
- [ ] Log file rotation
- [ ] Performance profiling integration
- [ ] Remote debugging protocol

---

*This debugging system provides professional-grade logging and debugging capabilities for ViOS development in QEMU environments.*
