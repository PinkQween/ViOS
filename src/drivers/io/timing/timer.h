#ifndef SRC_TIMER_H
#define SRC_TIMER_H

#include <stdint.h>
#include "rtc.h"

// Initialize all timer hardware (RTC, PIT, etc.)
void timer_init();

// Sleep for the specified number of milliseconds (uses PIT)
void timer_sleep_ms(int ms);

// Sleep for the specified number of seconds (uses RTC)
void timer_sleep_seconds(int seconds);

// POSIX-style sleep functions
void sleep(unsigned int seconds);
void usleep(unsigned int usec);
int nanosleep(unsigned int nsec);

// Get the current time (delegates to RTC)
void timer_get_time(struct rtc_time *time);

// Returns the number of timer ticks since boot (ms granularity)
unsigned long timer_get_ticks();

void timer_tick();

#endif // SRC_TIMER_H