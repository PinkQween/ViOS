#include "klog.h"
#include "serial.h"
#include "string/string.h"
#include "rtc/rtc.h"
#include "memory/heap/kheap.h"
#include "memory/memory.h"
#include <stddef.h>

// Global logging configuration
static klog_config_t g_klog_config = {0};

// Forward declarations
static bool klog_should_log(klog_level_t level, const char *subsystem);
static void klog_output_to_serial(klog_level_t level, const char *subsystem, const char *message);
static void klog_add_to_buffer(klog_level_t level, const char *subsystem, const char *message);

// vsnprintf implementation (declared at end of file)
int vsnprintf(char *buf, size_t size, const char *format, va_list args);

void klog_init(klog_level_t min_level, klog_dest_t destinations)
{
    memset(&g_klog_config, 0, sizeof(g_klog_config));
    
    g_klog_config.global_min_level = min_level;
    g_klog_config.destinations = destinations;
    g_klog_config.timestamps_enabled = true;
    g_klog_config.initialized = true;
    
    // Initialize memory buffer
    g_klog_config.memory_buffer.head = 0;
    g_klog_config.memory_buffer.tail = 0;
    g_klog_config.memory_buffer.count = 0;
    g_klog_config.memory_buffer.wrapped = false;
    
    // Log initialization
    klog(KLOG_INFO, "KLOG", "Logging system initialized (level: %s, destinations: 0x%x)", 
         klog_level_to_string(min_level), destinations);
}

void klog_shutdown(void)
{
    if (!g_klog_config.initialized) return;
    
    klog(KLOG_INFO, "KLOG", "Logging system shutting down");
    g_klog_config.initialized = false;
}

void klog(klog_level_t level, const char *subsystem, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    klog_v(level, subsystem, format, args);
    va_end(args);
}

void klog_v(klog_level_t level, const char *subsystem, const char *format, va_list args)
{
    if (!g_klog_config.initialized) return;
    if (!klog_should_log(level, subsystem)) return;
    
    // Format the message
    char message[512];
    vsnprintf(message, sizeof(message), format, args);
    
    // Output to configured destinations
    if (g_klog_config.destinations & KLOG_DEST_SERIAL) {
        klog_output_to_serial(level, subsystem, message);
    }
    
    if (g_klog_config.destinations & KLOG_DEST_MEMORY) {
        klog_add_to_buffer(level, subsystem, message);
    }
    
    // TODO: Add graphics output when implemented
}

void klog_boot(const char *format, ...)
{
    // Early boot logging - works even before full initialization
    if (!g_klog_config.initialized) {
        // Try to initialize with basic settings
        klog_init(KLOG_INFO, KLOG_DEST_SERIAL);
    }
    
    va_list args;
    va_start(args, format);
    klog_v(KLOG_INFO, "BOOT", format, args);
    va_end(args);
}

// Configuration functions
void klog_set_global_level(klog_level_t min_level)
{
    g_klog_config.global_min_level = min_level;
}

void klog_set_destinations(klog_dest_t destinations)
{
    g_klog_config.destinations = destinations;
}

void klog_enable_timestamps(bool enable)
{
    g_klog_config.timestamps_enabled = enable;
}

// Helper functions
const char *klog_level_to_string(klog_level_t level)
{
    switch (level) {
        case KLOG_TRACE: return "TRACE";
        case KLOG_DEBUG: return "DEBUG";
        case KLOG_INFO:  return "INFO ";
        case KLOG_WARN:  return "WARN ";
        case KLOG_ERROR: return "ERROR";
        case KLOG_FATAL: return "FATAL";
        default: return "?????";
    }
}

klog_level_t klog_string_to_level(const char *level_str)
{
    if (!level_str) return KLOG_INFO;
    
    if (strncmp(level_str, "TRACE", 5) == 0) return KLOG_TRACE;
    if (strncmp(level_str, "DEBUG", 5) == 0) return KLOG_DEBUG;
    if (strncmp(level_str, "INFO", 4) == 0) return KLOG_INFO;
    if (strncmp(level_str, "WARN", 4) == 0) return KLOG_WARN;
    if (strncmp(level_str, "ERROR", 5) == 0) return KLOG_ERROR;
    if (strncmp(level_str, "FATAL", 5) == 0) return KLOG_FATAL;
    
    return KLOG_INFO;
}

