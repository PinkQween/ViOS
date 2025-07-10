#ifndef SRC_RTC_H
#define SRC_RTC_H

#include <stdint.h>

struct rtc_time
{
    uint8_t second;
    uint8_t minute;
    uint8_t hour;
    uint8_t day;
    uint8_t month;
    uint16_t year;
};

void rtc_init();
void sleep_seconds(int seconds);
void rtc_read(struct rtc_time *time);
void sleep_ms(int ms);

#endif // SRC_IO_RTC_H