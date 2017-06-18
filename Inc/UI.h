#ifndef _UI_H
#define _UI_H

#include "STM32F10x.h"

#define HIGHLIGHT_FREQUENCY	60
#define HIGHLTGHT_REDUCE	20
#define SET_PROMPT_X		9 
#define SET_INFO_X			8

u32 Get_UpdataFlag(void);
void Set_UpdataFlag(uint32_t Cont);
void APP_Init(void);
u32 Calculation_TWork(uint8_t Flag);
uint8_t Calculated_Digit(int number);
void Set_temperature(void);
void SetOpt_Proc(void);
void SetOpt_UI(uint8_t key);
void Temperature_UI(void);
void Temp_SetProc(uint8_t key);
void Show_Volt(void);
void Display_Temp(uint8_t x, int16_t Temp);
void Show_Notice(void);
void Show_Warning(void);

void Show_TempDown(int16_t temp, int16_t dst_temp);
void Set_TemperatureShowFlag(uint8_t flag);
int16_t TemperatureShow_Change(uint8_t flag, s16 Tmp);
uint8_t Get_Exit_pos(void);
uint8_t Get_TemperatureShowFlag(void);
void Show_Ver(u8 ver[], uint8_t flag);
void Show_OrderChar(uint8_t * ptr, uint8_t num, uint8_t width);
uint8_t Reverse_Bin8(uint8_t data);
void Show_ReverseChar(uint8_t * ptr, uint8_t num, uint8_t width, uint8_t direction);
void Show_HeatingIcon(uint32_t ht_flag, uint8_t active);
void Display_Str(uint8_t x, char* str);
void Display_Str8(uint16_t x, char* str, uint8_t mode);
void Display_Str6(uint8_t x, char* str);
void Display_Str10(uint16_t x, char * str);
void Display_Str12(uint8_t x, char* str);
void Clear_Pervious(uint16_t data);
void Print_Integer(int32_t data, uint8_t posi);
void OLed_Display(void);
void Show_Cal(uint8_t flag);
void Show_Config(void);
void SetOpt_Detailed(void);
void Set_gSet_opt(uint8_t opt);

#endif
