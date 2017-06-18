#include <stdio.h>
#include <string.h>
#include "CTRL.h"
#include "Bios.h"
#include "UI.h"
#include "HARDWARE.h"
#include "S100V0_1.h"
#include "Disk.h"
#include "MMA8652FC.h"
#include "I2C.h"
#include "Oled.h"

#define HEATINGCYCLE	30

/******************************************************************************/
extern uint8_t gSet_opt;

/******************************************************************************/

DEVICE_INFO_SYS device_info;

typedef struct {
	volatile int16_t gTemp_data;
	int16_t gPrev_temp;
	uint16_t gHt_flag;
	uint8_t gCtrl_status;
	uint8_t gIs_restartkey;
	uint8_t gPre_status;
	uint8_t TEMP_SET_Pos;
	uint8_t gHanders;
} CTRL_Context_t;

CTRL_Context_t CTRL_Context = {
	.gTemp_data = 250,
	.gPrev_temp = 250,
	.gHt_flag = 0,
	.gCtrl_status = 1,
	.gIs_restartkey	= 0,
	.gPre_status	= 1,
	.TEMP_SET_Pos	= 0
};

const DEVICE_INFO_SYS info_def = {
    "2.17",			// Ver
    2000,			// T_Standby;	// 200??C=1800  2520,?????¶?
    3000,			// T_Work;		// 350??C=3362, ?????¶?
    100,			// T_Step;
    3 * 60 * 100,	// Wait_Time;	//3*60*100   3  mintute
    6 * 60 * 100,	// Idle_Time;	//6*60*100  6 minute
    0				// handers
};

struct _pid {
	int16_t settemp;	//¶¨ÒåÉè¶¨ÎÂ¶È
	int16_t actualtemp;	//¶¨ÒåÊµ¼ÊÎÂ¶È
	int16_t err;		//¶¨ÒåÎÂ¶È²îÖµ
	int16_t err_last;	//¶¨ÒåÉÏÒ»¸öÎÂ¶È²îÖµ
	int32_t ht_time;	//¶¨Òå¼ÓÈÈÊ±¼ä
	uint16_t kp,ki,kd;	//¶¨Òå±ÈÀý¡¢»ý·Ö¡¢Î¢·ÖÏµÊý
	int32_t integral;	//¶¨Òå»ý·ÖÖµ
} pid;

/*******************************************************************************
@@name  Get_Ctrl_Status
@@brief »ñÈ¡µ±Ç°×´Ì¬
@@param ÎÞ
@@return µ±Ç°×´Ì¬
*******************************************************************************/
u8 Get_CtrlStatus(void)
{
	return CTRL_Context.gCtrl_status;
}

uint8_t Get_TEMP_SET_Pos(void)
{
	return CTRL_Context.TEMP_SET_Pos;
}

uint8_t Get_Handers(void)
{
	return CTRL_Context.gHanders;
}

void Set_Handers(uint8_t handers)
{
	CTRL_Context.gHanders = handers;
}

/*******************************************************************************

*******************************************************************************/
void Set_CtrlStatus(u8 status)
{
	CTRL_Context.gCtrl_status = status;
}
/*******************************************************************************

*******************************************************************************/
void Set_PrevTemp(s16 temp)
{
	CTRL_Context.gPrev_temp = temp;
}

/*******************************************************************************

*******************************************************************************/
u16 Get_HtFlag(void)
{
	return CTRL_Context.gHt_flag;
}

/*******************************************************************************

*******************************************************************************/
s16 Get_TempVal(void)
{
	return CTRL_Context.gTemp_data;
}

/*******************************************************************************

*******************************************************************************/
void System_Init(void)
{
	memcpy((void *)&device_info, (void*)&info_def, sizeof(device_info));
}

/*******************************************************************************

*******************************************************************************/
void Pid_Init(void)
{
	pid.settemp		= 0;
	pid.actualtemp	= 0;
	pid.err			= 0;
	pid.err_last	= 0;
	pid.integral	= 0;
	pid.ht_time		= 0;
	pid.kp			= 15;
	pid.ki			= 2;
	pid.kd			= 1;
}

