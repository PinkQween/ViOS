#include "vios.h"
#include "stdio.h"

void _start()
{
    vix_screen_info_t screen_info;
    vix_get_screen_info(&screen_info);
    
    printf("VIX Graphics Demo\n");
    printf("Screen: %dx%d, %d bpp\n", screen_info.width, screen_info.height, screen_info.bpp);
    
    // Clear screen to black
    vix_clear_screen(VIX_COLOR_BLACK);
    
    // Draw some basic shapes
    vix_fill_rect(50, 50, 100, 100, VIX_COLOR_RED);
    vix_fill_rect(200, 50, 100, 100, VIX_COLOR_GREEN);
    vix_fill_rect(350, 50, 100, 100, VIX_COLOR_BLUE);
    
    // Draw circles
    vix_fill_circle(100, 250, 50, VIX_COLOR_YELLOW);
    vix_fill_circle(250, 250, 50, VIX_COLOR_CYAN);
    vix_fill_circle(400, 250, 50, VIX_COLOR_MAGENTA);
    
    // Draw lines
    vix_draw_line(50, 350, 450, 350, VIX_COLOR_WHITE);
    vix_draw_line(250, 300, 250, 400, VIX_COLOR_WHITE);
    
    // Draw outline rectangles
    vix_draw_rect(25, 25, 150, 150, VIX_COLOR_WHITE);
    vix_draw_rect(175, 25, 150, 150, VIX_COLOR_WHITE);
    vix_draw_rect(325, 25, 150, 150, VIX_COLOR_WHITE);
    
    // Draw some pixels
    for (int i = 0; i < 100; i++) {
        vix_draw_pixel(500 + i, 100 + i, VIX_COLOR_RED);
        vix_draw_pixel(500 + i, 200 - i, VIX_COLOR_GREEN);
    }
    
    // Present the final frame
    vix_present_frame();
    
    printf("VIX Graphics Demo Complete!\n");
    printf("Press any key to exit...\n");
    
    // Wait for keypress
    vios_getkeyblock();
    
    // Clear screen before exiting
    vix_clear_screen(VIX_COLOR_BLACK);
    vix_present_frame();
}
