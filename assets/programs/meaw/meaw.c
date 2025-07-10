#include "vios.h"
#include "stdlib.h"
#include "stdio.h"
#include "string.h"

int main(int argc, char **argv)
{
    // Clear screen effect by printing many newlines
    for (int i = 0; i < 25; i++) {
        print("\n");
    }
    
    // Create a rainbow pattern using different representations of "cat"
    const char* rainbow_cats[] = {
        "CAT",     // Red
        "cat",     // Orange  
        "C@T",     // Yellow
        "c4t",     // Green
        "CaT",     // Blue
        "c@t",     // Indigo
        "C4T",     // Violet
        "*cat*",   // Special
        "~CAT~",   // Special
        "[cat]"    // Special
    };
    
    int num_patterns = 10;
    int screen_width = 80;
    int screen_height = 25;
    
    // Fill the entire screen with colorful cat patterns
    for (int row = 0; row < screen_height; row++) {
        for (int col = 0; col < screen_width; col += 4) {
            // Choose pattern based on position for rainbow effect
            int pattern_index = (row + col/4) % num_patterns;
            print(rainbow_cats[pattern_index]);
            
            // Add space if we're not at the end of line
            if (col + 4 < screen_width) {
                print(" ");
            }
        }
        print("\n");
    }
    
    // Add a special meaw message at the bottom
    print("\n");
    print("    ~~ MEAW! text test ~~\n");
    print("  Press any key to return to shell...\n");
    
    // Wait for user input
    vios_getkeyblock();
    
    return 0;
}
