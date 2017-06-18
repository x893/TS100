#ifndef _CTRL_H
#define _CTRL_H

#include "STM32F10x.h"
#include "Bios.h"

#define TEMPSHOW_TIMER			gTime[0] /* TEMPSHOW_TIMER */
#define HEATING_TIMER			gTime[1] /* HEATING_TIMER */
#define ENTER_WAIT_TIMER		gTime[2] /* ENTER_WAIT_TIMER */
#define EFFECTIVE_KEY_TIMER		gTime[3] /* EFFECTIVE_KEY_TIMER */
#define LEAVE_WAIT_TIMER		gTime[4] /* LEAVE_WAIT_TIMER */
#define G6_TIMER				gTime[5] /* SWITCH_SHOW_TIMER */
#define UI_TIMER				gTime[6] /* UI_TIMER Image interface left shift time control */
#define KD_TIMER				gTime[7] /* Key delay */

#define   NO_KEY	0x0
#define   KEY_V1	0x0100
#define   KEY_V2	0x0040
#define   KEY_V3	0x0140
#define   KEY_CN	0x8000

typedef enum WORK_STATUS {
	IDLE = 1,
	THERMOMETER,
	TEMP_CTR,
	WAIT,
	TEMP_SET,
	CONFIG ,
	MODE_CNG,
	ALARM,
	VOLT,
} WORK_STATUS;

typedef enum WARNING_STATUS {
	NORMAL_TEMP = 1,
	HIGH_TEMP,
	SEN_ERR,
	HIGH_VOLTAGE,
	LOW_VOLTAGE,
} WARNING_STATUS;

typedef enum SET_OPT {
	WKT = 0,	// t_standby
	SDT,		// t_work
	WTT,		// sleep_time
	IDT,		// idle_time
	STP,		// t_step
	TOV,		// turn of V
	DGC,		// dgc
	HD,			// handers
	WDJ,		// thermometer
	EXW			// EX WORKS
} SET_OPT;

typedef struct {
	uint16_t set_opt;		// value
	uint8_t  settings[10];	// Set options
} SYS_SET;

typedef struct {
    u8 ver[16];             // ????
    int t_standby;          // 200??C=1800  2520,?????¶?
    int t_work;             // 350??C=3362, ?????¶?
    int t_step;             // ???ò???
    u32 wait_time;          // 3*60*100   3 mintute
    u32 idle_time;          // 6*60*100   6 minute 
    u8 handers;             // 0 right 1 left
} DEVICE_INFO_SYS;

extern DEVICE_INFO_SYS device_info;

uint8_t Get_TEMP_SET_Pos(void);
void Set_Handers(uint8_t handers);
uint8_t Get_Handers(void);
void Set_PrevTemp(int16_t Temp);
int16_t Get_TempVal(void);
uint8_t Get_CtrlStatus(void);
void Set_CtrlStatus(u8 status);
uint16_t Get_HtFlag(void);
void System_Init(void);
void Pid_Init(void);
uint16_t Pid_Realize(int16_t temp);
uint32_t Heating_Time(int16_t temp, int16_t wk_temp);
void Status_Tran(void);

#endif
