#include "utils.h"
#include <stdint.h>
#include "debug/simple_serial.h"
#include "drivers/output/vigfx/vigfx.h"
#include "memory/heap/kheap.h"
#include "fs/file.h"
#include "kernel.h"

// Font descriptor definitions
const struct font_descriptor FONT_ATARIST8X16 = {
    .get_character = getAtariST8x16SystemFontCharacter,
    .get_advance = getAtariST8x16SystemFontAdvance,
    .get_kerning = getAtariST8x16SystemFontKerning,
    .width = FONT_ATARIST8X16SYSTEMFONT_WIDTH,
    .height = FONT_ATARIST8X16SYSTEMFONT_HEIGHT};

const struct font_descriptor FONT_ARIAL = {
    .get_character = getArialCharacter,
    .get_advance = getArialAdvance,
    .get_kerning = getArialKerning,
    .width = FONT_ARIAL_WIDTH,
    .height = FONT_ARIAL_HEIGHT};

const struct font_descriptor FONT_ROBOTOTHIN = {
    .get_character = getRobotoThinCharacter,
    .get_advance = getRobotoThinAdvance,
    .get_kerning = getRobotoThinKerning,
    .width = FONT_ROBOTOTHIN_WIDTH,
    .height = FONT_ROBOTOTHIN_HEIGHT};

const struct font_descriptor FONT_CHERI = {
    .get_character = getCheriCharacter,
    .get_advance = getCheriAdvance,
    .get_kerning = getCheriKerning,
    .width = FONT_CHERI_WIDTH,
    .height = FONT_CHERI_HEIGHT};

const struct font_descriptor FONT_BRIGHTLY = {
    .get_character = getBrightlyCharacter,
    .get_advance = getBrightlyAdvance,
    .get_kerning = getBrightlyKerning,
    .width = FONT_BRIGHTLY_WIDTH,
    .height = FONT_BRIGHTLY_HEIGHT};

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

void draw_scaled_rgb_fill(const char *path, int start_x, int start_y, int dest_width, int dest_height)
{
    int fd = fopen(path, "r");
    if (ISERR(fd))
    {
        simple_serial_puts("Failed to open .rgb file.\n");
        return;
    }

    uint32_t src_width = 0, src_height = 0;
    if (fread(&src_width, sizeof(uint32_t), 1, fd) != 1)
    {
        simple_serial_puts("Failed to read width.\n");
        fclose(fd);
        return;
    }

    if (fread(&src_height, sizeof(uint32_t), 1, fd) != 1)
    {
        simple_serial_puts("Failed to read height.\n");
        fclose(fd);
        return;
    }

    if (src_width == 0 || src_height == 0 || dest_width == 0 || dest_height == 0)
    {
        simple_serial_puts("Invalid image or destination dimensions.\n");
        fclose(fd);
        return;
    }

    int image_size = src_width * src_height * 3;

    if (image_size <= 0)
    {
        simple_serial_puts("Invalid image size.\n");
        fclose(fd);
        return;
    }

    uint8_t *rgb_data = kzalloc(image_size);

    if (!rgb_data)
    {
        simple_serial_puts("Memory allocation failed.\n");
        fclose(fd);
        return;
    }

    if (fread(rgb_data, image_size, 1, fd) != 1)
    {
        simple_serial_puts("Failed to read image data.\n");
        kfree(rgb_data);
        fclose(fd);
        return;
    }

    // Use floating-point scaling for better accuracy
    float scale_x = (float)src_width / dest_width;
    float scale_y = (float)src_height / dest_height;

    for (int y = 0; y < dest_height; y++)
    {
        int src_y = (int)(y * scale_y);
        if (src_y >= (int)src_height)
            src_y = src_height - 1;
        for (int x = 0; x < dest_width; x++)
        {
            int src_x = (int)(x * scale_x);
            if (src_x >= (int)src_width)
                src_x = src_width - 1;
            int idx = (src_y * src_width + src_x) * 3;
            if (idx < 0 || idx + 2 >= image_size)
            {
                // Should never happen, but safety check
                continue;
            }
            int r = rgb_data[idx];
            int g = rgb_data[idx + 1];
            int b = rgb_data[idx + 2];
            gpu_draw(start_x + x, start_y + y, r, g, b);
        }
    }

    kfree(rgb_data);
    fclose(fd);
}

void DrawCharacterScaled(
    int (*f)(int, int),
    int font_width, int font_height,
    char character,
    int x, int y,
    int r, int g, int b,
    float scale_x, float scale_y)
{
    for (int j = 0; j < font_height; j++)
    {
        unsigned int row = (*f)((int)(character), j);
        int shift = font_width - 1;

        for (int i = 0; i < font_width; i++)
        {
            int bit_val = (row >> shift) & 1;
            if (bit_val == 1)
            {
                // Calculate scaled pixel box coordinates
                float start_x = x + i * scale_x;
                float end_x = x + (i + 1) * scale_x;
                float start_y = y + j * scale_y;
                float end_y = y + (j + 1) * scale_y;

                for (int draw_y = (int)start_y; draw_y < (int)end_y; draw_y++)
                {
                    for (int draw_x = (int)start_x; draw_x < (int)end_x; draw_x++)
                    {
                        gpu_draw(draw_x, draw_y, r, g, b);
                    }
                }
            }
            shift--;
        }
    }
}

void DrawStringScaled(
    int (*f)(int, int),
    int font_width, int font_height,
    char *string,
    int x, int y,
    int r, int g, int b,
    float scale_x, float scale_y,
    int (*get_advance)(int),
    int (*get_kerning)(int, int))
{
    float pen_x = 0.0f, pen_y = 0.0f;
    int prev_char = -1;

    for (int k = 0; string[k] != '\0'; k++)
    {
        char curr_char = string[k];
        if (curr_char != '\n')
        {
            if (prev_char != -1 && get_kerning)
                pen_x += get_kerning(prev_char, curr_char) * scale_x;

            DrawCharacterScaled(
                f, font_width, font_height,
                curr_char,
                x + (int)pen_x, y + (int)pen_y,
                r, g, b,
                scale_x, scale_y);

            if (get_advance)
                pen_x += get_advance(curr_char) * scale_x;
            else
                pen_x += font_width * scale_x;
        }

        if (curr_char == '\n')
        {
            pen_x = 0;
            pen_y += font_height * scale_y;
            prev_char = -1;
        }
        else
        {
            prev_char = curr_char;
        }
    }
}

void print_ex(const struct font_descriptor *font, char *str, int x, int y, int r, int g, int b, float scale_x, float scale_y)
{
    DrawStringScaled(
        font->get_character,
        font->width,
        font->height,
        str, x, y,
        r, g, b,
        scale_x, scale_y,
        font->get_advance,
        font->get_kerning);
}

void print(char *str, int x, int y, int r, int g, int b, float scale_x, float scale_y)
{
    print_ex(&FONT_ATARIST8X16, str, x, y, r, g, b, scale_x, scale_y);
}