#ifndef TERMINAL_H
#define TERMINAL_H

#include <stddef.h>
#include <stdint.h>

#define VGA_WIDTH 80
#define VGA_HEIGHT 25

#define VGA_COLOR_BLACK 0x0
#define VGA_COLOR_BLUE 0x1
#define VGA_COLOR_GREEN 0x2
#define VGA_COLOR_CYAN 0x3
#define VGA_COLOR_RED 0x4
#define VGA_COLOR_MAGENTA 0x5
#define VGA_COLOR_BROWN 0x6
#define VGA_COLOR_LIGHT_GREY 0x7
#define VGA_COLOR_DARK_GREY 0x8
#define VGA_COLOR_LIGHT_BLUE 0x9
#define VGA_COLOR_LIGHT_GREEN 0xA
#define VGA_COLOR_LIGHT_BLACK 0xB
#define VGA_COLOR_LIGHT_RED 0xC
#define VGA_COLOR_PINK 0xD
#define VGA_COLOR_YELLOW 0xE
#define VGA_COLOR_WHITE 0xF

uint8_t convert_color(int fg, int bg);
void print_status(const char *message, const char *status);
void terminal_writechar(char c, char colour);
void terminal_initialize();
void terminal_putchar(int x, int y, char c, char colour);
void terminal_writechar(char c, char colour);
void terminal_backspace(void);
void terminal_scroll(void);

void print(const char *str);
void print_colored(const char *str, char fg, char bg);
void print_status(const char *message, const char *status);

#endif