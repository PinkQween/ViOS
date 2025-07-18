#include "rtc.h"
#include <stdint.h>
#include <stddef.h>
#include "drivers/io/io.h"

#define CMOS_ADDRESS 0x70
#define CMOS_DATA 0x71

// Read a register from the RTC CMOS
static uint8_t rtc_read_register(uint8_t reg)
{
    outb(CMOS_ADDRESS, reg);
    uint8_t value;
    insb(CMOS_DATA, &value, 1);
    return value;
}

// Check if RTC update is in progress
static int is_update_in_progress()
{
    outb(CMOS_ADDRESS, 0x0A);
    return (inb(CMOS_DATA) & 0x80);
}

// Convert BCD to binary
static uint8_t bcd_to_bin(uint8_t val)
{
    return (val & 0x0F) + ((val / 16) * 10);
}

// Read the current time from the RTC into the provided struct
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

// Initialize the RTC (currently a stub for future use)
void rtc_init()
{
    // No initialization needed for basic read-only RTC
}