/*******************************************************************************
@@name  Pid_Realize
@@brief  PID×ÔÕû¶¨¼ÓÈÈÊ±¼ä
@@param tempµ±Ç°ÎÂ¶È
@@return ·µ»ØÊý¾Ýµ¥Î»/50us
*******************************************************************************/
uint16_t Pid_Realize(int16_t temp)
{
	u8 index = 0, index1 = 1;
	int16_t d_err = 0;

	pid.actualtemp	= temp;
	pid.err			= pid.settemp - pid.actualtemp;	//ÎÂ²î

	if (pid.err >= 500)  index = 0;
	else
	{
		index = 1;
		pid.integral += pid.err;//»ý·ÖÏî
	}

////////////////////////////////////////////////////////////////////////////////
//½µÎÂÈ¥»ý·Ö
	if (pid.settemp < pid.actualtemp)
	{
		d_err = pid.actualtemp - pid.settemp;
		if (d_err > 20)
		{
			pid.integral = 0; //¹ý³å5¡æ
			index1 = 0;
			index = 0;
		}
	}
////////////////////////////////////////////////////////////////////////////////
	index1 = (pid.err <= 30) ? 0 : 1;

	pid.ht_time	 = pid.kp * pid.err + pid.ki * index * pid.integral + pid.kd * (pid.err - pid.err_last) * index1;
	pid.err_last	= pid.err;

	if (pid.ht_time <= 0)				pid.ht_time = 0;
	else if (pid.ht_time > 30 * 200)	pid.ht_time = 30 * 200;

	return pid.ht_time;
}

/*******************************************************************************
@@name  Heating_Time
@@brief  ¼ÆËã¼ÓÈÈ±êÖ¾£¬·µ»Ø¼ÓÈÈÊ±¼ä
@@param tempµ±Ç°ÎÂ¶È£¬wk_temp ¹¤×÷ÎÂ¶È
@@return ·µ»ØÊý¾Ýµ¥Î»/50us
*******************************************************************************/
uint32_t Heating_Time(int16_t temp, int16_t wk_temp)
{
	uint32_t heat_timecnt;

	pid.settemp = wk_temp;
	if (wk_temp > temp)
		CTRL_Context.gHt_flag = (wk_temp - temp >= 18) ? 0 : 2;
	else
		CTRL_Context.gHt_flag = (temp - wk_temp <= 18) ? 2 : 1;

	heat_timecnt = Pid_Realize(temp);
	return heat_timecnt;
}

