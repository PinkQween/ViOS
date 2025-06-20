#ifndef UART_KEYBOARD_H
#define UART_KEYBOARD_H

#include "keyboard.h"

// Initialize the UART keyboard driver, sets up UART interrupts and handling
int uart_keyboard_init(void);

// Return a pointer to the UART keyboard driver struct
struct keyboard *uart_init(void);

#endif // UART_KEYBOARD_H
