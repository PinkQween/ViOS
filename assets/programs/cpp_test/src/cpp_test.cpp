#include <stdio.h>
#include <termios.h>
#include <unistd.h>

void set_raw_mode(int enable)
{
    static struct termios oldt, newt;
    if (enable)
    {
        tcgetattr(STDIN_FILENO, &oldt); // save old settings
        newt = oldt;
        newt.c_lflag &= ~(ICANON | ECHO);        // disable buffering and echo
        tcsetattr(STDIN_FILENO, TCSANOW, &newt); // apply new settings
    }
    else
    {
        tcsetattr(STDIN_FILENO, TCSANOW, &oldt); // restore old settings
    }
}

int main(int argc, char *argv[])
{
    // Print all arguments
    printf("Program called with %d arguments:\n", argc);
    for (int i = 0; i < argc; i++)
    {
        printf("argv[%d] = %s\n", i, argv[i]);
    }

    printf("\nStart typing. Press 'f' to stop.\n");

    set_raw_mode(1); // Enable raw mode

    int ch;
    while ((ch = getchar()) != 'f')
    {
        printf("You pressed: '%c' (ASCII: %d)\n", ch, ch);
    }

    set_raw_mode(0); // Restore terminal mode

    printf("You pressed 'f'. Exiting.\n");
    return 0;
}