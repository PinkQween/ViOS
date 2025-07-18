#ifndef UTILS_H
#define UTILS_H

#include <stdbool.h>
#include <stddef.h>

#include <stdint.h>

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

#ifdef __cplusplus
extern "C"
{
#endif

    // Clears the framebuffer to the specified RGB color (0xRRGGBB)
    void kernel_clear_screen_rgb(uint8_t r, uint8_t g, uint8_t b);

#ifdef __cplusplus
}
#endif

#endif
