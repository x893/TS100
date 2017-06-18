#ifndef __MMA8652FC__H
#define __MMA8652FC__H

#include <stdint.h>

//--------------MMA8652 定义部分----------------------------------------------//

#define DEVICE_ADDR	 0X1D

//--------------MMA8652 寄存器定义-------------------------------------------//
#define STATUS_REG			0x00
#define X_MSB_REG			0x01
#define X_LSB_REG			0x02
#define Y_MSB_REG			0x03
#define Y_LSB_REG			0x04
#define Z_MSB_REG			0x05
#define Z_LSB_REG			0x06

#define TRIG_CFG			0x0A
#define SYSMOD				0x0B
#define INT_SOURCE			0x0C
#define DEVICE_ID			0x0D

#define XYZ_DATA_CFG_REG	0x0E

#define CTRL_REG1			0x2A	  // System Control 1 register
#define CTRL_REG2			0x2B	  // System Control 2 register
#define CTRL_REG3			0x2C	  //
#define CTRL_REG4			0x2D	  // Interrupt Enable register
#define CTRL_REG5			0x2E	  //

//-----STATUS_REG(0x00)-----Bit Define----------------------------------------//
#define ZYXDR_BIT			0x08
//----XYZ_DATA_CFG_REG(0xE)-Bit Define----------------------------------------//
#define FS_MASK				0x03
#define FULL_SCALE_2G		0x00	// 2G = 0x0, 4G = 0x1, 8G = 0x2
#define FULL_SCALE_4G		0x01
#define FULL_SCALE_8G		0x02
//---------CTRL_REG1(0X2A)Bit Define------------------------------------------//
#define ACTIVE_MASK			(1 << 0)
#define DR_MASK				(0x7 << 3)
#define FHZ800				(0x0 << 3)	// 800hz
#define FHZ400				(0x1 << 3)	// 400hz
#define FHZ200				(0x2 << 3)	// 200hz
#define FHZ100				(0x3 << 3)	// 100hz
#define FHZ50				(0x4 << 3)	// 50hz
#define FHZ2				(0x5 << 3)	// 12.5hz
#define FHZ1				(0x6 << 3)	// 6.25hz
#define FHZ0				(0x7 << 3)	// 1.563hz
#define DataRateValue		FHZ100
//---------CTRL_REG2(0X2B)Bit Define------------------------------------------// 
#define MODS_MASK			0x03	// Oversampling Mode 4
#define Normal_Mode			0x0		// Normal=0, Low Noise Low Power MODS=1,
									// HI RESOLUTION=2, LOW POWER MODS = 11
//----CTRL_REG4---Interrupt Enable BIT ---------------------------------------//   
//0 interrupt is disabled (default) 
//1 interrupt is enabled	 
#define INT_EN_ASLP			(1 << 7)	// Auto-SLEEP/WAKE Interrupt Enable 
#define INT_EN_FIFO			(1 << 6)	// FIFO Interrupt Enable
#define INT_EN_TRANS		(1 << 5)	// Transient Interrupt Enable
#define INT_EN_LNDPRT		(1 << 4)	// Orientation(Landscape/Portrait)Interrupt Enable
#define INT_EN_PULSE		(1 << 3)	// Pulse Detection Interrupt Enable
#define INT_EN_FF_MT		(1 << 2)	// Freefall/Motion Interrupt Enable
#define INT_EN_DRDY			(1 << 0)	// Data Ready Interrupt Enable

uint8_t Get_MmaShift(void);
uint8_t Get_MmaActive(void);
uint16_t Cheak_XYData(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);
uint16_t Update_X(void);
uint16_t Update_Y(void);
uint16_t Update_Z(void);
void Check_Accelerated(void);
void StartUp_Accelerated(void);

#endif
