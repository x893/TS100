#include <stdio.h>
#include <string.h>
#include "APP_Version.h"
#include "HARDWARE.h"
#include "CTRL.h"
#include "bios.h"
#include "UI.h"

/******************************************************************************/

#define CAL_AD		250

const uint32_t gVol[]  = { 3900, 2760, 1720, 584 };
const uint16_t gRate[] = { 300, 150, 90, 40 };

Hard_Context_t Hard_Context = {
	.gZerop_ad = 239,
	.gTurn_offv = 50,
	.gKey_in = 0,
	.gAlarm_type	= 1,
	.gCalib_flag	= 0,
	.gLongkey_flag	= 0
};

uint32_t Get_gTurn_offv(void)
{
	return Hard_Context.gTurn_offv;
}

void Set_gTurn_offv(uint32_t value)
{
	Hard_Context.gTurn_offv = value;
}

void Set_gZerop_ad(int32_t value)
{
	Hard_Context.gZerop_ad = value;
}
int32_t Get_gZerop_ad(void)
{
	return Hard_Context.gZerop_ad;
}

uint8_t Get_gCalib_flag(void)
{
	return Hard_Context.gCalib_flag;
}
void Set_gCalib_flag(uint8_t flag)
{
	Hard_Context.gCalib_flag = flag;
}

/*******************************************************************************
@@name  Get_CalFlag
@@brief Read the calibration status
@@param NULL
@@return Calibration status flag
*******************************************************************************/
uint32_t Get_CalFlag(void)
{
	return Hard_Context.gCalib_flag;
}

/*******************************************************************************
@@name  Get_gKey
@@brief Get the key state
@@param NULL
@@return Get the key state
*******************************************************************************/
uint16_t Get_gKey(void)
{
	return Hard_Context.gKey_in;
}
/*******************************************************************************
@@name  Set_gKey
@@brief Set the key state
@@param The key state to set
@@return NULL
*******************************************************************************/
void Set_gKey(uint16_t key)
{
	Hard_Context.gKey_in = key;
}
/*******************************************************************************
@@name  Set_LongKeyFlag
@@brief Set the long button
@@param 0: can not press long key 1: can press long press
@@return NULL
*******************************************************************************/
void Set_LongKeyFlag(uint32_t flag)
{
	Hard_Context.gLongkey_flag = flag;
}
/*******************************************************************************
@@name  Get_AlarmType
@@brief Get the alarm type
@@param NULL
@@return  Warning type
			0:normal
			1:sen - err
			2:Overtemperature
			3:Overpressure
*******************************************************************************/
u8 Get_AlarmType(void)
{
	return Hard_Context.gAlarm_type;
}
/*******************************************************************************
@@name  Set_AlarmType
@@brief Set the alarm type
@@param  Warning type
			0:normal
			1:sen - err
			2:Overtemperature
			3:Overpressure
@@return NULL
*******************************************************************************/
void Set_AlarmType(uint8_t type)
{
	Hard_Context.gAlarm_type = type;
}
/*******************************************************************************
@@name  Read_Vb
@@brief Read the supply voltage value
@@param Logo
@@return NULL
*******************************************************************************/
int Read_Vb(uint8_t flag)
{
	uint32_t tmp, i, sum = 0;

	for (i = 0; i < 10; i++)
	{
		tmp = ADC_GetConversionValue(ADC2);
		sum += tmp;
	}
	tmp = sum / 10;
	if (tmp >= (gVol[0] + gVol[0] / 100))
	{
		Hard_Context.gAlarm_type = HIGH_VOLTAGE;
		return H_ALARM;	// Greater than 3500
	}
	tmp = (tmp * 10 / 144);// Voltage vb = 3.3 * 85 * ad / 40950

	for (i = 0; i < 4; i++)
	{
		if (i == 2)
		{
			if (flag == 0 )
			{
				if (tmp >= gRate[i])
					break;
			}
			else if (tmp >= Hard_Context.gTurn_offv)
				break;
		}
		else if (tmp >= gRate[i])
			break;
	}
	return (i+1);
}
/*******************************************************************************
@@name  Read_Key
@@brief
@@param NULL
@@return NULL
*******************************************************************************/
void Key_Read( void )
{
	static uint16_t buttonA_cnt = 0, buttonB_cnt = 0;
	static uint16_t kgap = 0; // Key gap twice
	static uint16_t press_buttonA = 0, press_buttonB = 0;

	press_buttonA = KEYA_READ;
	press_buttonB = KEYB_READ;

	if (Get_Exit_pos())
	{
		buttonA_cnt = 0;
		buttonB_cnt = 0;
	}
	if (press_buttonA == KEY1_PIN)
	{
		kgap = 0;
		buttonA_cnt++;
	}
	if (press_buttonB == KEY2_PIN)
	{
		kgap = 0;
		buttonB_cnt++;
	}

	if (Hard_Context.gLongkey_flag && buttonA_cnt >= SI_DATA && buttonB_cnt >= SI_DATA)
		Set_gKey(KEY_CN | KEY_V3);
	else
	{
		if (buttonA_cnt >= SI_DATA && Hard_Context.gLongkey_flag && buttonB_cnt == 0) 
		{
			if (device_info.handers)	Set_gKey(KEY_CN | KEY_V2);
			else						Set_gKey(KEY_CN | KEY_V1);
		}
		if (buttonB_cnt >= SI_DATA && Hard_Context.gLongkey_flag && buttonA_cnt == 0)
		{
			if (device_info.handers)	Set_gKey(KEY_CN | KEY_V1);
			else						Set_gKey(KEY_CN | KEY_V2);
		}
	}

	if (press_buttonA == 0)
	{
		if (buttonA_cnt < SI_DATA && buttonA_cnt != 0 && buttonB_cnt < SI_DATA && buttonB_cnt != 0)
		{
			Set_gKey(KEY_V3); 
			buttonB_cnt = 0;
		}
		else if (buttonA_cnt < SI_DATA && buttonA_cnt != 0)
		{
			if (device_info.handers)	Set_gKey(KEY_V2);
			else						Set_gKey(KEY_V1);
		}
		kgap++;
		buttonA_cnt = 0;
	}

	if (press_buttonB == 0)
	{
		if (buttonA_cnt < SI_DATA && buttonA_cnt != 0 && buttonB_cnt < SI_DATA && buttonB_cnt != 0)
		{
			Set_gKey(KEY_V3);
			buttonA_cnt = 0; 
		}
		else if (buttonB_cnt < SI_DATA && buttonB_cnt != 0)
		{
			if (device_info.handers)	Set_gKey(KEY_V1);
			else						Set_gKey(KEY_V2);
		}
		kgap++;
		buttonB_cnt = 0;
	}
}

