#include "rtc.h"
#include <stdint.h>
#include <stddef.h>
#include "io/io.h"

#define CMOS_ADDRESS 0x70
#define CMOS_DATA 0x71

static uint8_t rtc_read_register(uint8_t reg)
{
    outb(CMOS_ADDRESS, reg);
    uint8_t value;
    insb(CMOS_DATA, &value, 1);
    return value;
}

static int is_update_in_progress()
{
    outb(CMOS_ADDRESS, 0x0A);
    return (inb(CMOS_DATA) & 0x80);
}

static uint8_t bcd_to_bin(uint8_t val)
{
    return (val & 0x0F) + ((val / 16) * 10);
}

void rtc_read(struct rtc_time *time)
{
    if (!time)
        return;
    // Wait until update is not in progress
    while (is_update_in_progress())
        ;

    uint8_t second = rtc_read_register(0x00);
    uint8_t minute = rtc_read_register(0x02);
    uint8_t hour = rtc_read_register(0x04);
    uint8_t day = rtc_read_register(0x07);
    uint8_t month = rtc_read_register(0x08);
    uint8_t year = rtc_read_register(0x09);
    uint8_t reg_b = rtc_read_register(0x0B);

    // Convert BCD to binary if needed
    if (!(reg_b & 0x04))
    {
        second = bcd_to_bin(second);
        minute = bcd_to_bin(minute);
        hour = bcd_to_bin(hour);
        day = bcd_to_bin(day);
        month = bcd_to_bin(month);
        year = bcd_to_bin(year);
    }

    time->second = second;
    time->minute = minute;
    time->hour = hour;
    time->day = day;
    time->month = month;
    time->year = 2000 + year; // Assumes year is 00-99 (Y2K+)
}

void rtc_init()
{
    // No initialization needed for basic read-only RTC
}

void sleep_seconds(int seconds)
{
    if (seconds <= 0)
        return;

    struct rtc_time start, current;
    rtc_read(&start);

    int total_start_seconds = start.hour * 3600 + start.minute * 60 + start.second;
    int total_target_seconds = total_start_seconds + seconds;

    int total_current_seconds; // declare outside loop

    do
    {
        rtc_read(&current);
        total_current_seconds = current.hour * 3600 + current.minute * 60 + current.second;

        if (total_current_seconds < total_start_seconds)
        {
            total_current_seconds += 24 * 3600;
        }
    } while (total_current_seconds < total_target_seconds);
}

#define PIT_CHANNEL2_DATA 0x42
#define PIT_COMMAND 0x43

void sleep_ms(int ms)
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
