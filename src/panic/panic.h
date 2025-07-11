#ifndef PANIC_H
#define PANIC_H

#include <stdint.h>

// Forward declaration - actual definition is in idt.h
struct interrupt_frame;

// Extended interrupt frame for exceptions
struct exception_frame {
    uint32_t gs, fs, es, ds;
    uint32_t edi, esi, ebp, esp;
    uint32_t ebx, edx, ecx, eax;
    uint32_t int_no, err_code;
    uint32_t eip, cs, eflags, useresp, ss;
};

// Basic panic function (legacy)
void panic(const char *msg);

// Enhanced panic functions
void panic_with_frame(const char *msg, struct interrupt_frame *frame);
void panic_with_exception(const char *msg, struct exception_frame *frame);
void panic_assert(const char *file, int line, const char *condition);
void panic_assert_msg(const char *file, int line, const char *condition, const char *msg);

// Register and stack dump functions
void dump_registers(struct interrupt_frame *frame);
void dump_exception_registers(struct exception_frame *frame);
void dump_stack(uint32_t *stack_ptr, int count);
void dump_stack_trace(uint32_t ebp, int max_frames);

// Memory dump around crash location
void dump_memory_at_eip(uint32_t eip);

// System state dump
void dump_system_state(void);

// Assert macros
#ifdef DEBUG
    #define ASSERT(condition) \
        do { if (!(condition)) panic_assert(__FILE__, __LINE__, #condition); } while(0)
    #define ASSERT_MSG(condition, msg) \
        do { if (!(condition)) panic_assert_msg(__FILE__, __LINE__, #condition, msg); } while(0)
#else
    #define ASSERT(condition) ((void)0)
    #define ASSERT_MSG(condition, msg) ((void)0)
#endif

#endif
