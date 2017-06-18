#include <stdio.h>
#include <string.h>
#include "APP_Version.h"
#include "UI.h"
#include "CTRL.h"
#include "WordLib.h"
#include "Bios.h"
#include "OLed.h"
#include "HARDWARE.h"
#include "disk.h"
#include "MMA8652FC.h"
#include "own_sprintf.h"
#include "I2C.h"

const char * const gSys_settings[] = {
	"WkTemp ",
	"StbTemp",
	"SlpTime",
	"IdlTime",
	"TempStp",
	"OffVolt",
	"Temp   ",
	"Hand   "
};

/******************************************************************************/
uint8_t gTemp_array[16 * 16 + 16];
uint16_t gTemp_array_u16[208];
typedef struct {
	uint32_t gCont;
	uint8_t direction_flag;
	uint8_t gTemperatureshowflag;
	uint8_t digit;
	uint8_t gUp_flag, gDown_flag, gLevel_flag, gTempset_showctrl;
	uint8_t Exit_pos;
} UI_Context_t;
UI_Context_t UI_Context;

int32_t gSet_table[9][3] = {	{ 4000,  1000, 100},
								{ 4000,  1000, 100},
								{ 90000, 6000,3000},
								{ 90000,10000,3000},
								{ 250,     10,  10},
								{ 120,     50,   1},
								{ 1,        0,   1},
								{ 1,        0,   1},
								{ 7520,  2120, 100}
							};

uint8_t Ver_s[] = {  /*12*16*/
	0x04,0x0C,0x74,0x80,0x00,0x00,0x00,0x80,0x74,0x0C,0x04,0x00,
	0x00,0x00,0x00,0x03,0x1C,0x60,0x1C,0x03,0x00,0x00,0x00,0x00,/*"V",0*/
	0x00,0x00,0x80,0x40,0x40,0x40,0x40,0x80,0x00,0x00,0x00,0x00,
	0x00,0x1F,0x22,0x42,0x42,0x42,0x42,0x22,0x13,0x00,0x00,0x00,/*"e",1*/
	0x00,0x40,0x40,0xC0,0x00,0x80,0x40,0x40,0x40,0xC0,0x00,0x00,
	0x00,0x40,0x40,0x7F,0x41,0x40,0x40,0x00,0x00,0x00,0x00,0x00,/*"r",2*/
	0x00,0x00,0x00,0x00,0x60,0x60,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x60,0x60,0x00,0x00,0x00,0x00,0x00,0x00,/*":",3*/
	0x00,0x78,0x04,0x04,0x04,0x04,0x04,0x88,0x78,0x00,0x00,0x00,
	0x00,0x60,0x50,0x48,0x44,0x42,0x41,0x40,0x70,0x00,0x00,0x00,/*"2",4*/
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x60,0x60,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,/*".",5*/
	0x00,0xE0,0x18,0x08,0x04,0x04,0x04,0x08,0x18,0xE0,0x00,0x00,
	0x00,0x0F,0x30,0x20,0x40,0x40,0x40,0x20,0x30,0x0F,0x00,0x00,/*"0",6*/
	0x00,0x00,0x00,0x08,0x08,0xFC,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x40,0x40,0x7F,0x40,0x40,0x40,0x00,0x00,0x00,/*"1",7*/
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,/*" ",5*/
};

/******************************************************************************/


uint8_t Get_Exit_pos(void)
{
	return UI_Context.Exit_pos;
}

/*******************************************************************************
@@name  Get_UpdataFlag
@@brief Get the screen refresh flag
@@param NULL
@@return Screen refresh flag
*******************************************************************************/
uint32_t Get_UpdataFlag(void)
{
	return UI_Context.gCont;
}

/*******************************************************************************
@@name  Set_UpdataFlag
@@brief Set the screen refresh flag
@@param 1 refresh, 0 not refresh
@@return NULL
*******************************************************************************/
void Set_UpdataFlag(u32 cont)
{
	UI_Context.gCont = cont;
}
/*******************************************************************************
@@name  Set_TemperatureShowFlag
@@brief Set the temperature display mode
@@param Flag (0), Fahrenheit (1) sign
@@return NULL
*******************************************************************************/
void Set_TemperatureShowFlag(uint8_t flag)
{
	UI_Context.gTemperatureshowflag = flag;
}
/*******************************************************************************
@@name  Get_TemperatureShowFlag
@@brief Get the temperature display mode
@@param NULL
@@return Flag (0), Fahrenheit (1) sign
*******************************************************************************/
uint8_t Get_TemperatureShowFlag(void)
{
	return UI_Context.gTemperatureshowflag;
}
/*******************************************************************************
@@name	TemperatureShow_Change
@@brief	Degrees Fahrenheit conversion
@@param	Flag 0, degrees Celsius
		Flag 1, degrees of change in degrees Fahrenheit
		Tmp the temperature to be converted
@@return Converted value
*******************************************************************************/
s16 TemperatureShow_Change(uint8_t flag ,s16 tmp)
{
	if (flag == 0)
		return (320 + tmp * 9/5);
	else
		return (5*tmp - 1600) / 9;
}
/*******************************************************************************
@@name  APP_Init
@@brief According to the voltage initialization start state
@@param NULL
@@return NULL
*******************************************************************************/
void APP_Init(void)
{
	int rev;

	HEATING_TIMER = 0;
	UI_TIMER = 0;

	rev = Read_Vb(0);
	if (rev == 0)
		Set_CtrlStatus(ALARM);
	else if (rev >= 4)
	{
		Set_LongKeyFlag(1);
		Set_CtrlStatus(CONFIG);
	}
	else
	{
		Set_CtrlStatus(IDLE);
		G6_TIMER = device_info.idle_time;
	}

}

void Show_Volt(void)
{	   
	u32 tmp,i,sum = 0;
	uint8_t buf[20];
	
	for(i = 0; i < 10; i++)
	{
		tmp = ADC_GetConversionValue(ADC2);
		sum += tmp;
	}
	tmp = sum / 10;
	tmp = (tmp * 100 / 144);// Voltage vb = 3.3 * 85 * ad / 40950 expansion 100 times

	own_sprintf((char *)buf,"%fV",tmp);

	Display_Str8(1, (char *)buf, 0);
}

/*******************************************************************************
@@name  Display_Temp
@@brief 显示温度
@@param x:显示位置 Temp:温度
@@return NULL
*******************************************************************************/
void Display_Temp(uint8_t x,s16 temp)
{
	char Str[8];

	memset(Str,0x0,6);
	own_sprintf(Str,"%d",temp);

	if (UI_Context.gTemperatureshowflag == 0)
	{
		if (temp > 99)
			Str[3] = 'C';
		else if (temp > 9 )
		{
			Str[2] = 'C';
			Str[3] = ' ';
		}
		else if (temp < 0)
		{
			if (temp + 9 >= 0)
			{
				Str[2] = 'C';
				Str[3] = ' ';
			}
			else
				Str[3] = 'C';
		}
		else
		{
			Str[1] = 'C';
			Str[2] = ' ';
			Str[3] = ' ';
		}
		Str[4] = ' ';
	}
	else
	{
		if (temp > 99) Str[3] = 'E';
		else if (temp > 9 )
		{
			Str[2] = 'E';
			Str[3] = ' ';
		}
		else if (temp < 0)
		{
			if (temp + 9 >= 0)
			{
				Str[2] = 'E';
				Str[3] = ' ';
			}
			else
				Str[3] = 'E';
		}
		else
		{
			Str[1] = 'E';
			Str[2] = ' ';
			Str[3] = ' ';
		}
		Str[4] = ' ';
	}
	Display_Str(x,Str);
	TEMPSHOW_TIMER = 20; // 200ms
}

