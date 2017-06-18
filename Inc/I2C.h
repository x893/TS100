#ifndef __I2C_H
#define __I2C_H

#include <stdint.h>

void Delay_uS(uint32_t us);
void I2C_PageWrite(uint8_t * pbuf, uint16_t numbyte, uint8_t deviceaddr );
void I2C_PageRead(uint8_t * pbuf,  uint16_t numbyte, uint8_t deviceaddr, uint8_t readaddr);

#endif
