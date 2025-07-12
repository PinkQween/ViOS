#include "vga_text.h"
#include "string/string.h"

static uint16_t* vga_buffer = (uint16_t*)VGA_TEXT_BUFFER;

uint8_t vga_text_make_color(uint8_t fg, uint8_t bg) {
    return fg | bg << 4;
}

static uint16_t vga_text_make_entry(char c, uint8_t color) {
    return (uint16_t)c | (uint16_t)color << 8;
}

void vga_text_init(void) {
    vga_text_clear();
}

void vga_text_clear(void) {
    uint8_t color = vga_text_make_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    uint16_t blank = vga_text_make_entry(' ', color);
    
    for (int i = 0; i < VGA_TEXT_WIDTH * VGA_TEXT_HEIGHT; i++) {
        vga_buffer[i] = blank;
    }
}

void vga_text_put_char(char c, uint8_t color, int x, int y) {
    if (x >= 0 && x < VGA_TEXT_WIDTH && y >= 0 && y < VGA_TEXT_HEIGHT) {
        int index = y * VGA_TEXT_WIDTH + x;
        vga_buffer[index] = vga_text_make_entry(c, color);
    }
}

void vga_text_write_string(const char* str, uint8_t color, int x, int y) {
    if (!str) return;
    
    int curr_x = x;
    int curr_y = y;
    
    while (*str) {
        if (*str == '\n') {
            curr_y++;
            curr_x = x;
        } else {
            vga_text_put_char(*str, color, curr_x, curr_y);
            curr_x++;
            if (curr_x >= VGA_TEXT_WIDTH) {
                curr_x = x;
                curr_y++;
            }
        }
        
        if (curr_y >= VGA_TEXT_HEIGHT) {
            break;
        }
        
        str++;
    }
}

void vga_text_write_line(const char* str, uint8_t color, int line) {
    if (line >= 0 && line < VGA_TEXT_HEIGHT) {
        // Clear the line first
        uint16_t blank = vga_text_make_entry(' ', color);
        for (int x = 0; x < VGA_TEXT_WIDTH; x++) {
            vga_buffer[line * VGA_TEXT_WIDTH + x] = blank;
        }
        
        // Write the string
        vga_text_write_string(str, color, 0, line);
    }
}