/*******************************************************************************
@@name  Show_Notice
@@brief Display the standby message
*******************************************************************************/
void Show_Notice(void)
{
	int j, k;
	static uint8_t * ptr0;
	static uint8_t posi = 0, i = 0;

	if (i == 0)			// Step 1
		ptr0 = Oled_DrawArea(0, 0, 96, 16,(uint8_t *)Maplib);
	else if (i == 1)	// Step 2
		ptr0 = Oled_DrawArea(0, 0, 96, 16, ptr0);
	else if (i == 2)	// Step 3
		ptr0 = Oled_DrawArea(0, 0, 96, 16,(uint8_t *)Maplib);
	else if (i == 3)
	{	// Step 4
		for (j = 0 ; j < 6; j++)
		{
			k = 84;
			while (k >= posi)
			{
				ptr0 = (uint8_t *)Maplib1 + j * 28;
				Clean_Char(k + 7, 14);
				ptr0 = Oled_DrawArea(k, 0, 14, 16, ptr0);
				k -= 7;
				Delay_Ms(10);
			}
			posi += 14;
		}
		posi = 0;
	}
	i++;
	if (i == 4)
		i = 0;
}

/*******************************************************************************
@@name  Show_Ver
@@brief 显示版本
@@param ver 版本号flag (0 :滚动显示 )(1不滚动)
@@return NULL
*******************************************************************************/
void Show_Ver(uint8_t ver[], uint8_t flag)
{
	uint8_t *ptr;
	int k,i;
	uint8_t temp0, temp1, temp2;

	if (ver[2] >= '0' && ver[2] < ('9' + 1))  temp1 = ver[2] - '0';
	if (ver[3] >= '0' && ver[3] < ('9' + 1))  temp2 = ver[3] - '0';
	if (ver[0] >= '0' && ver[0] < ('9' + 1))  temp0 = ver[0] - '0';

	for (i = 0; i < 24; i++)
	{
		Ver_s[4 * 24 + i] = Number12[temp0 * 24 + i];
		Ver_s[6 * 24 + i] = Number12[temp1 * 24 + i];
		Ver_s[7 * 24 + i] = Number12[temp2 * 24 + i];
	}

	for (k = 0; k < 16; k++)
	{
		if (flag == 0)
		{
			Show_ReverseChar((uint8_t *)Ver_s, 8, 12, 2);
			ptr = (uint8_t *)gTemp_array;
		} else
			ptr = (uint8_t *)Ver_s;

		for (i = 0; i < 8; i++)
			ptr = Oled_DrawArea(i * 12, 0, 12, 16, ptr);
	}

	if (flag == 0)
	{
		Delay_Ms(1000);
		Clear_Screen();
		Clear_Watchdog();
	}
}


/*******************************************************************************
@@name  Show_Config
@@brief 显示CONFIG
@@param NULL
@@return NULL
*******************************************************************************/
void Show_Config(void)
{
	uint8_t * ptr;
	uint8_t j;

	ptr = (uint8_t *)Config;
	for (j = 0; j < 6; j++)
		ptr = Oled_DrawArea(j * 16, 0, 16, 16, ptr);
}

/*******************************************************************************
@@name  Show_TempDown
@@brief Display temperature drop: actual temperature >>> target temperature
@@param Temp actual temperature, dst_temp target temperature
@@return NULL
*******************************************************************************/
static void FillDegrees(int16_t temp, char * dst, uint16_t pos)
{
	char cf = (UI_Context.gTemperatureshowflag == 0) ? 'C' : 'E';

	memset(dst, 0, 6);
	own_sprintf(dst,"%d",temp);

	if (temp > 99)
		dst[3] = cf;
	else
	{
		dst[2] = cf;
		dst[3] = ' ';
	}
	Display_Str10(pos, dst);
}

void Show_TempDown(int16_t temp, int16_t dst_temp)
{
	static uint8_t guide_ui = 0;
	char str[8];

	FillDegrees(temp, str, 1);
	Display_Str10(1, str);

	Oled_DrawArea(41, 0, 16, 16, (uint8_t *)(Guide + (guide_ui++) * 32));
	if (guide_ui == 3)
		guide_ui = 0;

	FillDegrees(dst_temp, str, 56);
}
/*******************************************************************************
@@name  Show_Cal
@@brief 显示校准结果
@@param flag = 1 校准成功 flag = 2 校准失败
@@return NULL
*******************************************************************************/
void Show_Cal(uint8_t flag)
{
	uint8_t i;
	uint8_t* ptr;

	if (flag == 1)		ptr = (uint8_t*)Cal_Done;
	else if (flag == 2)	ptr = (uint8_t*)Cal_UnDone;

	ptr = Oled_DrawArea(0, 0, 16, 16, (uint8_t *)ptr);

	for (i = 1; i < 6; i++)
		ptr = Oled_DrawArea(16 * i, 0, 16, 16, (uint8_t*)ptr);
	Delay_Ms(1000);
	Clear_Watchdog();
}
/*******************************************************************************
@@name  Show_Warning
@@brief 显示警告界面
@@param NULL
@@return NULL
*******************************************************************************/
void Show_Warning(void)
{
	uint8_t i;
	uint8_t* ptr;
	static uint8_t flag = 0;

	switch(Get_AlarmType())
	{
	case HIGH_TEMP:
		ptr = (uint8_t*)Warning;
		break;
	case SEN_ERR:
		ptr = (uint8_t*)SenErr;
		break;
	case HIGH_VOLTAGE:
		ptr = (uint8_t*)HighVt;
		break;
	case LOW_VOLTAGE:
		ptr = (uint8_t*)LowVot;
		break;
	}

	Oled_DrawArea(0,0,16,16,(uint8_t*)Warning + 20 * 7);
	if (flag == 0) {
		for(i = 2; i < 9; i++)  Oled_DrawArea(10 * i,0,10,16,(uint8_t*)ptr +  (i - 2) * 20);
		flag = 1;
	} else {
		Clean_Char(16, 80);
		flag = 0;
	}
}

/*******************************************************************************
@@name  Show_OrderChar
@@brief 横向动态显示字符
@@param  ptr:字节库num:个数width:宽度
@@return NULL
*******************************************************************************/
void Show_OrderChar(uint8_t* ptr, uint8_t num, uint8_t width)
{
	static uint8_t i = 1, j = 0, k = 0, m = 10;
	// i是源数组开始位
	// j是目标数组开始位
	// m是整个的长度

	if (UI_Context.gLevel_flag == 0)
	{	// 前一状态不是恒温
		i = 1;
		j = 0;
		m = 8;
		UI_Context.gUp_flag	 = 0;
		UI_Context.gDown_flag   = 0;
		UI_Context.gLevel_flag  = 1;
	}

	if (i == 0)
	{
		for (k = 0; k <= m ; k++ )
		{
			gTemp_array[k] = *(ptr + 52 + j + k - 1);
			gTemp_array[k + width] = *(ptr + 52 + width + k + j - 1);
		}
		m++;
		j--;
		if (m == 11)
		{
			m--;
			j = 0;
			i = 1;
			return;
		}
	}

	if (j == 0)
	{
		for (k = 1; k < m ; k++ )
		{
			gTemp_array[k + i] = *(ptr + 52 + j + k - 1);
			gTemp_array[k + i + width] = *(ptr + 52 + width + k + j - 1);
		}
		m--;
		i++;
		if (m == 0)
		{
			m++;
			j = 9;
			i = 0;
		}
	}
}

