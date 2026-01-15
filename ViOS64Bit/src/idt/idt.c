#include "idt.h"
#include "config.h"
#include "kernel.h"
#include "memory/memory.h"
#include "task/task.h"
#include "task/process.h"
#include "memory/heap/kheap.h"
#include "io/io.h"
#include "status.h"
#include "io/serial.h"

// Official serial support for debugging and system messages
static void idt_serial_init(void) {
    serial_init(SERIAL_COM1_BASE);
    serial_write_string(SERIAL_COM1_BASE, "[IDT] Serial initialized.\n");
}

static void idt_serial_log(const char* msg) {
    serial_write_string(SERIAL_COM1_BASE, msg);
}

// To use:
// 1. Call idt_serial_init() early in your kernel or IDT setup.
// 2. Use idt_serial_log("message\n"); for debug output.
struct idt_desc idt_descriptors[VIOS_TOTAL_INTERRUPTS];
struct idtr_desc idtr_descriptor;

extern void* interrupt_pointer_table[VIOS_TOTAL_INTERRUPTS];

static INTERRUPT_CALLBACK_FUNCTION interrupt_callbacks[VIOS_TOTAL_INTERRUPTS];

static ISR80H_COMMAND isr80h_commands[VIOS_MAX_ISR80H_COMMANDS];

extern void idt_load(struct idtr_desc* ptr);
extern void int21h();
extern void no_interrupt();
extern void isr80h_wrapper();

void no_interrupt_handler()
{
    outb(0x20, 0x20);
    outb(0xA0, 0x20);
}

void interrupt_handler(int interrupt, struct interrupt_frame* frame)
{
    kernel_page();
    if (interrupt_callbacks[interrupt] != 0)
    {
        if (task_current())
        {
            task_current_save_state(frame);
        }
        interrupt_callbacks[interrupt](frame);
    }

    if (task_current())
    {
        task_page();
    }

    outb(0x20, 0x20); 
    outb(0xA0, 0x20);
}

void idt_zero()
{
    print("Divide by zero error\n");
    while(1) {}
}

// int 20h 
void idt_set(int interrupt_no, void* address)
{
   struct idt_desc* desc = &idt_descriptors[interrupt_no];
   uintptr_t _address = (uintptr_t) address;
   desc->offset_1 = _address & 0x000000000000ffff;
   desc->selector = KERNEL_LONG_MODE_CODE_SELECTOR;
   desc->ist = 0;

   desc->type_attr = 0xEE;
   if (interrupt_no <= 0x31)
   {
      desc->type_attr = 0x8E;
   }

   desc->offset_2 = (_address >> 16) & 0x000000000000ffff;
   desc->offset_3 = (_address >> 32) & 0x00000000ffffffff;
}

void idt_exception_0(struct interrupt_frame* frame)  { panic("Exception 0x00: Divide by Zero\n"); }
void idt_exception_1(struct interrupt_frame* frame)  { panic("Exception 0x01: Debug\n"); }
void idt_exception_2(struct interrupt_frame* frame)  { panic("Exception 0x02: NMI\n"); }
void idt_exception_3(struct interrupt_frame* frame)  { panic("Exception 0x03: Breakpoint\n"); }
void idt_exception_4(struct interrupt_frame* frame)  { panic("Exception 0x04: Overflow\n"); }
void idt_exception_5(struct interrupt_frame* frame)  { panic("Exception 0x05: Bound Range\n"); }
void idt_exception_6(struct interrupt_frame* frame)  { panic("Exception 0x06: Invalid Opcode\n"); }
void idt_exception_7(struct interrupt_frame* frame)  { panic("Exception 0x07: Device Not Available\n"); }
void idt_exception_8(struct interrupt_frame* frame)  { panic("Exception 0x08: Double Fault\n"); }
void idt_exception_9(struct interrupt_frame* frame)  { panic("Exception 0x09: Coprocessor Segment Overrun\n"); }
void idt_exception_A(struct interrupt_frame* frame)  { panic("Exception 0x0A: Invalid TSS\n"); }
void idt_exception_B(struct interrupt_frame* frame)  { panic("Exception 0x0B: Segment Not Present\n"); }
void idt_exception_C(struct interrupt_frame* frame)  { panic("Exception 0x0C: Stack Fault\n"); }
void idt_exception_D(struct interrupt_frame* frame)  { panic("Exception 0x0D: General Protection Fault\n"); }
void idt_exception_E(struct interrupt_frame* frame)  { panic("Exception 0x0E: Page Fault\n"); }
void idt_exception_F(struct interrupt_frame* frame)  { panic("Exception 0x0F: Reserved\n"); }
void idt_exception_10(struct interrupt_frame* frame) { panic("Exception 0x10: x87 FPU Error\n"); }
void idt_exception_11(struct interrupt_frame* frame) { panic("Exception 0x11: Alignment Check\n"); }
void idt_exception_12(struct interrupt_frame* frame) { panic("Exception 0x12: Machine Check\n"); }
void idt_exception_13(struct interrupt_frame* frame) { panic("Exception 0x13: SIMD FP Exception\n"); }

