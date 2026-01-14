#ifndef KERNEL_TSC_H
#define KERNEL_TSC_H

#include <stdint.h>

typedef uint64_t TIME_TSC;
typedef uint64_t TIME_MICROSECONDS;
typedef uint64_t TIME_MILLISECONDS;
typedef uint64_t TIME_SECONDS;

void call_pause(void);
void udelay(TIME_MICROSECONDS microseconds);
TIME_SECONDS tsc_seconds();
TIME_MILLISECONDS tsc_milliseconds();
TIME_MICROSECONDS tsc_microseconds();
TIME_TSC tsc_frequency(void);
TIME_TSC read_tsc(void);

#endif