/*******************************************************************************
@@name  Reverse_Bin8
@@brief 按位逆向8位二进制  10101010 变为01010101
@@param 逆向的数据
@@return 逆向后的数据
*******************************************************************************/
uint8_t Reverse_Bin8(uint8_t data)
{
	uint8_t i, cache, result = 0;

	for (i = 0; i < 8; i++)
	{
		cache = data & (1 << i);
		if (i < 4)
			cache <<= 7 - 2 * i;
		else
			cache >>= 2 * i - 7;

		result += cache;
	}
	return result;
}
/*******************************************************************************
@@name  Show_ReverseChar
@@brief 竖向动态显示字符
@@param  ptr:字节库   num:个数
			  width:宽度   direction :方向 (0 up, 1 down)
@@return NULL
*******************************************************************************/
void Show_ReverseChar(uint8_t * ptr, uint8_t num, uint8_t width, uint8_t direction)
{
	static u32 j = 0, m = 0, po_j[3] = {0, 0, 0}, po_m[3] = {0, 0, 16};
	uint32_t i, k;

	if (direction == 0)
	{	// up
		if (UI_Context.gUp_flag == 0)
		{	//前一状态不是加热
			j = 0;
			m = 0;
			UI_Context.gUp_flag	 	= 1;
			UI_Context.gDown_flag   = 0;
			UI_Context.gLevel_flag  = 0;
		}
		else
		{
			j = po_j[0];
			m = po_m[0];
		}
	}
	else if (direction == 1)
	{
		if (UI_Context.gDown_flag == 0)
		{	//前一状态不是降温
			j = 0;
			m = 0;
			UI_Context.gUp_flag		= 0;
			UI_Context.gDown_flag   = 1;
			UI_Context.gLevel_flag  = 0;
		}
		else
		{
			j = po_j[1];
			m = po_m[1];
		}
	}
	else
	{
		j = po_j[2];
		m = po_m[2];
	}
	for (i = 0; i < width * 2 * num; i++)
		gTemp_array[i] = Reverse_Bin8(*(ptr + i)); //逆向8位

	for (k = 0; k < width * 2 * num; k += width * 2)
		for(i = 0; i < width ; i++)
		{
			gTemp_array_u16[i + k] = ((gTemp_array[i + k] & 0x00FF) << 8) | gTemp_array[i + k + width] ;//上半部下半部与成u16 便于移位
			if (direction == 1)
			{
				if (j == 0)	gTemp_array_u16[i + k] <<= m;//下面空，上面显示
				else		gTemp_array_u16[i + k] >>= j;//上面空，下面显示
			}
			else
			{ //上
				if (m == 0)	gTemp_array_u16[i + k] <<= j;//下面空，上面显示
				else		gTemp_array_u16[i + k] >>= m;//上面空，下面显示
			}
			gTemp_array[i + k] = (gTemp_array_u16[i + k] & 0xFF00) >> 8;
			gTemp_array[i + k + width] = gTemp_array_u16[i + k] & 0x00FF;
		}

	for (i = 0; i < width * 2 * num; i++)
		gTemp_array[i] = Reverse_Bin8(gTemp_array[i]); //移位后再逆向

	if (m == 0 && j == 16)
	{	//全显示，换显示'头数'
		j = 0;
		m = 16;
	}
	if (m == 0)	j++;
	else		m--;

	if (direction == 0)
	{	//up
		po_j[0] = j;
		po_m[0] = m;
	}
	else if (direction == 1)
	{
		po_j[1] = j;
		po_m[1] = m;
	}
	else
	{
		po_j[2] = j;
		po_m[2] = m;
	}
}

/*******************************************************************************
@@name  Show_HeatingIcon
@@brief 动态选择加热等状态标识
@@param  ht_flag 恒温表示  active 移动标识
@@return NULL
*******************************************************************************/
void Show_HeatingIcon(uint32_t ht_flag, uint8_t active)
{
	uint8_t * ptr;

	memset(gTemp_array,0,20);
	if (ht_flag == 0)		Show_ReverseChar((uint8_t *)TempIcon, 1, 10, 0);		// 加热
	else if (ht_flag == 1)	Show_ReverseChar((uint8_t *)TempIcon + 32, 1, 10, 1);// 降温
	else if (ht_flag == 2)	Show_OrderChar((uint8_t *)TempIcon, 1, 10);			// 恒温

	ptr = (uint8_t*)gTemp_array;
	Oled_DrawArea(86, 0, 10, 16, (uint8_t*)ptr);

	if (Get_CtrlStatus() == WAIT)
		Oled_DrawArea(80, 0, 6, 16, (uint8_t*)IhIcon + 12);
	else
	{
		if (active == 0)	Oled_DrawArea(80, 0, 6, 16, (uint8_t *)TempIcon + 20);
		else				Oled_DrawArea(80, 0, 6, 16, (uint8_t *)IhIcon);
	}
}
/*******************************************************************************
@@name  Display_Str
@@brief 显示16*16字符串
@@param x: 位置 str :显示字符串
@@return NULL
*******************************************************************************/
void Display_Str(uint8_t x, char* str)
{
	uint8_t* ptr;
	uint8_t  temp;

	if ((x < 1)||(x > 8))  x = 0;
	else x--;
	while(*str != 0) {
		temp = *str++;
		if (temp == ' ') temp = 10;
		else if (temp == '-')temp = 12;
		else {
			if ((temp >= '0') && (temp < ('9' + 1))) temp = temp - '0'; /* 0 --9 */
			else {
				if ((temp >= 0x41) && (temp <= 0x46))  temp = temp - 0x41 + 11;/*ABD*/
				else return ;
			}
		}
		ptr = (uint8_t*)wordlib;
		ptr += temp*32;
		Oled_DrawArea(x * 16,0,16,16,(uint8_t*)ptr);
		x++;
	}
}

/*******************************************************************************
@@name  Display_Str
@@brief 显示12*16字符串
@@param x: 位置 str :显示字符串
@@return NULL
*******************************************************************************/
void Display_Str12(uint8_t x, char* str)
{
	uint8_t* ptr;
	uint8_t  temp;

	if ((x < 1)||(x > 16))  x = 0;
	else x--;
	while(*str != 0) {
		temp = *str++ - ' ';//得到偏移后的值

		ptr = (uint8_t*)ASCII12X16;
		ptr += temp*24;
		Oled_DrawArea(x * 12,0,12,16,(uint8_t*)ptr);
		x++;
	}
}
/*******************************************************************************
@@name  Display_Str
@@brief 显示8*16字符串
@@param x: 位置 str :显示字符串  mode: 1：小数点字库
@@return NULL
*******************************************************************************/
void Display_Str8(uint16_t x, char * str , uint8_t mode)
{
	uint8_t * ptr;
	uint8_t  temp;
  
	if (x < 1 || x > 16 )	x = 0;
	else					x--;

	while (*str != 0)
	{
		if (mode == 0)
		{
			temp = *str++ - ' ';
			ptr = (uint8_t *)ASCII8X16;
		}
		else
		{
			temp = *str++ - '0';
			ptr = (uint8_t *)Decimal;
		}
		ptr += temp * 16;
		Oled_DrawArea(x * 8, 0, 8, 16, ptr);
		x++;
	}
}
/*******************************************************************************
@@name  Triangle_Str
@@brief 显示宽度为16的箭头
@@param x: 位置 pos :箭头标志位
@@return NULL
*******************************************************************************/
void Triangle_Str(uint8_t x, uint8_t pos)
{
	uint8_t* ptr;
	ptr = (uint8_t*)Triangle + 16*2*pos ;
	Oled_DrawArea(x,0,16,16,(uint8_t*)ptr);
}