/*******************************************************************************
@@name  Get_SlAvg
@@brief 滑动平均值
@@param avg_data 平均的AD值
@@return 滑动平均值
*******************************************************************************/
uint32_t Get_SlAvg(uint32_t avg_data)
{
	static uint32_t sum_avg = 0;
	static uint8_t  init_flag = 0;
	uint16_t si_avg = sum_avg / SI_COE, abs;

	if (init_flag == 0)
	{	// First power on
		sum_avg = SI_COE * avg_data;
		init_flag = 1;
		return sum_avg/SI_COE;
	}

	if (avg_data > si_avg)	abs = avg_data - si_avg;
	else					abs = si_avg - avg_data;

	if (abs > SI_THRESHOLD)	sum_avg = SI_COE * avg_data;
	else					sum_avg += avg_data	- sum_avg / SI_COE;

	return sum_avg/SI_COE;
}

/*******************************************************************************
@@name  Get_AvgAd
@@brief 获取并计算热端AD平均值
@@param NULL
@@return AD平均值
*******************************************************************************/
uint32_t Get_AvgAd(void)
{
	uint32_t ad_sum = 0, min = 5000, max = 0;
	uint32_t ad_value,avg_data, slide_data;
	
	Clear_HeatingTime();
	HEAT_OFF();
	Delay_HalfMs(25);
	int cnt = 10;
	while (cnt != 0)
	{
		cnt--;

		ad_value = Get_TMP36();
		ad_sum += ad_value;
		if (ad_value > max)	max = ad_value;
		if (ad_value < min)	min = ad_value;
	}
	ad_sum	 = ad_sum - max - min;
	avg_data = ad_sum / 8;

	slide_data = Get_SlAvg(avg_data);
	return slide_data;
}

/*******************************************************************************
@@name  Get_TempSlAvg
@@brief 冷端温度滑动平均值
@@param avg_data 冷端温度平均值
@@return 冷端温度滑动平均值
*******************************************************************************/
int Get_TempSlAvg(int avg_data)
{
	static int sum_avg = 0;
	static u8 init_flag = 0;

	if (init_flag == 0)
	{	// First power on
		sum_avg = 8 * avg_data;
		init_flag = 1;
		return sum_avg / 8;
	}

	sum_avg += avg_data	- sum_avg / 8;
	return sum_avg / 8;
}

