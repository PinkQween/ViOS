#include "timer.h"
#include "drivers/io/io.h"
#include <stddef.h>

// PIT (Programmable Interval Timer) definitions
#define PIT_CHANNEL0_DATA 0x40
#define PIT_CHANNEL2_DATA 0x42
#define PIT_COMMAND 0x43

// PIT frequency constants
#define PIT_FREQUENCY 1193182
#define DESIRED_TIMER_FREQUENCY 1000  // 1000 Hz for 1ms timer ticks

// Initialize all timer hardware (RTC, PIT, etc.)
void timer_init()
{
    // Calculate the divisor for the desired frequency
    int divisor = PIT_FREQUENCY / DESIRED_TIMER_FREQUENCY;
    
    // Send command to PIT control port:
    // Channel 0, Access mode: low byte then high byte, Mode 2: rate generator, Binary mode
    outb(PIT_COMMAND, 0x34);
    
    // Send the divisor (low byte first, then high byte)
    outb(PIT_CHANNEL0_DATA, divisor & 0xFF);
    outb(PIT_CHANNEL0_DATA, (divisor >> 8) & 0xFF);
}

// Sleep for the specified number of milliseconds (uses PIT)
void timer_sleep_ms(int ms)
{
    if (ms <= 0)
        return;

    // Calculate countdown value (approximate)
    // PIT runs at 1,193,182 Hz, so counts = ms * 1193
    int count = ms * 1193;

    // Setup PIT channel 2 in mode 0 (one-shot)
    outb(PIT_COMMAND, 0xB6);

    // Load low byte then high byte
    outb(PIT_CHANNEL2_DATA, count & 0xFF);
    outb(PIT_CHANNEL2_DATA, (count >> 8) & 0xFF);

    // Wait until counter reaches zero by polling bit 5 of port 0x61
    while (inb(0x61) & 0x20)
        ;
}

// Sleep for the specified number of seconds (uses RTC)
void timer_sleep_seconds(int seconds)
{
    if (seconds <= 0)
        return;

    struct rtc_time start, current;
    timer_get_time(&start);

    int total_start_seconds = start.hour * 3600 + start.minute * 60 + start.second;
    int total_target_seconds = total_start_seconds + seconds;

    int total_current_seconds;

    do
    {
        timer_get_time(&current);
        total_current_seconds = current.hour * 3600 + current.minute * 60 + current.second;

        if (total_current_seconds < total_start_seconds)
        {
            total_current_seconds += 24 * 3600;
        }
    } while (total_current_seconds < total_target_seconds);
}

// Sleep for the specified number of seconds (POSIX-style)
void sleep(unsigned int seconds)
{
    timer_sleep_seconds((int)seconds);
}

// Sleep for the specified number of microseconds (POSIX-style)
void usleep(unsigned int usec)
{
    if (usec >= 1000)
    {
        timer_sleep_ms(usec / 1000);
        usec = usec % 1000;
    }
    // Busy-wait for remaining microseconds (approximate)
    for (volatile unsigned int i = 0; i < usec * 10; ++i)
    {
        // Just wait
    }
}

// Sleep for the specified number of nanoseconds (POSIX-style)
int nanosleep(unsigned int nsec)
{
    if (nsec >= 1000000)
    {
        timer_sleep_ms(nsec / 1000000);
        nsec = nsec % 1000000;
    }
    if (nsec >= 1000)
    {
        // Busy-wait for remaining microseconds
        for (volatile unsigned int i = 0; i < (nsec / 1000) * 10; ++i)
        {
            // Just wait
        }
        nsec = nsec % 1000;
    }
    // Busy-wait for remaining nanoseconds (very approximate)
    for (volatile unsigned int i = 0; i < nsec / 100; ++i)
    {
        // Just wait
    }
    return 0;
}

// Get the current time (delegates to RTC)
void timer_get_time(struct rtc_time *time)
{
    rtc_read(time);
}

static volatile unsigned long timer_ticks = 0;

unsigned long timer_get_ticks()
{
    return timer_ticks;
}

// Call this from the timer interrupt handler
void timer_tick()
{
    timer_ticks++;
}