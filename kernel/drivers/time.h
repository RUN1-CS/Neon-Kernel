#ifndef TIME_H
#define TIME_H

#include <stdint.h>

#define RTC_I2C_ADDRESS 0x68 // Standard DS3231 I2C address
#define BCD_TO_DEC(val) (((val & 0xF0) >> 4) * 10 + (val & 0x0F))

int leap_year(uint16_t year);

void epoch_to_date(uint32_t epoch, SystemDateTime *dt);

#endif // TIME_H
