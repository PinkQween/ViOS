#include "isr80h.h"
#include "io.h"
#include "idt/idt.h"
#include "heap.h"
#include "kernel.h"
#include "process.h"
#include "file.h"
#include "vix_graphics.h"

void isr80h_register_commands()
{
    isr80h_register_command(SYSTEM_COMMAND0_EXIT, isr80h_command0_exit);
    isr80h_register_command(SYSTEM_COMMAND1_PRINT, isr80h_command1_print);
    isr80h_register_command(SYSTEM_COMMAND2_GETKEY, isr80h_command2_getkey);
    isr80h_register_command(SYSTEM_COMMAND3_PUTCHAR, isr80h_command3_putchar);
    isr80h_register_command(SYSTEM_COMMAND4_MALLOC, isr80h_command4_malloc);
    isr80h_register_command(SYSTEM_COMMAND5_FREE, isr80h_command5_free);
    isr80h_register_command(SYSTEM_COMMAND6_PROCESS_LOAD_START, isr80h_command6_process_load_start);
    isr80h_register_command(SYSTEM_COMMAND7_INVOKE_SYSTEM_COMMAND, isr80h_command7_invoke_system_command);
    isr80h_register_command(SYSTEM_COMMAND8_GET_PROGRAM_ARGUMENTS, isr80h_command8_get_program_arguments);
    isr80h_register_command(SYSTEM_COMMAND9_SLEEP, isr80h_command9_sleep);
    isr80h_register_command(SYSTEM_COMMAND10_READ, isr80h_command10_read);
    
    isr80h_register_command(SYSTEM_COMMAND11_VIX_DRAW_PIXEL, isr80h_command11_vix_draw_pixel);
    isr80h_register_command(SYSTEM_COMMAND12_VIX_DRAW_RECT, isr80h_command12_vix_draw_rect);
    isr80h_register_command(SYSTEM_COMMAND13_VIX_FILL_RECT, isr80h_command13_vix_fill_rect);
    isr80h_register_command(SYSTEM_COMMAND14_VIX_CLEAR_SCREEN, isr80h_command14_vix_clear_screen);
    isr80h_register_command(SYSTEM_COMMAND15_VIX_PRESENT_FRAME, isr80h_command15_vix_present_frame);
    isr80h_register_command(SYSTEM_COMMAND16_VIX_GET_SCREEN_INFO, isr80h_command16_vix_get_screen_info);
    isr80h_register_command(SYSTEM_COMMAND17_VIX_DRAW_LINE, isr80h_command17_vix_draw_line);
    isr80h_register_command(SYSTEM_COMMAND18_VIX_DRAW_CIRCLE, isr80h_command18_vix_draw_circle);
    isr80h_register_command(SYSTEM_COMMAND19_VIX_FILL_CIRCLE, isr80h_command19_vix_fill_circle);
}