/*****************************************************************
@@name  Show_Triangle
@@brief 显示设置模式中按键方向的三角方向的各种形态
@@param  empty_trgl: 0,1,2  不加粗,左加粗,右加粗
		  fill_trgl:  0,1,2  不实体,左实体,右实体
@@return NULL
*******************************************************************/
void Show_Triangle(uint8_t empty_trgl,uint8_t fill_trgl)
{
	int j;
	uint8_t* ptr;

	ptr = (uint8_t*)Triangle;

	if ((empty_trgl == 0)&&(fill_trgl == 0)) {
		for(j = 0; j < 2; j++) {
			if (j == 0)		ptr = Oled_DrawArea(0,0,16,16,(uint8_t*)ptr);
			else if (j == 1)   ptr = Oled_DrawArea(5*16,0,16,16,(uint8_t*)ptr);
		}
	} else if ((empty_trgl != 0)&&(fill_trgl == 0)) {
		if (empty_trgl == 1) {
			ptr += 32;
			Oled_DrawArea(5*16,0,16,16,(uint8_t*)ptr);
			ptr += 32;
			Oled_DrawArea(0,0,16,16,(uint8_t*)ptr);
		} else if (empty_trgl == 2) {
			Oled_DrawArea(0,0,16,16,(uint8_t*)ptr);
			ptr += 32*3;
			Oled_DrawArea(5*16,0,16,16,(uint8_t*)ptr);
		}
	} else if ((empty_trgl == 0)&&(fill_trgl != 0)) {
		if (fill_trgl == 1) {
			ptr += 32;
			Oled_DrawArea(5*16,0,16,16,(uint8_t*)ptr);
			ptr += 32*3;
			Oled_DrawArea(0,0,16,16,(uint8_t*)ptr);
		} else if (fill_trgl == 2) {
			Oled_DrawArea(0,0,16,16,(uint8_t*)ptr);
			ptr += 32*5;
			Oled_DrawArea(5*16,0,16,16,(uint8_t*)ptr);
		}
	}
}

/*******************************************************************************
@@name  Display_Str6	   
@@brief 显示宽度为6的字符串
@@param x: 位置 str :显示字符串
@@return NULL
*******************************************************************************/
void Display_Str6(uint8_t x, char* str)
{
	uint8_t* ptr;
	uint8_t  temp;
  
	if ((x < 1)||(x > 96))  x = 0;
	else x--;
	while(*str != 0) {
		temp = *str++ - ' ';//得到偏移后的值

		ptr = (uint8_t*)ASCII6X12;
		ptr += temp*12;
		Oled_DrawArea(x,0,6,12,(uint8_t*)ptr);
		x = x + 6;
	}
}

/*******************************************************************************
@@name  Display_Str10
@@brief 显示宽度为10的字符串
@@param x: 位置 str :显示字符串
@@return NULL
*******************************************************************************/
void Display_Str10(uint16_t x, char * str)
{
	uint8_t * ptr;
	uint8_t temp;
	uint8_t position = 0;

	while ((temp = *str++) != 0)
	{
		if (temp == 'C')
			temp = 11;
		else if (temp == 'E')
			temp = 12;
		else
		{
			if (temp >= '0' && temp < ('9' + 1))
				temp = temp - '0'; /* 0 --9 */
			else
				temp = 10;
		}
		ptr = (uint8_t *)Number10;
		ptr += temp * 20;
		Oled_DrawArea(x + position * 10, 0, 10, 16, ptr);
		position++;
	}
}


/*******************************************************************************
@@name  Print_Integer
@@brief 整形数据打印函数
@@param data 数据,posi 位置
@@return NULL
*******************************************************************************/
void Print_Integer(s32 data,uint8_t posi)
{
	char str[8];

	Clear_Screen();
	own_sprintf(str,"%d",data);
	Display_Str(posi,str);
}
/*******************************************************************************
@@name  Calculated_Digit 
@@brief 计算一个整数的位数
@@param number ：整数
@@return 整数的位数
*******************************************************************************/
uint8_t Calculated_Digit(int number)
{
	int n = 0;
	while(number!=0)
	{
		number = number/10;
		n++;
	}
	return n;
}

uint8_t gSet_opt = HD; //项目标志位
#define EFFECTIVE_KEY 800			  //无按键等待返回时间
/*******************************************************************************
@@name  SetOpt_Detailed
@@brief 设置选中项具体信息
@@param NULL
@@return NULL
*******************************************************************************/
void SetOpt_Detailed(void)
{
	int *temporary_set;// The address of the parameter
	int32_t max_value, min_value;
	uint16_t step;
	UI_TIMER = HIGHLIGHT_FREQUENCY * 2 - HIGHLTGHT_REDUCE;
	EFFECTIVE_KEY_TIMER = EFFECTIVE_KEY / 2;

	SetOpt_UI(1);
	Delay_Ms(300);
	Set_LongKeyFlag(1);
	Set_gKey(NO_KEY);

	switch(gSet_opt){ // Under different options
	case WKT:
	case SDT:
		if (gSet_opt == SDT)		temporary_set = (int *)&device_info.t_standby;
		else if (gSet_opt == WKT)	temporary_set = (int *)&device_info.t_work;
		gSet_table[gSet_opt][2] = device_info.t_step;// Set the step size
		break;
	case WTT:
		temporary_set = (int *)&device_info.wait_time;
		break;
	case IDT:
		temporary_set = (int *)&device_info.idle_time;
		break;
	case STP:
		temporary_set = (int *)&device_info.t_step;
		break;
	case TOV:
		temporary_set = (int *)&Hard_Context.gTurn_offv;
		break;
	case DGC:// Temperature unit selection
		temporary_set = (int *)&UI_Context.gTemperatureshowflag;
		break;
	case HD:
		temporary_set = (int *)&device_info.handers;
		break;
	}
  /*----------------Range and increase unit assignment-------------------*/
	max_value = gSet_table[gSet_opt][0];
	min_value = gSet_table[gSet_opt][1];
	step = gSet_table[gSet_opt][2];

	if (UI_Context.gTemperatureshowflag == 1
	&&	(gSet_opt == WKT || gSet_opt ==SDT)
		) // F°Mode maximum value minimum conversion
	{
		max_value = gSet_table[8][0];
		min_value = gSet_table[8][1];
		step = gSet_table[8][2];
	}
	while(1)
	{
		SetOpt_UI(1);
		/*------------------Enter the option after the operation-------------------*/
		if (Get_gKey() == KEY_V1)
		{	// Less
			if (gSet_opt == DGC && UI_Context.gTemperatureshowflag > 0)// Switch the temperature mode
			{
				UI_Context.gTemperatureshowflag--;
				device_info.t_work = TemperatureShow_Change(1,device_info.t_work);
				device_info.t_standby = TemperatureShow_Change(1,device_info.t_standby);
				if (device_info.t_work < 600)	device_info.t_work = 600;//越界判断
				if (device_info.t_standby < 600)	device_info.t_standby = 600;
			}
			else if (gSet_opt != DGC)
			{
				if (*temporary_set > min_value)
				{	// Greater than the minimum (can be reduced)
					*temporary_set -= step;
					if (*temporary_set < min_value) *temporary_set = min_value;// Less than the minimum (can not be reduced)
				}
			}
			Set_gKey(NO_KEY);// Press the key to wipe out
			EFFECTIVE_KEY_TIMER = EFFECTIVE_KEY / 2;// Exit countdown count
			UI_TIMER=HIGHLIGHT_FREQUENCY-HIGHLTGHT_REDUCE;// The flashing display is counting again
			UI_Context.direction_flag = 1;
			SetOpt_UI(1);// The flashing image is displayed
		}
		else if (Get_gKey() == KEY_V2)
		{	// Plus
			if (gSet_opt == DGC && UI_Context.gTemperatureshowflag < 1)// Switch the temperature mode
			{
				UI_Context.gTemperatureshowflag++;
				device_info.t_work = TemperatureShow_Change(0, device_info.t_work);
				device_info.t_standby = TemperatureShow_Change(0, device_info.t_standby);
			}
			else if (gSet_opt != DGC)
			{
				if ((*temporary_set) < max_value)
				{
					*temporary_set += step;
					if ((*temporary_set) > max_value)
						*temporary_set = max_value;
				}
			}
			Set_gKey(NO_KEY);
			EFFECTIVE_KEY_TIMER = EFFECTIVE_KEY/2;
			UI_TIMER=HIGHLIGHT_FREQUENCY-HIGHLTGHT_REDUCE;
			UI_Context.direction_flag = 2;
			SetOpt_UI(1);
		}
		else if (Get_gKey() == (KEY_CN | KEY_V1))// Continuous reduction
		{
			if (gSet_opt == DGC && UI_Context.gTemperatureshowflag > 0)// Switch the temperature mode
			{
				UI_Context.gTemperatureshowflag--;
				device_info.t_work = TemperatureShow_Change(1,device_info.t_work);
				device_info.t_standby = TemperatureShow_Change(1,device_info.t_standby);
				if (device_info.t_work < 600)	device_info.t_work = 600;//越界判断
				if (device_info.t_standby < 600)	device_info.t_standby = 600;
			}
			else if (gSet_opt != DGC)
			{
				if (*temporary_set > min_value) {//大于最小值(可以减少)
					*temporary_set -= step;
					if (*temporary_set < min_value) *temporary_set = min_value;//小于最小值(不能减少)
				}
			}
			Delay_Ms(100);
			Set_gKey(NO_KEY);
			EFFECTIVE_KEY_TIMER = EFFECTIVE_KEY/2;
			UI_TIMER=HIGHLIGHT_FREQUENCY-HIGHLTGHT_REDUCE;
			UI_Context.direction_flag = 1;
			SetOpt_UI(1);
		}
		else if (Get_gKey() == (KEY_CN | KEY_V2))//连续加
		{
			if (gSet_opt == DGC && UI_Context.gTemperatureshowflag < 1)//切换温度模式
			{
				UI_Context.gTemperatureshowflag++;
				device_info.t_work = TemperatureShow_Change(0,device_info.t_work);
				device_info.t_standby = TemperatureShow_Change(0,device_info.t_standby);
			}
			else if (gSet_opt != DGC)
			{
				if ((*temporary_set) < max_value)
				{
					*temporary_set += step;
					if ((*temporary_set) > max_value) *temporary_set = max_value;
				}
			}
			Delay_Ms(100);
			Set_gKey(NO_KEY);
			EFFECTIVE_KEY_TIMER = EFFECTIVE_KEY/2;
			UI_TIMER=HIGHLIGHT_FREQUENCY-HIGHLTGHT_REDUCE;
			UI_Context.direction_flag = 2;
			SetOpt_UI(1);
		}
		else if (Get_gKey() == (KEY_CN | KEY_V3))//双键同时按，退出设置
		{
			EFFECTIVE_KEY_TIMER = 0;
		}
		Clear_Watchdog();
		if (EFFECTIVE_KEY_TIMER == 0)
		{
			Set_LongKeyFlag(0);
			Set_gKey(NO_KEY);
			EFFECTIVE_KEY_TIMER = EFFECTIVE_KEY;
			UI_TIMER = 200;
			UI_Context.Exit_pos = 1;
			return ;
		}
	}
}

