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

typedef void (*DRAW_CALLBACK)(void *context);

struct ScaledRGBDrawContext
{
    int start_x, start_y;
    int dest_width, dest_height;
    int src_width, src_height;
    float scale_x, scale_y;
    uint8_t *rgb_data;
};

void draw_scaled_rgb_fill(void *ctx_ptr);

int getFuncToDrawScaledRGBFill(
    const char *path, int start_x, int start_y,
    int dest_width, int dest_height,
    DRAW_CALLBACK *out_func, void **out_ctx);

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

void button(void (*CallbackFunction)(void), char *str,
            int x, int y,
            int fgr, int fgg, int fgb,            // Text color
            float scale_x, float scale_y,   // Text scaling
            int bgr, int bgg, int bgb,      // Background color
            int pt, int pb, int pl, int pr); // Padding: top, bottom, left, right

void buttonBySize(void (*CallbackFunction)(void), char *str,
                  int x, int y,
                  int fgr, int fgg, int fgb,
                  float scale_x, float scale_y,
                  int bgr, int bgg, int bgb,
                  int width, int height);

#endif