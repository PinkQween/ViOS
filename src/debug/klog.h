#ifndef KLOG_H
#define KLOG_H

#include <stdint.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>

// Log levels
typedef enum {
    KLOG_TRACE = 0,
    KLOG_DEBUG = 1,
    KLOG_INFO  = 2,
    KLOG_WARN  = 3,
    KLOG_ERROR = 4,
    KLOG_FATAL = 5
} klog_level_t;

// Log destinations
typedef enum {
    KLOG_DEST_SERIAL   = 0x01,
    KLOG_DEST_GRAPHICS = 0x02,
    KLOG_DEST_MEMORY   = 0x04,
    KLOG_DEST_ALL      = 0x07
} klog_dest_t;

// Maximum subsystem name length
#define KLOG_MAX_SUBSYSTEM_LEN 16

// Log buffer configuration
#define KLOG_BUFFER_SIZE 4096
#define KLOG_MAX_SUBSYSTEMS 32

// Subsystem filtering
typedef struct {
    char name[KLOG_MAX_SUBSYSTEM_LEN];
    klog_level_t min_level;
    bool enabled;
} klog_subsystem_filter_t;

// Log entry structure for memory logging
typedef struct {
    uint32_t timestamp;
    klog_level_t level;
    char subsystem[KLOG_MAX_SUBSYSTEM_LEN];
    char message[256];
} klog_entry_t;

// Circular buffer for in-memory logging
typedef struct {
    klog_entry_t entries[KLOG_BUFFER_SIZE];
    uint32_t head;
    uint32_t tail;
    uint32_t count;
    bool wrapped;
} klog_buffer_t;

// Global logging configuration
typedef struct {
    klog_level_t global_min_level;
    klog_dest_t destinations;
    klog_subsystem_filter_t subsystems[KLOG_MAX_SUBSYSTEMS];
    uint32_t num_subsystems;
    klog_buffer_t memory_buffer;
    bool initialized;
    bool timestamps_enabled;
} klog_config_t;

// Core logging functions
void klog_init(klog_level_t min_level, klog_dest_t destinations);
void klog_shutdown(void);

// Main logging function
void klog(klog_level_t level, const char *subsystem, const char *format, ...);
void klog_v(klog_level_t level, const char *subsystem, const char *format, va_list args);

// Configuration functions
void klog_set_global_level(klog_level_t min_level);
void klog_set_destinations(klog_dest_t destinations);
void klog_enable_timestamps(bool enable);

// Subsystem filtering
int klog_add_subsystem_filter(const char *subsystem, klog_level_t min_level);
int klog_remove_subsystem_filter(const char *subsystem);
void klog_enable_subsystem(const char *subsystem, bool enable);
void klog_set_subsystem_level(const char *subsystem, klog_level_t min_level);

// Memory buffer functions
void klog_clear_buffer(void);
uint32_t klog_get_buffer_count(void);
klog_entry_t *klog_get_buffer_entries(uint32_t *count);
void klog_dump_buffer_to_serial(void);

// Boot logging (available early, before full init)
void klog_boot(const char *format, ...);

// Helper functions
const char *klog_level_to_string(klog_level_t level);
klog_level_t klog_string_to_level(const char *level_str);

// Convenience macros
#define KLOG_TRACE(subsystem, ...) klog(KLOG_TRACE, subsystem, __VA_ARGS__)
#define KLOG_DEBUG(subsystem, ...) klog(KLOG_DEBUG, subsystem, __VA_ARGS__)
#define KLOG_INFO(subsystem, ...)  klog(KLOG_INFO,  subsystem, __VA_ARGS__)
#define KLOG_WARN(subsystem, ...)  klog(KLOG_WARN,  subsystem, __VA_ARGS__)
#define KLOG_ERROR(subsystem, ...) klog(KLOG_ERROR, subsystem, __VA_ARGS__)
#define KLOG_FATAL(subsystem, ...) klog(KLOG_FATAL, subsystem, __VA_ARGS__)

// Conditional logging macros
#ifdef DEBUG
    #define KLOG_TRACE_IF(cond, subsystem, ...) \
        do { if (cond) klog(KLOG_TRACE, subsystem, __VA_ARGS__); } while(0)
    #define KLOG_DEBUG_IF(cond, subsystem, ...) \
        do { if (cond) klog(KLOG_DEBUG, subsystem, __VA_ARGS__); } while(0)
#else
    #define KLOG_TRACE_IF(cond, subsystem, ...) ((void)0)
    #define KLOG_DEBUG_IF(cond, subsystem, ...) ((void)0)
#endif

// Function entry/exit tracing
#ifdef DEBUG
    #define KLOG_FUNC_ENTRY(subsystem) \
        klog(KLOG_TRACE, subsystem, "Entering %s", __func__)
    #define KLOG_FUNC_EXIT(subsystem) \
        klog(KLOG_TRACE, subsystem, "Exiting %s", __func__)
    #define KLOG_FUNC_EXIT_VAL(subsystem, val) \
        klog(KLOG_TRACE, subsystem, "Exiting %s (return: %d)", __func__, val)
#else
    #define KLOG_FUNC_ENTRY(subsystem) ((void)0)
    #define KLOG_FUNC_EXIT(subsystem) ((void)0)
    #define KLOG_FUNC_EXIT_VAL(subsystem, val) ((void)0)
#endif

// Hex dump logging
void klog_hex_dump(klog_level_t level, const char *subsystem, 
                   const void *data, size_t size, uint32_t base_addr);

#endif // KLOG_H
