#ifndef I2C_H
#define I2C_H

#include <stdint.h>

int I2C_Init();

void I2C_Start();

void I2C_Write_Address(int addr, uint8_t mode);

void I2C_Write_Byte(uint8_t byte);

void I2C_Stop();

uint8_t I2C_Read_Byte(uint8_t mode);

uint8_t BCD_TO_DEC(uint8_t bcd);

#endif // I2C_H
