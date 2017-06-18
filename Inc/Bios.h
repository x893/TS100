#ifndef __BIOS_H
#define __BIOS_H

#include "STM32F10x.h"

extern volatile u32 gTime[];

#define USB_DN_OUT()	GPIOA->CRH = (GPIOA->CRH & 0xFFFF3FFF) | 0x00003000
#define USB_DP_OUT()	GPIOA->CRH = (GPIOA->CRH & 0xFFF3FFFF) | 0x00030000

#define USB_DN_EN()		GPIOA->CRH = (GPIOA->CRH & 0xFFFFBFFF) | 0x0000B000
#define USB_DP_EN()		GPIOA->CRH = (GPIOA->CRH & 0xFFFBFFFF) | 0x000B0000

#define USB_DP_PD()		GPIOA->CRH = (GPIOA->CRH & 0xFFF3FFFF) | 0x00030000

#define USB_DN_HIGH()	GPIOA->BSRR  = GPIO_Pin_11
#define USB_DP_HIGH()	GPIOA->BSRR  = GPIO_Pin_12

#define USB_DN_LOW()	GPIOA->BRR  = GPIO_Pin_11
#define USB_DP_LOW()	GPIOA->BRR  = GPIO_Pin_12

#define LOW			0
#define HIGH		1

#define BLINK		1		// Bit0 : 0/1 œ‘ æ/…¡À∏◊¥Ã¨±Í÷æ
#define WAIT_TIMES	100000


#define SECTOR_SIZE		512
#define SECTOR_CNT		4096
#define HEAT_T			200

u32 Get_HeatingTime(void);
void Set_HeatingTime(uint32_t heating_time);
void Clear_HeatingTime(void);
// uint16_t Get_AdcValue(int i);
uint16_t Get_TMP36(void);	// Get_AdcValue(0);
uint16_t Get_IRON(void);	// Get_AdcValue(1);
void Init_Gtime(void);
void Delay_Ms(u32 ms);
void Delay_HalfMs(u32 ms);
void USB_Port(u8 state);
void NVIC_Config(u16 tab_offset);
void RCC_Config(void);
void GPIO_Config(void);
void Adc_Init(void);
void Init_Timers(void);

#endif