/*******************************************************************************
@@name  SetOpt_Proc  
@@brief 设置参数过程  
@@param NULL
@@return NULL
*******************************************************************************/
void SetOpt_Proc(void)
{
	uint8_t gCalib_flag = Get_gCalib_flag();

	if (KD_TIMER > 100)
	{
		Set_gKey(NO_KEY);
		return;
	}

	if (EFFECTIVE_KEY_TIMER < (EFFECTIVE_KEY - 100) && UI_Context.Exit_pos == 1)
		UI_Context.Exit_pos = 0;

	if ((Get_gKey() & KEY_CN) == KEY_CN)
	{	//After long press, within 1S can no longer press
		Set_LongKeyFlag(0);
		KD_TIMER = 100;
	}

	if (KD_TIMER == 0)
		Set_LongKeyFlag(1);	//You can press the logo
	
	if (Get_gKey() == KEY_V1)
	{	// Stand-alone left and right keys to select menu items
		if (gSet_opt > WKT)	gSet_opt--;
		else				gSet_opt = EXW;
		Set_gKey(NO_KEY);
		EFFECTIVE_KEY_TIMER = EFFECTIVE_KEY;
		Clear_Screen();
	}
	else if (Get_gKey() == KEY_V2)
	{
		if (gSet_opt < EXW)	gSet_opt++;
		else				gSet_opt = WKT;
		Set_gKey(NO_KEY);
		EFFECTIVE_KEY_TIMER = EFFECTIVE_KEY;
		Clear_Screen();
	}

	if (Get_gKey() == (KEY_CN | KEY_V1)
	||	Get_gKey() == (KEY_CN | KEY_V2)
		) // Enter the settings for the selected item
	{
		UI_TIMER = 0;
		if (gSet_opt != EXW && gSet_opt != WDJ)
		{
			SetOpt_Detailed();
		}
		else if (gSet_opt == EXW)
		{	// Initialization
			Restore_Setting();
			Set_gKey(NO_KEY);
			Clear_Screen();
		}
		else if (gSet_opt == WDJ)
		{	// Calibration
			Clear_Watchdog();
			Delay_Ms(1000);
			Clear_Screen();
			Zero_Calibration();// Calibration zero

			if (gCalib_flag == 1)
			{
				Disk_BuffInit();
				Config_Analysis(); // Start virtual U disk
			}
			if (gCalib_flag != 0)
			{
				// Display calibration is complete
				Show_Cal(gCalib_flag);
				Clear_Watchdog();
				Delay_Ms(1000);
				Set_gCalib_flag(0);
				Clear_Screen();
			}
			Set_gKey(NO_KEY);
		}
		EFFECTIVE_KEY_TIMER = EFFECTIVE_KEY;
	}
	else if (Get_gKey() == (KEY_CN|KEY_V3) && UI_Context.Exit_pos == 0)
	{	// Drop out
		Set_LongKeyFlag(0);
		Set_gKey(NO_KEY);
		EFFECTIVE_KEY_TIMER = 0;
	}
	SetOpt_UI(0);
}
/*******************************************************************************
@@name  SetOpt_UI
@@brief 设置参数显示
@@param key:闪烁开关
@@return NULL
*******************************************************************************/
void SetOpt_UI(uint8_t key)
{
	uint8_t buf[20] = {0};
	uint8_t buf_TOV[2] = {0};
	uint8_t wdj[5];
	uint8_t buf_R[2] = ">";
	uint8_t buf_L[2] = "<";
	int16_t temp_val;

	if (gSet_opt != EXW && gSet_opt != WDJ)
		Display_Str8(0, (char *)(gSys_settings[gSet_opt]), 0);

	if (gSet_opt == WKT)
	{
		own_sprintf((char *)buf,"%d",device_info.t_work / 10);
		UI_Context.digit = Calculated_Digit(device_info.t_work / 10);
	}
	else if (gSet_opt == SDT)
	{
		own_sprintf((char *)buf,"%d",device_info.t_standby / 10);
		UI_Context.digit = Calculated_Digit(device_info.t_standby / 10);
	}
	else if (gSet_opt == WTT)
	{
		own_sprintf((char *)buf,"%d",device_info.wait_time / 100);
		UI_Context.digit = Calculated_Digit(device_info.wait_time / 100);
	}
	else if (gSet_opt == IDT)
	{
		own_sprintf((char *)buf,"%d",device_info.idle_time / 100);
		UI_Context.digit = Calculated_Digit(device_info.idle_time / 100);
	}
	else if (gSet_opt == STP)
	{
		own_sprintf((char *)buf,"%d",device_info.t_step / 10);
		UI_Context.digit = Calculated_Digit(device_info.t_step / 10);
	}
	else if (gSet_opt == TOV)
	{
		own_sprintf((char *)buf,"%d",Get_gTurn_offv() / 10);
		own_sprintf((char *)buf_TOV,"%d",Get_gTurn_offv() % 10);
		UI_Context.digit = Calculated_Digit(Get_gTurn_offv());
	}
	else if (gSet_opt == DGC)
	{
		strcpy((char *)buf, (UI_Context.gTemperatureshowflag == 0)	? "CT" : "FT");
		UI_Context.digit = 2;
	}
	else if (gSet_opt == HD)
	{
		strcpy((char *)buf, (device_info.handers == 0) ? "RT" : "LT");

		if (Get_Handers() != device_info.handers)
		{
			Init_Oled();
			Set_Handers(device_info.handers);
		}
		UI_Context.digit = 2;
	}
	else if (gSet_opt == EXW)
		own_sprintf((char *)buf, " Factory Reset");

	if (key == 1)
	{
		if (UI_TIMER < HIGHLIGHT_FREQUENCY)
		{	// Display
			if (UI_Context.direction_flag == 1)//左
			{
				Display_Str8(SET_INFO_X, (char *)buf_L, 0);// <
				if (gSet_opt == TOV){						 
					Display_Str8(SET_PROMPT_X, (char *)buf, 0);
					Display_Str8(SET_PROMPT_X + UI_Context.digit - 1, (char *)buf_TOV, 1);
				}
				else
					Display_Str8(SET_PROMPT_X, (char *)buf, 0);// Value
				Display_Str8(SET_PROMPT_X + UI_Context.digit, " ", 0);
			}
			else if (UI_Context.direction_flag == 2)
			{	// Right
				Display_Str8(SET_PROMPT_X-1, " ", 0);
				if (gSet_opt == TOV)
				{
					Display_Str8(SET_PROMPT_X, (char *)buf, 0);
					Display_Str8(SET_PROMPT_X + UI_Context.digit - 1, (char *)buf_TOV, 1);
				}
				else	Display_Str8(SET_PROMPT_X, (char *)buf, 0);//数值
				Display_Str8(SET_PROMPT_X + UI_Context.digit, (char *)buf_R, 0);//>
			}
			else
			{	// No operation
				Display_Str8(SET_INFO_X, (char *)buf_L, 0);//<
				if (gSet_opt == TOV)
				{
					Display_Str8(SET_PROMPT_X, (char *)buf, 0);
					Display_Str8(SET_PROMPT_X + UI_Context.digit - 1,(char *)buf_TOV,1);
				}
				else	Display_Str8(SET_PROMPT_X, (char *)buf, 0);//数值
				Display_Str8(SET_PROMPT_X + UI_Context.digit, (char *)buf_R, 0);//>
			} 
			if (UI_TIMER == 0)
			{	// Re-count the end of a highlight cycle
				UI_TIMER=(HIGHLIGHT_FREQUENCY * 2) -HIGHLTGHT_REDUCE;
				UI_Context.direction_flag = 0;
			}
		}
		else
		{	// Wipe out
			Display_Str8(SET_INFO_X, " ", 0);
			Display_Str8(SET_PROMPT_X + UI_Context.digit, "   ", 0);
		}
	}
	else
	{	// Did not turn on the flashing switch
		if (gSet_opt == WDJ)
		{	// Thermometer and voltage display
			if (UI_TIMER == 0)
			{
				temp_val = Get_Temp(0);// Get the current temperature
				if (Get_TemperatureShowFlag() == 1)
				{	// Get the temperature display mode
					temp_val = TemperatureShow_Change(0,temp_val);
					own_sprintf((char *)wdj, "%d} ", temp_val / 10);
				}
				else
					own_sprintf((char *)wdj,"%d{ ", temp_val/10);
				Display_Str8(8, (char *)wdj, 0);// Show temperature
				Show_Volt();// Display voltage
				UI_TIMER = 20;
			}
		}
		else if (gSet_opt == EXW) // Restore factory equipment
		{
			Display_Str6(4, (char *)buf);
		}
		else
		{   
			Display_Str8(SET_INFO_X, " ", 0);
			if (gSet_opt == TOV)
			{
				Display_Str8(SET_PROMPT_X,(char *)buf, 0);
				Display_Str8(SET_PROMPT_X + UI_Context.digit - 1, (char *)buf_TOV, 1);
			}
			else
				Display_Str8(SET_PROMPT_X,(char *)buf, 0);

			Display_Str8(SET_PROMPT_X + UI_Context.digit,"   ", 0);
		}
	}
}   
	

