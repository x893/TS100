#ifndef _HARDWARE_H
#define _HARDWARE_H

#include "STM32F10x.h"

#define SI_COE 8//56
#define SI_THRESHOLD   60

typedef enum VOL_CLASS {
	H_ALARM = 0,
	VOL_24,
	VOL_19,
	VOL_12,
	VOL_5,
	L_ALARM,
} VOL_CLASS;

typedef struct {
	int32_t  gZerop_ad;
	uint32_t gTurn_offv;
	uint16_t gKey_in;
	uint8_t gAlarm_type;
	uint8_t gCalib_flag;
	uint8_t gLongkey_flag;
} Hard_Context_t;

extern Hard_Context_t Hard_Context;

uint8_t Get_gCalib_flag(void);
void Set_gCalib_flag(uint8_t flag);

uint32_t Get_gTurn_offv(void);
void Set_gTurn_offv(uint32_t value);

int32_t Get_gZerop_ad(void);
void Set_gZerop_ad(int32_t value);

uint16_t Get_gKey(void);
void Set_gKey(uint16_t key);
void Set_LongKeyFlag(uint32_t flag);

void Zero_Calibration(void);
int Read_Vb(uint8_t flag);
uint32_t Get_SlAvg(uint32_t avg_data);
int Get_TempSlAvg(int avg_data);
uint32_t Get_AvgAd(void);
int Get_SensorTmp(void);
uint16_t Get_ThermometerTemp(void);
int16_t Get_Temp(s16 wk_temp);
uint32_t Clear_Watchdog(void);
uint32_t Start_Watchdog(uint32_t ms);
uint8_t Get_AlarmType(void);
void Set_AlarmType(uint8_t type);
uint32_t Get_CalFlag(void);

void Key_Read( void );

#endif