/*******************************************************************************
@@name		Status_Tran
@@brief		According to the key, temperature control and other control state conversion
@@param		NULL
@@return	NULL
*******************************************************************************/
void Status_Tran(void) // State transition
{
	// Initial Standby Time Flag: 0 => Uninitialized, 1 => Initialized
	static int16_t init_waitingtime = 0;
	static uint8_t temo_set_pos = 1;

	int16_t heat_timecnt = 0, wk_temp;
	
	switch (Get_CtrlStatus())	// Get the current status
	{	
	case IDLE:// standby mode
		switch(Get_gKey())
		{
		case KEY_V1:	// Press the A key to enter the temperature control state
			if (CTRL_Context.gIs_restartkey != 1 && Read_Vb(1) < 4)
			{	// Restart flag bit gIs_restartkey not 1
				// To determine whether the voltage is normal
				Set_CtrlStatus(TEMP_CTR);	// Set the current state to temperature control
				init_waitingtime	= 0;	// Initialize wait count
				TEMPSHOW_TIMER  = 0;		// Initialize the timer
				UI_TIMER = 0;				// Image delay = 0
				G6_TIMER = 0;
			}
			break;
		case KEY_V2:	// Press the B button
			if (CTRL_Context.gIs_restartkey != 1)
			{
//				Set_CtrlStatus(THERMOMETER);//½øÈëÎÂ¶È¼ÆÄ£Ê½
//				UI_TIMER = 0;
//				Set_LongKeyFlag(1);
				CTRL_Context.TEMP_SET_Pos = 1;	// Change the parameters and save them
				Display_Str8(0,"                ",0);
				SetOpt_UI(0);
				EFFECTIVE_KEY_TIMER = 1000;	// Exit Setup Mode Time Initialization
				Clear_Screen();
				Set_gSet_opt(HD);
				Set_CtrlStatus(TEMP_SET);	// Go to setup mode
				KD_TIMER = 50;
			}
			break;
		case KEY_CN | KEY_V3:	// Press AB two keys at the same time
			break;
		}

		if (CTRL_Context.gIs_restartkey && (KD_TIMER == 0))
		{
			CTRL_Context.gIs_restartkey = 0;// Initialization
			Set_gKey(NO_KEY);
		}

		if (Read_Vb(1) == 0)
		{	// The voltage goes into alarm mode
			if (Get_UpdataFlag() == 1) Set_UpdataFlag(0);
			Set_CtrlStatus(ALARM);	// ALARM: Alarm mode
		}

		if (CTRL_Context.gPre_status != WAIT && CTRL_Context.gPre_status != IDLE)
		{	// Screen protection is automatically black
			G6_TIMER = device_info.idle_time;
			Set_gKey(NO_KEY);
			CTRL_Context.gPre_status = IDLE;
		}
		break;
	case TEMP_CTR:	// Temperature control state, the main state of the work of soldering iron
		if (temo_set_pos)
		{	// From the set state to the temperature control state
			temo_set_pos = 0;
			Delay_uS(10000000);
			Set_LongKeyFlag(1);
			Set_gKey(NO_KEY);
		}
		switch(Get_gKey())
		{
		case KEY_CN | KEY_V1:
		case KEY_CN | KEY_V2:		// Long press any key
			Clear_HeatingTime();	// The heating time is set to 0 to stop heating
			Clear_Screen();
			CTRL_Context.TEMP_SET_Pos = 0;	// The setting mode only changes the temporary temperature
			Set_CtrlStatus(TEMP_SET);	// Go to setup mode
			Set_gKey(NO_KEY);			// Clear the key
			HEATING_TIMER		= 0;	// Internal heating
			EFFECTIVE_KEY_TIMER	= 1000;	// Exit Setup Mode Time Initialization
			KD_TIMER = 150;				// Key delay
			break;
		case KEY_CN|KEY_V3:		// Press AB two keys at the same time
			Clear_HeatingTime();
			Set_LongKeyFlag(0);		// Set the long press flag = 0
			Set_CtrlStatus(IDLE);	// Return to standby
			CTRL_Context.gPre_status = IDLE;
			CTRL_Context.gIs_restartkey = 1;		// Restart flag
			KD_TIMER = 50;
			break;
		}

		if (Read_Vb(1) >= 4)
		{
			Clear_HeatingTime();	// Set the heating time
			Set_LongKeyFlag(0);
			Set_CtrlStatus(IDLE);	// Current status setting
			CTRL_Context.gPre_status = TEMP_CTR;
			CTRL_Context.gIs_restartkey = 1;		// Soft restart flag 1
			KD_TIMER = 50;			// 2 seconds
		}

		wk_temp = (Get_TemperatureShowFlag())
					? TemperatureShow_Change(1, device_info.t_work)
					: device_info.t_work;

		if (HEATING_TIMER == 0)
		{	// Timing is over
			CTRL_Context.gTemp_data	= Get_Temp(wk_temp);
			heat_timecnt  = Heating_Time(CTRL_Context.gTemp_data,wk_temp);  // Calculate the heating time
			Set_HeatingTime(heat_timecnt);
			HEATING_TIMER = HEATINGCYCLE;
		}
		if (Get_HeatingTime() == 0)
			HEATING_TIMER = 0;

		if (Get_MmaShift() == 0)
		{	// MMA_active = 0 ==> static, MMA_active = 1 ==> move
			if (init_waitingtime == 0)
			{
				init_waitingtime	= 1;
				ENTER_WAIT_TIMER = device_info.wait_time;
			}
			if (init_waitingtime != 0 && ENTER_WAIT_TIMER == 0)
			{
				CTRL_Context.gHt_flag	 = 0;
				UI_TIMER	 = 0;
				Clear_HeatingTime();
				Set_gKey(0);
				G6_TIMER = device_info.idle_time;
				Set_CtrlStatus(WAIT);
			}
		} else
			init_waitingtime = 0;

		if (Get_AlarmType() > NORMAL_TEMP)
		{	// caveat
			if (Get_UpdataFlag() == 1) Set_UpdataFlag(0);
			Set_CtrlStatus(ALARM);
		}
		break;
	case WAIT:// Sleep state
		wk_temp = (Get_TemperatureShowFlag())
					? TemperatureShow_Change(1, device_info.t_standby)
					: device_info.t_standby;

		if (device_info.t_standby > device_info.t_work)
		{	// Sleep temperature is higher than working temperature
			wk_temp = (Get_TemperatureShowFlag())
						? TemperatureShow_Change(1,device_info.t_work)
						: device_info.t_work;// No longer keep the temperature low
		}
		if (HEATING_TIMER == 0)
		{
			CTRL_Context.gTemp_data	= Get_Temp(wk_temp);
			heat_timecnt  = Heating_Time(CTRL_Context.gTemp_data, wk_temp);  // Calculate the heating time
			Set_HeatingTime(heat_timecnt);
			HEATING_TIMER = 30;
		}

		if (Read_Vb(1) >= 4)
		{
			Clear_HeatingTime();
			Set_LongKeyFlag(0);
			Set_CtrlStatus(IDLE);
			G6_TIMER = device_info.idle_time;
			CTRL_Context.gPre_status = WAIT;
			CTRL_Context.gIs_restartkey = 1;
			KD_TIMER = 50; // 2 seconds
		}

		if (G6_TIMER == 0)
		{	// Go to standby
			Clear_HeatingTime();
			Set_LongKeyFlag(0);
			CTRL_Context.gIs_restartkey = 1;
			KD_TIMER = 200; //  4 seconds
			CTRL_Context.gPre_status = WAIT;
			Set_CtrlStatus(IDLE);
		}

		if (Get_MmaShift() == 1 || Get_gKey() != 0)
		{
			UI_TIMER	  = 0;
			G6_TIMER	  = 0;
			init_waitingtime = 0;
			Set_CtrlStatus(TEMP_CTR);
		}

		if (Get_AlarmType() > NORMAL_TEMP)
		{	// caveat
			if (Get_UpdataFlag() == 1) Set_UpdataFlag(0);
			Set_CtrlStatus(ALARM);
		}
		break;
	case TEMP_SET:	// Temperature setting state
		if (EFFECTIVE_KEY_TIMER == 0)
		{	// Time is exhausted
			Set_gCalib_flag(1);
			gTime[5]=1;
			if (CTRL_Context.TEMP_SET_Pos == 1)
			{	// Save the modified parameters
				Disk_BuffInit();	// Disk data is initialized
				Config_Analysis();	// Start virtual u disk
			}
			Set_gCalib_flag(0);
			temo_set_pos=1;
			if (CTRL_Context.TEMP_SET_Pos == 0)
			{
				Set_LongKeyFlag(0);	// Set the long press flag = 0
				Set_gKey(NO_KEY);
				Delay_uS(3000000);
				Set_CtrlStatus(TEMP_CTR);	// Return to temperature control
				TEMPSHOW_TIMER = 0;			// Timer initialization
			}
			else 
			{
				Clear_HeatingTime();
				Set_LongKeyFlag(0);		// Set the long press flag = 0
				Set_CtrlStatus(IDLE);	// Return to standby
				CTRL_Context.gPre_status = TEMP_CTR;
				CTRL_Context.gIs_restartkey = 1;		// Restart flag
				KD_TIMER = 100; 
			}
		}
		if (Get_gKey() == (KEY_CN | KEY_V3) && CTRL_Context.TEMP_SET_Pos == 0)
		{	// While long press AB key to exit the temperature setting state
	//		gCalib_flag = 1;
	//		Disk_BuffInit();
	//		Config_Analysis();	// Start virtual u disk
	//		gCalib_flag = 0;
	//		Set_CtrlStatus(VOLT);
			EFFECTIVE_KEY_TIMER = 0;
			Set_gKey(NO_KEY);
			Set_LongKeyFlag(0);	// Set the long press flag = 0
		}
		break;
	case VOLT:
		if (KD_TIMER > 0)
		{
			Set_gKey(NO_KEY);
			break;
		}
		if (Get_gKey() != NO_KEY)
		{
			EFFECTIVE_KEY_TIMER = 1000;// Button to refresh the time
			Set_CtrlStatus(TEMP_CTR);
		}
		break;
#if 0
	case THERMOMETER://ÎÂ¶È¼ÆÄ£Ê½
		if (KD_TIMER > 0)
		{
			Set_gKey(NO_KEY);
			break;
		}
		switch(Get_gKey())
		{
		case KEY_CN | KEY_V1:
		case KEY_CN | KEY_V2:
			back_prestatus = 1;
			break;
		case KEY_CN | KEY_V3:
			Zero_Calibration();		//Ð£×¼Áãµã
			if (Get_CalFlag() == 1)
			{
				Disk_BuffInit();
				Config_Analysis();	// Æô¶¯ÐéÄâUÅÌ
			}
			KD_TIMER = 200;			//20150717 ÐÞ¸Ä
			break;
		default:
			break;
		}
		if (back_prestatus == 1)
		{
			back_prestatus = 0;
			Clear_HeatingTime();
			Set_CtrlStatus(IDLE);
			gPre_status = THERMOMETER;
			gIs_restartkey = 1;
			Set_LongKeyFlag(0);
			KD_TIMER = 50;
		}
		break;
#endif
	case ALARM:	// Warning mode
		switch(Get_AlarmType())
		{
		case HIGH_TEMP:
		case SEN_ERR:
			wk_temp = (Get_TemperatureShowFlag())
						? TemperatureShow_Change(1,device_info.t_standby)
						: device_info.t_standby;
			CTRL_Context.gTemp_data  = Get_Temp(wk_temp);
			if (Get_AlarmType() == NORMAL_TEMP)
			{
				Set_CtrlStatus(TEMP_CTR);
				Set_UpdataFlag(0);
			}
			break;
		case HIGH_VOLTAGE:
		case LOW_VOLTAGE:
			if (Read_Vb(1) >= 1 && Read_Vb(1) <= 3)
			{
				Clear_HeatingTime();
				Set_LongKeyFlag(0);
				CTRL_Context.gIs_restartkey = 1;
				UI_TIMER = 2;
				CTRL_Context.gPre_status = THERMOMETER;
				Set_CtrlStatus(IDLE);
			}
			break;
		}
		if (Get_HeatingTime() != 0)
		{	// Immediately stop heating
			Clear_HeatingTime();
			HEAT_OFF();
		}
		break;
	default:
		break;
	}
}