/*******************************************************************************
@@name  Shift_Char
@@brief The character moves from right to left to the specified position
@@param	ptr Moves the character
		pos to the specified position
@@return NULL
*******************************************************************************/
void Shift_Char(uint8_t * ptr, uint8_t pos)
{
	uint8_t k;
	uint8_t * ptr0;

	k = 80;
	pos = pos * 16;// Position * character = character location
	while (k >= pos)
	{
		ptr0 = (uint8_t *)ptr;
		Clean_Char(k + 16, 16);
		ptr0 = Oled_DrawArea(k, 0, 16, 16, (uint8_t *)ptr0);
		k -= 16;
		Delay_Ms(25);
	}
}

/*******************************************************************************
Show_TempReverse
Vertical dynamic display temperature character
word_num:	 The number of
word_width:   width
direction :   Direction (0 up, 1 down)
*******************************************************************************/
uint8_t Show_TempReverse(uint8_t num, uint8_t width, uint8_t direction)
{
	static int i, j = 0, m = 16, k;
	uint8_t g, s, b;
	uint8_t * ptr;
	int16_t num_temp;
   
	num_temp = device_info.t_work;

	num_temp = num_temp / 10;
	b = num_temp / 100;
	s = (num_temp - b * 100) / 10;
	g = (num_temp - b * 100 - s * 10);

	if (UI_Context.gTempset_showctrl == 1)
	{
		j = 1;
		m = 0;
		UI_Context.gTempset_showctrl = 0;
	}
	else if (UI_Context.gTempset_showctrl == 2)
	{
		j = 0;
		m = 16;
		UI_Context.gTempset_showctrl = 0;
	}

	for(i = 0; i < width * 2; i++)
	{
		gTemp_array[0 * 32 + i] = Reverse_Bin8(*(wordlib + b * 32 + i));	// Reverse 8 bits
		gTemp_array[1 * 32 + i] = Reverse_Bin8(*(wordlib + s * 32 + i));
		gTemp_array[2 * 32 + i] = Reverse_Bin8(*(wordlib + g * 32 + i));
		if (Get_TemperatureShowFlag() == 1)
			gTemp_array[3*32 + i] = Reverse_Bin8(*(wordlib + 15 * 32 + i));
		else
			gTemp_array[3*32 + i] = Reverse_Bin8(*(wordlib + 13 * 32 + i));
	}
	// Modify the initial value of K can change the position of the character to start rolling or ten or hundred
	for (k = (3 - num) * width * 2; k < width * 2 * 3; k += width * 2)
		for (i = 0; i < width ; i++)
		{
			// The upper half of the upper half with the u16 is easy to shift
			gTemp_array_u16[i + k] = ((gTemp_array[i + k] & 0x00FF) << 8) | gTemp_array[i + k + width] ;
			if (direction == 0)
			{	// on
				if (m == 0)	gTemp_array_u16[i + k] <<= j;// Below empty, above the show
				else		gTemp_array_u16[i + k] >>= m;// Above the empty, the following shows
			}
			else
			{
				if (j == 0)	gTemp_array_u16[i + k] <<= m;// Below empty, above the show
				else		gTemp_array_u16[i + k] >>= j;// Above the empty, the following shows
			}
			gTemp_array[i + k] = (gTemp_array_u16[i + k] & 0xFF00) >> 8;
			gTemp_array[i + k + width] = gTemp_array_u16[i + k] & 0x00FF;
		}
	for (i = 0; i < width * 2; i++)
	{
		gTemp_array[0 * 32 + i] = Reverse_Bin8(gTemp_array[0 * 32 + i]);
		gTemp_array[1 * 32 + i] = Reverse_Bin8(gTemp_array[1 * 32 + i]);
		gTemp_array[2 * 32 + i] = Reverse_Bin8(gTemp_array[2 * 32 + i]);
		gTemp_array[3 * 32 + i] = Reverse_Bin8(gTemp_array[3 * 32 + i]);
	}

	ptr = (uint8_t*)gTemp_array;
	for (i = 1; i <= 4; i++)
		ptr = Oled_DrawArea(16 * i, 0, 16, 16, (uint8_t *)ptr);
	if ((m == 0 && j == 0) || (m == 0 && j == 16))
		return 0 ;

	if (m == 0)	j++;
	else		m--;
	return 1;
}

