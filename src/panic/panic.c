
#include "panic.h"
#include "debug/serial.h"
#include "rtc/rtc.h"
#include "graphics/graphics.h"
#include "memory/memory.h"
#include "idt/idt.h"
#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>

// Helper function to use serial debug safely
static void serial_fatal_log(const char *format, ...) {
    va_list args;
    va_start(args, format);
    serial_debug(SERIAL_DEBUG_FATAL, "PANIC", format, args);
    va_end(args);
}

// Basic legacy panic
void panic(const char *msg) {
    serial_fatal_log("System Panic: %s", msg);
    // TODO: Add graphics fatal display when implemented
    while (1);
}

// Enhanced panic with interrupt frame
void panic_with_frame(const char *msg, struct interrupt_frame *frame) {
    // Display on serial port
    serial_fatal_log("Panic: %s", msg);
    dump_registers(frame);
    dump_system_state();

    // TODO: Add graphics fatal display when implemented
    
    // Halt
    while (1);
}

// Enhanced panic with exception
void panic_with_exception(const char *msg, struct exception_frame *frame) {
    // Display on serial port
    serial_fatal_log("Panic (Exception): %s", msg);
    dump_exception_registers(frame);
    dump_memory_at_eip(frame->eip);
    dump_system_state();

    // TODO: Add graphics fatal display when implemented
    
    // Halt
    while (1);
}

// Register dump functions
void dump_registers(struct interrupt_frame *frame) {
    if (!frame) return;
    serial_fatal_log("EIP: %x  CS: %x  EFLAGS: %x  ESP: %x  SS: %x",
                    frame->ip, frame->cs, frame->flags,
                    frame->esp, frame->ss);
    serial_fatal_log("EAX: %x  EBX: %x  ECX: %x  EDX: %x",
                    frame->eax, frame->ebx, frame->ecx, frame->edx);
    serial_fatal_log("ESI: %x  EDI: %x  EBP: %x",
                    frame->esi, frame->edi, frame->ebp);
}

void dump_exception_registers(struct exception_frame *frame) {
    if (!frame) return;
    serial_fatal_log("Registers: EAX=%x, EBX=%x, ECX=%x, EDX=%x, ESP=%x",
                    frame->eax, frame->ebx, frame->ecx,
                    frame->edx, frame->esp);
    serial_fatal_log("EBP: %x  ESI: %x  EDI: %x  EFLAGS: %x",
                    frame->ebp, frame->esi, frame->edi,
                    frame->eflags);
}

// Memory dump
void dump_memory_at_eip(uint32_t eip) {
    serial_hex_dump((void*)eip, 64, eip);
}

// System state
void dump_system_state(void) {
    serial_fatal_log("System state: Pending..."); // Placeholder for further functionality
}

// Assert functions
void panic_assert(const char *file, int line, const char *condition) {
    serial_fatal_log("Assert Failure: %s in %s at line %d", condition, file, line);
    panic_with_frame("Assert Failure", 0);
}

void panic_assert_msg(const char *file, int line, const char *condition, const char *msg) {
    serial_fatal_log("Assert Failure: %s (%s) in %s at line %d", msg, condition, file, line);
    panic_with_frame("Assert Failure", 0);
}
