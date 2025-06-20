#include "idt/idt.h"
#include "keyboard.h"
#include "io/io.h"
#include "task/task.h"
#include "kernel.h"
#include <stdint.h>
#include "task/process.h"
#include "terminal/terminal.h"

#define UART_BASE 0x3F8
#define UART_IRQ_VECTOR (0x20 + 4)

int uart_keyboard_init(void);

void uart_keyboard_handle_interrupt(struct interrupt_frame *frame)
{
    kernel_page();

    uint8_t c = inb(UART_BASE);

    // For debug: print code as hex
    // print("Received UART code: 0x");
    // print_hex(c); // You need a print_hex function that prints hex digits
    // print("\n");

    if (c == 0x08 || c == 0x7F) // Handle backspace or delete
    {
        // Call your keyboard_backspace function to remove last char from buffer
        keyboard_backspace(process_current());
    }
    else
    {
        keyboard_push(c);
    }

    task_page();
}

int uart_keyboard_init(void)
{
    outb(UART_BASE + 1, 0x01);
    idt_register_interrupt_callback(UART_IRQ_VECTOR, uart_keyboard_handle_interrupt);
    return 0;
}

static struct keyboard uart_keyboard = {
    .name = {"UART"},
    .init = uart_keyboard_init,
};

struct keyboard *uart_init(void)
{
    return &uart_keyboard;
}