/*******************************************************************************
@@name  Show_Set
@@brief Display settings interface, from right to left to move over
@@param NULL
@@return NULL
*******************************************************************************/
void Show_Set(void)
{
	uint8_t m ,k;
	int16_t num_temp = device_info.t_work;
	
	Shift_Char((uint8_t*)Triangle + 0 * 32, 0);		//<
	m = num_temp / 1000;   						// Hundred
	Shift_Char((uint8_t *)wordlib + m * 32, 1);
	k = (num_temp - m * 1000)/100;				// Ten
	Shift_Char((uint8_t *)wordlib + k * 32, 2);
	m = (num_temp - m * 1000 - k * 100)/10;		// One
	Shift_Char((uint8_t *)wordlib + m * 32, 3);
	
	Shift_Char(
		(uint8_t *)wordlib + (
			(UI_Context.gTemperatureshowflag)
			? 15
			: 13
		) * 32, 4); // F
	
	Shift_Char((uint8_t *)Triangle + 1 * 32, 5);
}
/*******************************************************************************
@@name  Roll_Num
@@brief In the setting, the number of bits of the temperature value to be scrolled
@@param Step, Flag (0+ or 1-)
@@return Need to scroll the number
*******************************************************************************/
uint8_t Roll_Num(uint16_t step, uint8_t flag)
{
	u16 b1,b2,g1,g2;
	s16 num_temp;

	num_temp = device_info.t_work;	
	b2 = (num_temp) / 1000;	   
	g2 = (num_temp) / 100;		

	if (flag == 0)
	{
		b1 = (num_temp + step) / 1000;
		g1 = (num_temp + step) / 100;
		if (b1 != b2)		return 3;
		else if (g1 != g2)	return 2;
		else				return 1;
	}
	else
	{
		b1 = (num_temp - step) / 1000;
		g1 = (num_temp - step) / 100;

		if (b1 != b2)		return 3;
		else if (g1 != g2)	return 2;
		else				return 1;
	}
}

/*******************************************************************************
@@name  Temp_SetProc
@@brief Temporarily set the temperature
@@param NULL
@@return NULL
*******************************************************************************/
void TempSet_Proc(void)
{
	static uint8_t add_step = 0;

	uint8_t theRoll_num = 3;
	int *temporary_set;
	int32_t max_value,min_value;
	uint16_t step = device_info.t_step;

	Set_LongKeyFlag(1);
	temporary_set = (int *)&device_info.t_work;

	if (UI_Context.gTemperatureshowflag == 0)
	{
		max_value = gSet_table[WKT][0];
		min_value = gSet_table[WKT][1];
	}
	else
	{
		max_value = gSet_table[8][0];
		min_value = gSet_table[8][1];
	}
	step = device_info.t_step;

	if (EFFECTIVE_KEY_TIMER > 0)
	{
		if (UI_Context.gCont != 0)
		{
			UI_Context.gCont = 0;
			Set_LongKeyFlag(0);
			Clear_Screen();
			Show_Set();
		}
		if (Get_gKey() == KEY_V3)
		{
			Clear_Screen();
			Set_gKey(NO_KEY);
			while(1)
			{
				if (Get_gKey() == (KEY_V3 | KEY_CN))
				{
					EFFECTIVE_KEY_TIMER = EFFECTIVE_KEY / 2;
					break;
				}
				
				if (EFFECTIVE_KEY_TIMER % 20 == 0)
					Show_Volt();
				if (EFFECTIVE_KEY_TIMER == 0)
					EFFECTIVE_KEY_TIMER = EFFECTIVE_KEY / 2;
				Clear_Watchdog();				
			}
		}
		else if (Get_gKey() == KEY_V1)
		{
			if (*temporary_set > min_value)
			{
				add_step = 0;
				UI_Context.gTempset_showctrl = 1;
				theRoll_num = Roll_Num(step, 1);
				*temporary_set -= step;
				if (*temporary_set < min_value)
					*temporary_set = min_value;
				while(Show_TempReverse(theRoll_num, 16, 1))
					;
				UI_Context.gTempset_showctrl = 2;
				while(Show_TempReverse(theRoll_num, 16, 1))
					;
				Show_Triangle(1, 0);
				if (*temporary_set == min_value)
					Show_Triangle(0, 1);
			}
			Set_gKey(NO_KEY);
			EFFECTIVE_KEY_TIMER = EFFECTIVE_KEY / 2;
		}
		else if (Get_gKey() == KEY_V2)
		{
			if ((*temporary_set) < max_value)
			{
				add_step = 0;
				UI_Context.gTempset_showctrl = 1;
				theRoll_num = Roll_Num(step,0);
				*temporary_set += step;
				if ((*temporary_set) > max_value)
					*temporary_set = max_value;
				while(Show_TempReverse(theRoll_num,16, 0))
					;
				UI_Context.gTempset_showctrl = 2;
				while(Show_TempReverse(theRoll_num,16, 0))
					;
				Show_Triangle(2, 0);
				if (*temporary_set == max_value)
					Show_Triangle(0, 2);
			}
			Set_gKey(NO_KEY);
			EFFECTIVE_KEY_TIMER = EFFECTIVE_KEY / 2;
		}
		else if (Get_gKey() == (KEY_CN | KEY_V1))
		{
			if (*temporary_set > min_value)
			{
				if (add_step < 250 && device_info.t_step == 10)
					add_step += 10;
				step = device_info.t_step + add_step;
				UI_Context.gTempset_showctrl = 1;
				theRoll_num = Roll_Num(step,1);
				*temporary_set -= step;
				if (*temporary_set < min_value)
					*temporary_set = min_value;
				while(Show_TempReverse(theRoll_num, 16, 1))
					;
				UI_Context.gTempset_showctrl = 2;
				while(Show_TempReverse(theRoll_num, 16, 1))
					;
				Show_Triangle(1, 0);
				if (*temporary_set == min_value)
					Show_Triangle(0, 1);
			}
			Delay_Ms(50);
			Set_gKey(NO_KEY);
			EFFECTIVE_KEY_TIMER = EFFECTIVE_KEY / 2;
		}
		else if (Get_gKey() == (KEY_CN | KEY_V2))
		{
			if ((*temporary_set) < max_value)
			{
				if (add_step < 250 && device_info.t_step == 10)
					add_step += 10;
				step = device_info.t_step + add_step;
				UI_Context.gTempset_showctrl = 1;
				theRoll_num = Roll_Num(step, 0);
				*temporary_set += step;
				if ((*temporary_set) > max_value)
					*temporary_set = max_value;
				while(Show_TempReverse(theRoll_num, 16, 0))
					;
				UI_Context.gTempset_showctrl = 2;
				while(Show_TempReverse(theRoll_num, 16, 0))
					;
				Show_Triangle(2, 0);
				if (*temporary_set == max_value)
					Show_Triangle(0, 2);
			}
			Delay_Ms(50);
			Set_gKey(NO_KEY);
			EFFECTIVE_KEY_TIMER = EFFECTIVE_KEY / 2;
		}
		if (EFFECTIVE_KEY_TIMER < EFFECTIVE_KEY / 2 -50)
			add_step = 0;
	}
}
/*******************************************************************************
@@name  Temp_SetProc
@@brief Set the relevant parameters or temporary temperature
@@param key 0: only set the temporary temperature
			1: set and save the parameters
@@return NULL
*******************************************************************************/
void Temp_SetProc(uint8_t key)
{	   
	if (key == 0)
		TempSet_Proc();
	else if (EFFECTIVE_KEY_TIMER > 0)
		SetOpt_Proc();
}

