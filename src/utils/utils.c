#include "utils.h"
#include "../graphics/graphics.h"
#include "../fonts/characters_AtariST8x16SystemFont.h"

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

// void print_status(const char *msg, bool ok)
// {
//     VBEInfoBlock *VBE = (VBEInfoBlock *)VBEInfoAddress;
//     int term_width = VBE->x_resolution;
//     int x = 0;
//     extern int cursor_x, cursor_y; // Use terminal's cursor
//     x = cursor_x;

//     // Print the message
//     print(msg);
//     int msg_len = 0;
//     for (const char *p = msg; *p; ++p)
//         msg_len++;
//     int px = x + msg_len * FONT_ATARIST8X16SYSTEMFONT_WIDTH;

//     // Calculate how many dots to fill
//     int status_width = 7 * FONT_ATARIST8X16SYSTEMFONT_WIDTH; // e.g. " [OK] "
//     int dots = (term_width - px - status_width) / FONT_ATARIST8X16SYSTEMFONT_WIDTH;
//     for (int i = 0; i < dots; ++i)
//     {
//         terminal_putchar('.');
//     }
//     // Print space before status
//     terminal_putchar(' ');
//     // Print status box
//     int status_x = cursor_x;
//     int status_y = cursor_y;
//     int color_r = ok ? 0 : 255;
//     int color_g = ok ? 255 : 0;
//     int color_b = 0;
//     // Draw background box
//     DrawRect(status_x, status_y, 5 * FONT_ATARIST8X16SYSTEMFONT_WIDTH, FONT_ATARIST8X16SYSTEMFONT_HEIGHT, color_r, color_g, color_b);
//     // Print [OK] or [FAIL] in box
//     const char *status_str = ok ? "[OK]" : "[FAIL]";
//     int text_r = 255, text_g = 255, text_b = 255;
//     DrawString(getAtariST8x16SystemFontCharacter, FONT_ATARIST8X16SYSTEMFONT_WIDTH, FONT_ATARIST8X16SYSTEMFONT_HEIGHT, (char *)status_str, status_x, status_y, text_r, text_g, text_b, 1);
//     cursor_x += 5 * FONT_ATARIST8X16SYSTEMFONT_WIDTH;
//     Flush();
//     // Move to next line
//     terminal_putchar('\n');
// }