void klog_hex_dump(klog_level_t level, const char *subsystem, 
                   const void *data, size_t size, uint32_t base_addr)
{
    if (!klog_should_log(level, subsystem)) return;
    
    klog(level, subsystem, "Hex dump of %u bytes at 0x%08x:", size, base_addr);
    
    // Use serial hex dump if available and serial output is enabled
    if (g_klog_config.destinations & KLOG_DEST_SERIAL) {
        serial_hex_dump(data, size, base_addr);
    }
}

// Memory buffer functions
void klog_clear_buffer(void)
{
    g_klog_config.memory_buffer.head = 0;
    g_klog_config.memory_buffer.tail = 0;
    g_klog_config.memory_buffer.count = 0;
    g_klog_config.memory_buffer.wrapped = false;
}

uint32_t klog_get_buffer_count(void)
{
    return g_klog_config.memory_buffer.count;
}

klog_entry_t *klog_get_buffer_entries(uint32_t *count)
{
    if (count) *count = g_klog_config.memory_buffer.count;
    return g_klog_config.memory_buffer.entries;
}

void klog_dump_buffer_to_serial(void)
{
    if (!(g_klog_config.destinations & KLOG_DEST_SERIAL)) return;
    
    serial_printf("=== Kernel Log Buffer Dump ===\n");
    serial_printf("Buffer contains %u entries\n", g_klog_config.memory_buffer.count);
    
    // Implementation would iterate through buffer and output each entry
    // For now, just indicate that this feature is available
    serial_printf("Buffer dump functionality available\n");
}

// Private helper functions
static bool klog_should_log(klog_level_t level, const char *subsystem)
{
    if (!g_klog_config.initialized) return false;
    
    // Check global level first
    if (level < g_klog_config.global_min_level) return false;
    
    // TODO: Check subsystem-specific filters
    
    return true;
}

static void klog_output_to_serial(klog_level_t level, const char *subsystem, const char *message)
{
    // Get timestamp if enabled
    if (g_klog_config.timestamps_enabled) {
        struct rtc_time current_time;
        rtc_read(&current_time);
        
        serial_printf("[%02d:%02d:%02d] ", 
                      current_time.hour, current_time.minute, current_time.second);
    }
    
    // Output level and subsystem
    serial_printf("[%s] ", klog_level_to_string(level));
    
    if (subsystem) {
        serial_printf("[%s] ", subsystem);
    }
    
    // Output message
    serial_printf("%s\n", message);
}

static void klog_add_to_buffer(klog_level_t level, const char *subsystem, const char *message)
{
    // TODO: Implement circular buffer storage
    // For now, just track that we would store it
    (void)level;
    (void)subsystem;
    (void)message;
}

// Simple vsnprintf implementation for kernel use
int vsnprintf(char *buf, size_t size, const char *format, va_list args)
{
    if (!buf || size == 0) return 0;
    
    char *p = buf;
    const char *f = format;
    size_t remaining = size - 1; // Leave space for null terminator
    
    while (*f && remaining > 0) {
        if (*f == '%' && *(f + 1)) {
            f++;
            switch (*f) {
                case 'd':
                case 'i': {
                    int num = va_arg(args, int);
                    // Simple number to string conversion
                    if (num < 0) {
                        *p++ = '-';
                        remaining--;
                        num = -num;
                    }
                    
                    char temp[16];
                    int i = 0;
                    if (num == 0) {
                        temp[i++] = '0';
                    } else {
                        while (num > 0 && i < 15) {
                            temp[i++] = '0' + (num % 10);
                            num /= 10;
                        }
                    }
                    
                    while (i > 0 && remaining > 0) {
                        *p++ = temp[--i];
                        remaining--;
                    }
                    break;
                }
                case 's': {
                    const char *str = va_arg(args, const char*);
                    if (!str) str = "(null)";
                    
                    while (*str && remaining > 0) {
                        *p++ = *str++;
                        remaining--;
                    }
                    break;
                }
                case 'c': {
                    char c = (char)va_arg(args, int);
                    *p++ = c;
                    remaining--;
                    break;
                }
                case '%': {
                    *p++ = '%';
                    remaining--;
                    break;
                }
                default: {
                    *p++ = '%';
                    if (remaining > 1) {
                        *p++ = *f;
                        remaining--;
                    }
                    remaining--;
                    break;
                }
            }
        } else {
            *p++ = *f;
            remaining--;
        }
        f++;
    }
    
    *p = '\0';
    return p - buf;
}