/*******************************************************************************
@@name  Get_SensorTmp
@@brief 获取冷端温度
@@param NULL
@@return 获取冷端温度
*******************************************************************************/
int Get_SensorTmp(void)
{
	uint32_t ad_sum = 0;
	uint32_t max = 0, min = 5000;
	uint32_t ad_value, avg_data, slide_data;
	int sensor_temp = 0;
	int cnt = 10;

	while (cnt != 0)
	{
		cnt--;
		ad_value = Get_IRON();
		ad_sum += ad_value;
		if (ad_value > max)	max = ad_value;
		if (ad_value < min)	min = ad_value;
	}
	ad_sum		= ad_sum - max - min;
	avg_data	= ad_sum / 8;

	slide_data = Get_TempSlAvg(avg_data);
	sensor_temp = (250 + (3300 * slide_data / 4096) - 750);

	return sensor_temp;
}

/*******************************************************************************
@@name  Zero_Calibration
@@brief Calibration zero
@@param NULL
@@return NULL
*******************************************************************************/
void Zero_Calibration(void)
{
	uint32_t zerop;
	int cool_tmp;

	zerop	 = Get_AvgAd();
	cool_tmp = Get_SensorTmp();

	if (zerop >= 400)
		// The average temperature is greater than or equal to 400
		Set_gCalib_flag(2); // Verification failed
	else
	{
		if (cool_tmp < 300)
		{	// The cooling end temperature is less than 300
			Set_gZerop_ad(zerop);
			Set_gCalib_flag(1);
		} else
			Set_gCalib_flag(2);// Verification failed
	}
}
/******************************************************************************* 
@@name	Get_Temp
@@brief	Calculate the temperature according to the cold end,
		the hot end temperature, and the compensation AD
@@param Wk_temp operating temperature
@@return Actual temperature
*******************************************************************************/
int16_t Get_Temp(int16_t wk_temp)
{
	int ad_value, cool_tmp, compensation = 0;
	static uint16_t cnt = 0, h_cnt = 0;
	int16_t rl_temp = 0;

	ad_value = Get_AvgAd();
	cool_tmp = Get_SensorTmp();

	if (ad_value == 4095)
		h_cnt++;
	else
	{
		h_cnt = 0;
		if (ad_value > 3800 && ad_value < 4095)	cnt++;
		else									cnt = 0;
	}
	if (h_cnt >= 60 && cnt ==  0)	Set_AlarmType(SEN_ERR);	//Sen-err
	if (h_cnt == 0  && cnt >= 10)	Set_AlarmType(HIGH_TEMP); // Overtemperature
	if (h_cnt < 60  && cnt <  10)	Set_AlarmType(NORMAL_TEMP);

	compensation = 80 + 150 * (wk_temp - 1000) / 3000;
	if (wk_temp == 1000)
		compensation -= 10;

	if (wk_temp != 0 && (ad_value > (compensation + Get_gZerop_ad())))
		ad_value -= compensation;
	if (cool_tmp > 400)
		cool_tmp = 400;
	rl_temp = (ad_value * 1000 + 806 * cool_tmp - Get_gZerop_ad() * 1000) / 806;

	return rl_temp;
}

/*******************************************************************************
@@name  Start_Watchdog
@@brief Initialize the opening dog
@@param ms Open the dog count
@@return 1
*******************************************************************************/
uint32_t Start_Watchdog(uint32_t ms)
{
	/* Enable write protection 0x5555 */
	IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);

	/* IWDG counter clock: 40KHz(LSI) / 32 = 1.25 KHz (min:0.8ms -- max:3276.8ms */
	IWDG_SetPrescaler(IWDG_Prescaler_32);

	/* Set counter reload value to XXms */
	IWDG_SetReload(ms*10/8);

	/* Reload IWDG counter */
	IWDG_ReloadCounter();

	/* Enable IWDG (the LSI oscillator will be enabled by hardware) */
	IWDG_Enable();
	return 1;
}
/*******************************************************************************
@@name  Clear_Watchdog
@@brief Reset the opening dog count
@@param NULL
@@return 1
*******************************************************************************/
uint32_t Clear_Watchdog(void)
{
	IWDG_ReloadCounter();
	return 1;
}
