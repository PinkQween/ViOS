#include "vios.h"
#include "string.h"

struct command_argument *vios_parse_command(const char *command, int max)
{
    struct command_argument *root_command = 0;
    char scommand[1025];
    if (max >= (int)sizeof(scommand))
    {
        return 0;
    }

    strncpy(scommand, command, sizeof(scommand));
    scommand[sizeof(scommand) - 1] = 0; // Ensure null-termination
    char *token = strtok(scommand, " ");
    if (!token)
    {
        goto out;
    }

    root_command = vios_malloc(sizeof(struct command_argument));
    if (!root_command)
    {
        goto out;
    }

    strncpy(root_command->argument, token, sizeof(root_command->argument));
    root_command->argument[sizeof(root_command->argument) - 1] = 0; // Ensure null-termination
    root_command->next = 0;

    struct command_argument *current = root_command;
    token = strtok(NULL, " ");
    while (token != 0)
    {
        struct command_argument *new_command = vios_malloc(sizeof(struct command_argument));
        if (!new_command)
        {
            break;
        }

        strncpy(new_command->argument, token, sizeof(new_command->argument));
        new_command->argument[sizeof(new_command->argument) - 1] = 0; // Ensure null-termination
        new_command->next = 0x00;
        current->next = new_command;
        current = new_command;
        token = strtok(NULL, " ");
    }
out:
    return root_command;
}

int vios_getkeyblock()
{
    int val = 0;
    do
    {
        val = vios_getkey();
    } while (val == 0);
    return val;
}

int vios_system_run(const char *command)
{
    char buf[1024];
    strncpy(buf, command, sizeof(buf));
    buf[sizeof(buf) - 1] = 0;
    struct command_argument *root_command_argument = vios_parse_command(buf, sizeof(buf));
    buf[sizeof(buf) - 1] = 0;
    if (!root_command_argument)
    {
        return -1;
    }

    int result = vios_system(root_command_argument);
    // Clean up allocated command arguments
    struct command_argument *current = root_command_argument;
    while (current)
    {
        struct command_argument *next = current->next;
        vios_free(current);
        current = next;
    }

    return result;
}

// VIX Graphics API Implementation
void vix_draw_pixel(int x, int y, uint32_t color)
{
    asm volatile("int $0x80" : : "a"(11), "b"(x), "c"(y), "d"(color) : "memory");
}

void vix_draw_rect(int x, int y, int width, int height, uint32_t color)
{
    asm volatile("int $0x80" : : "a"(12), "b"(x), "c"(y), "d"(width), "S"(height), "D"(color) : "memory");
}

void vix_fill_rect(int x, int y, int width, int height, uint32_t color)
{
    asm volatile("int $0x80" : : "a"(13), "b"(x), "c"(y), "d"(width), "S"(height), "D"(color) : "memory");
}

void vix_clear_screen(uint32_t color)
{
    asm volatile("int $0x80" : : "a"(14), "b"(color) : "memory");
}

void vix_present_frame(void)
{
    asm volatile("int $0x80" : : "a"(15) : "memory");
}

void vix_get_screen_info(vix_screen_info_t *info)
{
    asm volatile("int $0x80" : : "a"(16), "b"(info) : "memory");
}

void vix_draw_line(int x1, int y1, int x2, int y2, uint32_t color)
{
    asm volatile("int $0x80" : : "a"(17), "b"(x1), "c"(y1), "d"(x2), "S"(y2), "D"(color) : "memory");
}

void vix_draw_circle(int x, int y, int radius, uint32_t color)
{
    asm volatile("int $0x80" : : "a"(18), "b"(x), "c"(y), "d"(radius), "S"(color) : "memory");
}

void vix_fill_circle(int x, int y, int radius, uint32_t color)
{
    asm volatile("int $0x80" : : "a"(19), "b"(x), "c"(y), "d"(radius), "S"(color) : "memory");
}
