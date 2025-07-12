#ifndef VGA_TEXT_H
#define VGA_TEXT_H

#include <stdint.h>

// VGA text mode constants
#define VGA_TEXT_BUFFER 0xB8000
#define VGA_TEXT_WIDTH  80
#define VGA_TEXT_HEIGHT 25

// VGA colors
#define VGA_COLOR_BLACK         0
#define VGA_COLOR_BLUE          1
#define VGA_COLOR_GREEN         2
#define VGA_COLOR_CYAN          3
#define VGA_COLOR_RED           4
#define VGA_COLOR_MAGENTA       5
#define VGA_COLOR_BROWN         6
#define VGA_COLOR_LIGHT_GREY    7
#define VGA_COLOR_DARK_GREY     8
#define VGA_COLOR_LIGHT_BLUE    9
#define VGA_COLOR_LIGHT_GREEN   10
#define VGA_COLOR_LIGHT_CYAN    11
#define VGA_COLOR_LIGHT_RED     12
#define VGA_COLOR_LIGHT_MAGENTA 13
#define VGA_COLOR_LIGHT_BROWN   14
#define VGA_COLOR_WHITE         15

// Function prototypes
void vga_text_init(void);
void vga_text_clear(void);
void vga_text_put_char(char c, uint8_t color, int x, int y);
void vga_text_write_string(const char* str, uint8_t color, int x, int y);
void vga_text_write_line(const char* str, uint8_t color, int line);
uint8_t vga_text_make_color(uint8_t fg, uint8_t bg);

#endif // VGA_TEXT_H
