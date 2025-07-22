#ifndef KERNEL_MAINLOOP_H
#define KERNEL_MAINLOOP_H

struct mouse;

/**
 * Run the main kernel loop
 * @param mouse Pointer to mouse interface
 */
void kernel_run_main_loop();

#endif // KERNEL_MAINLOOP_H
