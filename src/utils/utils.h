#ifndef UTILS_H
#define UTILS_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "fonts/characters_AtariST8x16SystemFont.h"
#include "fonts/characters_Arial.h"
#include "fonts/characters_RobotoThin.h"
#include "fonts/characters_Cheri.h"
#include "fonts/characters_Brightly.h"

void int_to_ascii(int num, char *str);
// void print_status(const char *msg, bool ok);

/**
 * Check if two floating point numbers are approximately equal
 * @param a First number
 * @param b Second number
 * @param epsilon Tolerance for comparison
 * @return 1 if approximately equal, 0 otherwise
 */
int almost_equal(double a, double b, double epsilon);

void draw_scaled_rgb_fill(const char *path, int start_x, int start_y, int dest_width, int dest_height);

void print(char *str, int x, int y, int r, int g, int b, float scale_x, float scale_y);

// Font descriptor for flexible font rendering
struct font_descriptor
{
    int (*get_character)(int index, int y);
    int (*get_advance)(int index);
    int (*get_kerning)(int left, int right);
    int width;
    int height;
};

// Font descriptors for all available fonts (declarations)
extern const struct font_descriptor FONT_ATARIST8X16;
extern const struct font_descriptor FONT_ARIAL;
extern const struct font_descriptor FONT_ROBOTOTHIN;
extern const struct font_descriptor FONT_CHERI;
extern const struct font_descriptor FONT_BRIGHTLY;

// Print with font selection
void print_ex(const struct font_descriptor *font, char *str, int x, int y, int r, int g, int b, float scale_x, float scale_y);

#endif