void idt_handle_exception(struct interrupt_frame* frame)
{
    panic("Generic Exception\n");
}

void idt_clock()
{
    outb(0x20, 0x20);

    if (!task_current())
    {
        return;
    }
    
    // Switch to the next task
    task_next();
}

void idt_init()
{
    memset(idt_descriptors, 0, sizeof(idt_descriptors));
    idtr_descriptor.limit = sizeof(idt_descriptors) -1;
    idtr_descriptor.base = (uint64_t) idt_descriptors;

    for (int i = 0; i < VIOS_TOTAL_INTERRUPTS; i++)
    {
        idt_set(i, interrupt_pointer_table[i]);
    }

    idt_set(0, idt_zero);
    idt_set(0x80, isr80h_wrapper);

    // Register specific exception handlers
    idt_register_interrupt_callback(0x00, idt_exception_0);
    idt_register_interrupt_callback(0x01, idt_exception_1);
    idt_register_interrupt_callback(0x02, idt_exception_2);
    idt_register_interrupt_callback(0x03, idt_exception_3);
    idt_register_interrupt_callback(0x04, idt_exception_4);
    idt_register_interrupt_callback(0x05, idt_exception_5);
    idt_register_interrupt_callback(0x06, idt_exception_6);
    idt_register_interrupt_callback(0x07, idt_exception_7);
    idt_register_interrupt_callback(0x08, idt_exception_8);
    idt_register_interrupt_callback(0x09, idt_exception_9);
    idt_register_interrupt_callback(0x0A, idt_exception_A);
    idt_register_interrupt_callback(0x0B, idt_exception_B);
    idt_register_interrupt_callback(0x0C, idt_exception_C);
    idt_register_interrupt_callback(0x0D, idt_exception_D);
    idt_register_interrupt_callback(0x0E, idt_exception_E);
    idt_register_interrupt_callback(0x0F, idt_exception_F);
    idt_register_interrupt_callback(0x10, idt_exception_10);
    idt_register_interrupt_callback(0x11, idt_exception_11);
    idt_register_interrupt_callback(0x12, idt_exception_12);
    idt_register_interrupt_callback(0x13, idt_exception_13);
    
    // Generic handler for remaining exceptions
    for (int i = 0x14; i < 0x20; i++)
    {
        idt_register_interrupt_callback(i, idt_handle_exception);
    }
    

    // DISABLED: Timer interrupt causes crashes due to broken task_next()
    // idt_register_interrupt_callback(0x20, idt_clock);

    // Load the interrupt descriptor table
    idt_load(&idtr_descriptor);
}

int idt_register_interrupt_callback(int interrupt, INTERRUPT_CALLBACK_FUNCTION interrupt_callback)
{
    if (interrupt < 0 || interrupt >= VIOS_TOTAL_INTERRUPTS)
    {
        return -EINVARG;
    }

    interrupt_callbacks[interrupt] = interrupt_callback;
    return 0;
}

void isr80h_register_command(int command_id, ISR80H_COMMAND command)
{
    if (command_id < 0 || command_id >= VIOS_MAX_ISR80H_COMMANDS)
    {
        panic("The command is out of bounds\n");
    }

    if (isr80h_commands[command_id])
    {
        panic("Your attempting to overwrite an existing command\n");
    }

    isr80h_commands[command_id] = command;
}

void* isr80h_handle_command(int command, struct interrupt_frame* frame)
{
    void* result = 0;

    if(command < 0 || command >= VIOS_MAX_ISR80H_COMMANDS)
    {
        // Invalid command
        return 0;
    }

    ISR80H_COMMAND command_func = isr80h_commands[command];
    if (!command_func)
    {
        return 0;
    }

    result = command_func(frame);
    return result;
}

void* isr80h_handler(int command, struct interrupt_frame* frame)
{
    void* res = 0;
    kernel_page();
    task_current_save_state(frame);
    res = isr80h_handle_command(command, frame);
    task_page();
    return res;
}