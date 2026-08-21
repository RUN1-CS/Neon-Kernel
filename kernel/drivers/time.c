#ifndef TIME_C
#define TIME_C

#include "time.h"

// SystemDate keeps track of years, months and days
typedef struct {
  uint16_t year;
  uint8_t month;
  uint8_t day;
} SystemDate;

// SystemTime keeps track of hours, minutes and seconds
typedef struct {
  uint8_t hour;
  uint8_t minute;
  uint8_t second;
} SystemTime;

// SystemDateTime is a combination of the two above
typedef struct {
  SystemDate date;
  SystemTime time;
} SystemDateTime;

// Read the RTC chip
void read_rtc(SystemDateTime *dt) {
  uint8_t reg_ptr = 0x00; // Reading seconds register
  uint8_t raw_data[7];    // Buffer holding 7 time parameters

  // 1. Start reading the from 0x00
  I2C_Start();
  //I2C_Write_Address(RTC_I2C_ADDRESS, I2C_WRITE_MODE);
  I2C_Write_Byte(reg_ptr);
  I2C_Stop();

  // 2. Reading 7 bytes
  I2C_Start();
  //I2C_Write_Address(RTC_I2C_ADDRESS, I2C_READ_MODE);

  for (int i = 0; i < 6; i++) {
    //raw_data[i] = I2C_Read_Byte(ACK); // Send next byte
  }
  //raw_data[6] = I2C_Read_Byte(NACK); // Done
  I2C_Stop();

  // 3. Converting BDC data to DEC integers
  dt->time.second = BCD_TO_DEC(raw_data[0] & 0x7F);
  dt->time.minute = BCD_TO_DEC(raw_data[1]);
  dt->time.hour = BCD_TO_DEC(raw_data[2] & 0x3F);
  dt->date.day = BCD_TO_DEC(raw_data[4]);
  dt->date.month = BCD_TO_DEC(raw_data[5] & 0x1F);

  // 4. RTCs save only two last digits
  dt->date.year = BCD_TO_DEC(raw_data[6]) + 2000;
}

// Calculate if a year is a leap year
int leap_year(uint16_t year) {
  return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

void epoch_to_date(uint32_t epoch, SystemDateTime *dt) {
  uint32_t minutes, hours, days;

  // 1. Get time of day
  dt->time.second = epoch % 60;
  minutes = epoch / 60;
  dt->time.minute = minutes % 60;
  hours = minutes / 60;
  dt->time.hour = hours % 24;

  // 2. shift to total since
  days = hours / 24;

  // 3. Calculate year
  dt->date.year = 1970;
  while (1) {
    uint16_t days_in_year = leap_year(dt->date.year) ? 366 : 365;
    if (days < days_in_year) {
      break;
    }
    days -= days_in_year;
    dt->date.year++;
  }

  // 4. Calculate month
  dt->date.month = 0;
  for (uint8_t i = 1; i <= 12; i++) {
    uint8_t dim = i == 2 ? 28 : i % 2 == 0 ? 30 : 31;
    if (i == 2 && leap_year(dt->date.year)) {
      dim = 29;
    }

    if (days < dim) {
      dt->date.month = i;
      break;
    }
    days -= dim;
  }

  // 5. Calculate day
  dt->date.day = days + 1;
}

#endif TIME_C
