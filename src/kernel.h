#ifndef KERNEL_H
#define KERNEL_H

#define VGA_WIDTH 80
#define VGA_HEIGHT 20

#define VIOS_MAX_PATH 128

void kernel_main();
void print(const char *str);

#endif