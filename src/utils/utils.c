#include "utils.h"
#include <stdint.h>
#include "debug/simple_serial.h"
#include "drivers/output/vigfx/vigfx.h"

void int_to_ascii(int num, char *str)
{
    int i = 0, is_negative = 0;

    if (num == 0)
    {
        str[i++] = '0';
        str[i] = '\0';
        return;
    }

    if (num < 0)
    {
        is_negative = 1;
        num = -num;
    }

    while (num)
    {
        str[i++] = (num % 10) + '0';
        num /= 10;
    }

    if (is_negative)
    {
        str[i++] = '-';
    }

    str[i] = '\0';

    for (int j = 0, k = i - 1; j < k; j++, k--)
    {
        char temp = str[j];
        str[j] = str[k];
        str[k] = temp;
    }
}

int almost_equal(double a, double b, double epsilon)
{
    double diff = a - b;
    return (diff < 0 ? -diff : diff) < epsilon;
}

// Stub print_hex32 function
extern void print_hex32(uint32_t value);

// Clear screen using GPU framebuffer
void kernel_clear_screen_rgb(uint8_t r, uint8_t g, uint8_t b)
{
    simple_serial_puts("DEBUG: kernel_clear_screen_rgb called\n");
    // Create a persistent context and command buffer for screen clearing
    struct vigfx_context *ctx = vigfx_create_context(NULL);
    if (!ctx)
    {
        simple_serial_puts("DEBUG: Failed to create ViGFX context\n");
        return;
    }
    struct vigfx_command_buffer *cmd = vigfx_create_command_buffer(NULL);
    if (!cmd)
    {
        simple_serial_puts("DEBUG: Failed to create ViGFX command buffer\n");
        vigfx_destroy_context(ctx);
        return;
    }
    // Begin command buffer recording
    vigfx_begin_command_buffer(cmd);
    // Convert RGB values to float (0.0-1.0)
    float fr = r / 255.0f;
    float fg = g / 255.0f;
    float fb = b / 255.0f;
    // Clear the framebuffer with the specified color
    vigfx_cmd_clear(cmd, fr, fg, fb, 1.0f);
    // End command buffer recording
    vigfx_end_command_buffer(cmd);
    // Submit the command buffer to the GPU
    vigfx_submit_command_buffer(ctx, cmd);
    // Present the cleared frame to the display
    vigfx_present(ctx, NULL);
    // Clean up resources
    vigfx_destroy_command_buffer(cmd);
    vigfx_destroy_context(ctx);
    simple_serial_puts("DEBUG: Screen cleared and presented\n");
}