/*******************************************************************************
@@name  OLed_Display
@@brief The screen displays the main control function
@@param NULL
@@return NULL
*******************************************************************************/
void OLed_Display(void)
{
	static uint16_t bk = 0x33, id_cnt, td_cnt;
	static uint8_t td_flag, ver_flag, config_show;

	uint16_t ht_flag;
	int16_t temp_val,dst_temp;

	switch (Get_CtrlStatus())
	{
	case IDLE:
		if (UI_Context.gCont == 1)
		{
			UI_Context.gCont = 0;
			Clear_Screen();
		}

		if (ver_flag == 0)
		{
			Display_BG();
			Show_Ver(device_info.ver, 0);
			ver_flag = 1;
		}
		else if (UI_TIMER == 0 && G6_TIMER != 0)
		{
			Show_Notice();
			UI_TIMER = 50;
		}
		if (G6_TIMER == 0)
		{
			id_cnt++;
			if (id_cnt == 50)
				Sc_Pt(bk--);
			if (bk == 0)
				Oled_DisplayOff();
		}

		if (Get_MmaShift() == 1 || Get_gKey() != NO_KEY)
		{
			G6_TIMER = device_info.idle_time;
			bk = 0x33;
			Sc_Pt(bk);
			Oled_DisplayOn();
		}
		break;
	case TEMP_CTR:
		if (UI_Context.gCont == 0)
		{
			UI_Context.gCont = 1;
			Set_LongKeyFlag(1);
			Clear_Screen();
		}

		ht_flag = Get_HtFlag();

		if (ht_flag != 1)
		{
			if (td_flag == 1)
				Clear_Screen();
			td_cnt  = 0;
			td_flag = 0;
		}

		if (td_cnt == 75 && td_flag == 0)
		{
			Clear_Screen();
			td_flag = 1;
		}

		if (td_flag && UI_TIMER == 0)
		{
			temp_val = Get_TempVal();
			dst_temp = device_info.t_work;
			if (Get_TemperatureShowFlag() == 1)
			{
				temp_val = TemperatureShow_Change(0, temp_val);
				dst_temp = TemperatureShow_Change(0, dst_temp);
			}
			Show_TempDown(temp_val,dst_temp);
			UI_TIMER = 50;
		}
		if (TEMPSHOW_TIMER == 0 && !td_flag)
		{
			temp_val = Get_TempVal();
			if (temp_val > device_info.t_work && (temp_val - device_info.t_work < 18))
				temp_val = device_info.t_work;
			else if (temp_val <= device_info.t_work && (device_info.t_work - temp_val < 18))
				temp_val = device_info.t_work;
			if (Get_TemperatureShowFlag() == 1)
				temp_val = TemperatureShow_Change(0, temp_val);

			Display_Temp(1, temp_val / 10);
			Show_HeatingIcon(ht_flag, Get_MmaActive());
			td_cnt++;
		}
		break;
	case TEMP_SET:
		Temp_SetProc((Get_TEMP_SET_Pos() == 0) ? 0 : 1);
		break;
//	case VOLT:
//		Show_Volt();
//		break;
	case CONFIG:
		if (UI_Context.gCont == 1)
		{
			UI_Context.gCont = 0;
			Clear_Screen();
		}
		switch(Get_gKey())
		{
		case KEY_CN | KEY_V1:
			config_show = 1;
			Display_BG();
		break;
		case KEY_CN | KEY_V2:
			config_show = 2;
			Show_Ver(device_info.ver, 1);
			break;
		case KEY_CN | KEY_V3:
			config_show = 0;
			Show_Config();
			break;
		default:
			break;
		}

		if (config_show != 3)
		{
			Set_gKey(NO_KEY);
			config_show = 3;
		}
		break;

//	case THERMOMETER://温度计模式
//		if (gCont == 0) {
//			gCont = 1;
//			Clear_Screen();//清屏
//		}
//		if (gCalib_flag != 0) {
//			//显示校准完成
//			Show_Cal(gCalib_flag);
//			gCalib_flag = 0;
//			Clear_Screen();
//		}
//		if (UI_TIMER == 0) {
//			temp_val = Get_Temp(0);
//			if (Get_TemperatureShowFlag() == 1) {
//				temp_val = TemperatureShow_Change(0,temp_val);
//			}
//			Display_Temp(1,temp_val/10);
//			Display_Str(6,"F");
//			UI_TIMER = 20;
//		}
//		break;
	case ALARM:
		if (UI_Context.gCont == 0)
		{
			UI_Context.gCont = 1;
			Clear_Screen();
		}
		if (UI_Context.gCont == 1 && UI_TIMER == 0)
		{
			Show_Warning();
			UI_TIMER = 50;
		}
		break;
	case WAIT:
		temp_val = Get_TempVal();
		if (temp_val > device_info.t_standby && temp_val - device_info.t_standby <= 18)
			temp_val = device_info.t_standby;
		else if (temp_val <= device_info.t_standby && device_info.t_standby - temp_val <= 18)
			temp_val = device_info.t_standby;

		ht_flag = Get_HtFlag();

		if (ht_flag != 1)
		{
			if (td_flag == 1)
				Clear_Screen();
			td_cnt  = 0;
			td_flag = 0;
		}

		if (td_cnt == 50 && td_flag == 0)
		{
			Clear_Screen();
			td_flag = 1;
		}

		if (td_flag && UI_TIMER == 0)
		{
			dst_temp = device_info.t_standby;
			if (Get_TemperatureShowFlag() == 1)
			{
				temp_val = TemperatureShow_Change(0, temp_val);
				dst_temp = TemperatureShow_Change(0, dst_temp);
			}
			Show_TempDown(temp_val,dst_temp);
			UI_TIMER = 50;
		}
		if (TEMPSHOW_TIMER == 0 && !td_flag)
		{
			if (Get_TemperatureShowFlag() == 1)
				temp_val = TemperatureShow_Change(0,temp_val);

			Display_Temp(1, temp_val / 10);
			Show_HeatingIcon(ht_flag, Get_MmaActive());
			td_cnt++;
		}
		break;
	default:
		break;
	}
}
