#include <string.h>
#include <stdio.h>

#include "APP_Version.h"
#include "Disk.h"
#include "Bios.h"
#include "Flash.h"
#include "Oled.h"
#include "UI.h"
#include "CTRL.h"
#include "HARDWARE.h"
#include "own_sprintf.h"

#define Delay_mS	Delay_Ms
void Disk_SecWrite(uint8_t* pBuffer, uint32_t DiskAddr);
void Disk_SecRead (uint8_t* pBuffer, uint32_t DiskAddr);
void Soft_Delay(void);

const uint8_t BOOT_SEC[512] = {
	0xEB, 0x3C, 0x90, 0x4D, 0x53, 0x44, 0x4F, 0x53,
	0x35, 0x2E, 0x30, 0x00, 0x02, 0x01, 0x08, 0x00,
	0x02, 0x00, 0x02, 0x50, 0x00, 0xF8, 0x0c, 0x00,
	0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x29, 0xA2,
	0x98, 0xE4, 0x6C, 0x4E, 0x4F, 0x20, 0x4E, 0x41,
	0x4D, 0x45, 0x20, 0x20, 0x20, 0x20, 0x46, 0x41,
	0x54, 0x31, 0x32, 0x20, 0x20, 0x20, 0x33, 0xC9,
	0x8E, 0xD1, 0xBC, 0xF0, 0x7B, 0x8E, 0xD9, 0xB8,
	0x00, 0x20, 0x8E, 0xC0, 0xFC, 0xBD, 0x00, 0x7C,
	0x38, 0x4E, 0x24, 0x7D, 0x24, 0x8B, 0xC1, 0x99,
	0xE8, 0x3C, 0x01, 0x72, 0x1C, 0x83, 0xEB, 0x3A,
	0x66, 0xA1, 0x1C, 0x7C, 0x26, 0x66, 0x3B, 0x07,
	0x26, 0x8A, 0x57, 0xFC, 0x75, 0x06, 0x80, 0xCA,
	0x02, 0x88, 0x56, 0x02, 0x80, 0xC3, 0x10, 0x73,
	0xEB, 0x33, 0xC9, 0x8A, 0x46, 0x10, 0x98, 0xF7,
	0x66, 0x16, 0x03, 0x46, 0x1C, 0x13, 0x56, 0x1E,
	0x03, 0x46, 0x0E, 0x13, 0xD1, 0x8B, 0x76, 0x11,
	0x60, 0x89, 0x46, 0xFC, 0x89, 0x56, 0xFE, 0xB8,
	0x20, 0x00, 0xF7, 0xE6, 0x8B, 0x5E, 0x0B, 0x03,
	0xC3, 0x48, 0xF7, 0xF3, 0x01, 0x46, 0xFC, 0x11,
	0x4E, 0xFE, 0x61, 0xBF, 0x00, 0x00, 0xE8, 0xE6,
	0x00, 0x72, 0x39, 0x26, 0x38, 0x2D, 0x74, 0x17,
	0x60, 0xB1, 0x0B, 0xBE, 0xA1, 0x7D, 0xF3, 0xA6,
	0x61, 0x74, 0x32, 0x4E, 0x74, 0x09, 0x83, 0xC7,
	0x20, 0x3B, 0xFB, 0x72, 0xE6, 0xEB, 0xDC, 0xA0,
	0xFB, 0x7D, 0xB4, 0x7D, 0x8B, 0xF0, 0xAC, 0x98,
	0x40, 0x74, 0x0C, 0x48, 0x74, 0x13, 0xB4, 0x0E,
	0xBB, 0x07, 0x00, 0xCD, 0x10, 0xEB, 0xEF, 0xA0,
	0xFD, 0x7D, 0xEB, 0xE6, 0xA0, 0xFC, 0x7D, 0xEB,
	0xE1, 0xCD, 0x16, 0xCD, 0x19, 0x26, 0x8B, 0x55,
	0x1A, 0x52, 0xB0, 0x01, 0xBB, 0x00, 0x00, 0xE8,
	0x3B, 0x00, 0x72, 0xE8, 0x5B, 0x8A, 0x56, 0x24,
	0xBE, 0x0B, 0x7C, 0x8B, 0xFC, 0xC7, 0x46, 0xF0,
	0x3D, 0x7D, 0xC7, 0x46, 0xF4, 0x29, 0x7D, 0x8C,
	0xD9, 0x89, 0x4E, 0xF2, 0x89, 0x4E, 0xF6, 0xC6,
	0x06, 0x96, 0x7D, 0xCB, 0xEA, 0x03, 0x00, 0x00,
	0x20, 0x0F, 0xB6, 0xC8, 0x66, 0x8B, 0x46, 0xF8,
	0x66, 0x03, 0x46, 0x1C, 0x66, 0x8B, 0xD0, 0x66,
	0xC1, 0xEA, 0x10, 0xEB, 0x5E, 0x0F, 0xB6, 0xC8,
	0x4A, 0x4A, 0x8A, 0x46, 0x0D, 0x32, 0xE4, 0xF7,
	0xE2, 0x03, 0x46, 0xFC, 0x13, 0x56, 0xFE, 0xEB,
	0x4A, 0x52, 0x50, 0x06, 0x53, 0x6A, 0x01, 0x6A,
	0x10, 0x91, 0x8B, 0x46, 0x18, 0x96, 0x92, 0x33,
	0xD2, 0xF7, 0xF6, 0x91, 0xF7, 0xF6, 0x42, 0x87,
	0xCA, 0xF7, 0x76, 0x1A, 0x8A, 0xF2, 0x8A, 0xE8,
	0xC0, 0xCC, 0x02, 0x0A, 0xCC, 0xB8, 0x01, 0x02,
	0x80, 0x7E, 0x02, 0x0E, 0x75, 0x04, 0xB4, 0x42,
	0x8B, 0xF4, 0x8A, 0x56, 0x24, 0xCD, 0x13, 0x61,
	0x61, 0x72, 0x0B, 0x40, 0x75, 0x01, 0x42, 0x03,
	0x5E, 0x0B, 0x49, 0x75, 0x06, 0xF8, 0xC3, 0x41,
	0xBB, 0x00, 0x00, 0x60, 0x66, 0x6A, 0x00, 0xEB,
	0xB0, 0x4E, 0x54, 0x4C, 0x44, 0x52, 0x20, 0x20,
	0x20, 0x20, 0x20, 0x20, 0x0D, 0x0A, 0x52, 0x65,
	0x6D, 0x6F, 0x76, 0x65, 0x20, 0x64, 0x69, 0x73,
	0x6B, 0x73, 0x20, 0x6F, 0x72, 0x20, 0x6F, 0x74,
	0x68, 0x65, 0x72, 0x20, 0x6D, 0x65, 0x64, 0x69,
	0x61, 0x2E, 0xFF, 0x0D, 0x0A, 0x44, 0x69, 0x73,
	0x6B, 0x20, 0x65, 0x72, 0x72, 0x6F, 0x72, 0xFF,
	0x0D, 0x0A, 0x50, 0x72, 0x65, 0x73, 0x73, 0x20,
	0x61, 0x6E, 0x79, 0x20, 0x6B, 0x65, 0x79, 0x20,
	0x74, 0x6F, 0x20, 0x72, 0x65, 0x73, 0x74, 0x61,
	0x72, 0x74, 0x0D, 0x0A, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0xAC, 0xCB, 0xD8, 0x55, 0xAA
};

static uint8_t  gDisk_buff[0x2600];
static uint32_t  gDisk_var[(512 + 32 + 28) / 4];	 // 磁盘工作数据区
static uint32_t *gV32  = (uint32_t *)&gDisk_var[512 / 4];
static uint8_t  *gVar  = (uint8_t  *)&gDisk_var[512 / 4 + 8];

static uint8_t *gBuff = (uint8_t *) &gDisk_var[0];
uint8_t  gFat_data[]= {0xF8, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

const char * gKey_words[] = {
	"StbTemp",
	"WkTemp",
	"SlpTime",
	"IdlTime",
	"TempStp",
	"OffVolt",
	"Temp",
	"Hand",
	"ZeroP_Ad"
};

const char * gDef_set[]   = {
	"StbTemp=200",
	"WkTemp=300",
	"SlpTime=180",
	"IdlTime=360",
	"TempStp=10",
	"OffVolt=10",
	"Temp=0",
	"Hand=0",
	"ZeroP_Ad=239"
};
const char * gSet_range[] = {
	"	#(100~400)\r\n",
	"	#(100~400)\r\n",
	"	#(60~999)\r\n",
	"	#(100~999)\r\n",
	"	#(1~25)\r\n",
	"	#(5~19)\r\n",
	"	#(0,1)\r\n",
	"	#(0,1)\r\n",
	"	#ReadOnly\r\n"
};

static uint8_t gFile_con[512];
uint8_t gRewriteflag[16];

#define CONFIG_CONT	9
#define ROW_CONT	35
#define FILE_CONT	254
						   
/*******************************************************************************

*******************************************************************************/
void Soft_Delay()
{
	volatile int i, j;
	for (i = 0 ; i < 1000 ; i++)
		for (j = 0; j < 100; j++)
			;
}

static uint16_t Get_Decimal(uint8_t * src)
{
	uint16_t value = 0;
	uint8_t b;

	while ((b = *src) != 0)
	{
		src++;
		if (b < '0' || b > '9')
			break;
		value = value * 10 + (b - '0');
	}
	return value;
}

/*******************************************************************************

*******************************************************************************/
void Set_Value(uint8_t * str, uint8_t k)
{
	uint16_t value = Get_Decimal(str);
	uint8_t gCalib_flag = Get_gCalib_flag();

	switch(k)
	{
	case WKT:		// StbTemp	#(100~400)
		if (!gCalib_flag)
			device_info.t_standby = value * 10;
		break;
	case SDT:		// WkTemp	#(100~400)
		if (!gCalib_flag)
			device_info.t_work = value * 10;
		break;
	case WTT:		// SlpTime	#(60~999)
		if (!gCalib_flag)
			device_info.wait_time = value * 100;
		break;
	case IDT:		// IdlTime	#(100~999)
		if (!gCalib_flag)
			device_info.idle_time = value * 100;
		break;
	case STP:		// TempStp	T_Step=10 #(5~25)
		if (!gCalib_flag)
			device_info.t_step = value * 10;
		break;
	case TOV:		// OffVolt	Turn_Off_v=10   #(5~19)
		if (!gCalib_flag)
			Set_gTurn_offv(value * 10);
		break;
	case DGC:		// Temp		TempShowFlag=0  #(0,1)
		if (!gCalib_flag)
			Set_TemperatureShowFlag(value);
		break;
	case HD:		// Hand		handers=0  #(0,1)
		if (!gCalib_flag)
			device_info.handers = value;
		break;	
	case 8:		// ZeroP_Ad	ReadOnly
		if (!gCalib_flag)
			Set_gZerop_ad(value);
		break;
	default:
		break;
	}
}
/*******************************************************************************

*******************************************************************************/
uint8_t Check_Value(uint8_t * str, uint16_t k)
{
	uint16_t value = Get_Decimal(str);
	switch(k)
	{
	case WKT:		// StbTemp	#(100~400)
	case SDT:		// WkTemp	#(100~400)
		if (Get_TemperatureShowFlag() == 0)
		{
			if (value < 100 || value > 400)
				return 0;
		}
		else if (value < 212 || value > 759)
				return 0;
		break;
	case WTT:		// SlpTime	#(60~999)
		if (value < 60 || value > 999)
			return 0;
		break;
	case IDT:		// IdlTime	#(100~999)
		if (value < 100 || value > 999)
			return 0;
		break;
	case STP:		// TempStp	T_Step=10 #(5~25)
		if (value < 5 || value > 25)
			return 0;
		break;
	case TOV:		// OffVolt	Turn_Off_v=10   #(5~19)
		if (value < 5 || value > 19)
			return 0;
		break;
	case DGC:		// Temp		TempShowFlag=0  #(0,1)
		if (value > 1)
		  return 0;
		break;
	case HD:		// Hand		handers=0  #(0,1)
		if (value > 1)
		  return 0;
		break;
	case 8:			// ZeroP_Ad	ReadOnly
	default:
		break;
	}
	return 1;
}
/*******************************************************************************
@@name  Disk_BuffInit
@@brief Disk data is initialized
@@param NULL
@@return NULL
*******************************************************************************/
void Disk_BuffInit(void)
{
	memcpy(gDisk_buff, (uint8_t *)APP_BASE, 0x2600);
	memset(gRewriteflag, 0, 16);
}
/*******************************************************************************
@@name  Upper
@@brief Lowercase to uppercase
@@param
@@return NULL
*******************************************************************************/
static void toUpper(uint8_t * src, uint16_t len)
{
	while (len != 0)
	{
		--len;
		if (*src >= 'a' && *src <= 'z')
			*src -= ' ';
		src++;
	}
}
/*******************************************************************************
@@name  SearchFile
@@brief Find the file
@@param	pfilename = file name
		pfilelen = file length
		root_addr = find the start address
@@return address of the file
*******************************************************************************/
uint8_t * SearchFile(uint8_t * pfilename, uint16_t * pfilelen, uint16_t * root_addr)
{
	uint16_t n, sector;
	uint8_t  str_name[11];
	uint8_t *pdiraddr;

	pdiraddr = ROOT_SECTOR;

	for (n = 0; n < 16; n++)
	{
		memcpy(str_name, pdiraddr, 11);
		toUpper(str_name, 11);
		if (memcmp(str_name, pfilename, 11) == 0)
		{
			*pfilelen = *(uint16_t *)(pdiraddr + 0x1C);
			// memcpy((uint8_t *)pfilelen, (uint16_t *)(pdiraddr + 0x1C), 2);
			sector = *(uint16_t *)(pdiraddr + 0x1A);
			// memcpy((uint8_t *)&sector, pdiraddr + 0x1A, 2);
			return ((uint8_t *)FILE_SECTOR + (sector - 2) * 512);
		}
		pdiraddr += 32;
		root_addr++;
	}
	return NULL;
}

const uint8_t LOGO[] = {
	0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF,
	0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0x00 ,0x00 ,0x00 ,0x7F ,0xC0 ,0x00 ,0x00,

	0x00 ,0x00 ,0x00 ,0x01 ,0xC0 ,0xFF ,0xFF ,0xFF ,0x80 ,0x1F ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFD,
	0x80 ,0xFF ,0xFF ,0xFF ,0x80 ,0x1F ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFD ,0xFF ,0x00 ,0x00 ,0x00,

	0x7F ,0x9F ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFD ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xCF ,0xFF ,0xFF,
	0xFF ,0xFF ,0xFF ,0xFD ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xF7 ,0xFF ,0xFF ,0xF0 ,0x00 ,0x03 ,0xFD,

	0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xF8 ,0x00 ,0x00 ,0x07 ,0xFF ,0xFC ,0x01 ,0xFF ,0xFF ,0xFF ,0xFF,
	0xFF ,0xFE ,0x07 ,0x81 ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0x9F ,0xFF,

	0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0x0F ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF,
	0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFE ,0x07 ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF,

	0xFF ,0xFF ,0x9F ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0x9F ,0xFF,
	0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0x9F ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF
};

uint8_t Setting_Analysis(uint8_t* p_file , uint16_t root_addr)
{
	uint16_t k;
	uint32_t i, j, m, flag;
	uint8_t t_p[CONFIG_CONT][ROW_CONT];
	uint8_t str[FILE_CONT];
	uint8_t is_illegality = 0;

	m = 0;
	j = 0;

	memset(t_p, 0, CONFIG_CONT * ROW_CONT);
	memcpy((uint8_t *)gFile_con, p_file, 512);

	for (k = 0; k < CONFIG_CONT; k++)
	{	// take out CONFIG_CONT row
		j = 0;
		for (i = m; i < strlen((char *)gFile_con); i++)
		{	// Calculates where the set value string is located
			if (gFile_con[i] == '\r' || gFile_con[i] == '\n')
				break;
			else
			{
				if (j < ROW_CONT)
					t_p[k][j++] = gFile_con[i];
				m++;
			}
		}
		t_p[k][j] = '\0';
		m = i + 2;
	}

	for (k = 0; k < CONFIG_CONT; k++)
	{	// analysis CONFIG_CONT row
		if (memcmp(t_p[k], gKey_words[k], strlen(gKey_words[k])) == 0)
		{	// Find the keyword
			flag = 0;
			for (i = strlen(gKey_words[k]); i < strlen((char *)t_p[k]); i++)
			{	// Set the value to be valid
				if (t_p[k][i] >= '0' && t_p[k][i] <= '9')
				{
					if (t_p[k][i] == '0')
					{
						if (k == 6 || k == 7)
							flag = 1;
						else
							flag = 0;
						break;
					}
					flag = 1;
					break;
				}
				else if ((t_p[k][i] != 0x20) && (t_p[k][i] != 0x3d))
				{	// Spaces and equals
					flag = 0;
					break;
				}
			}
			if (flag && Check_Value(t_p[k] + i, k))
			{	// Set value is legal
				Set_Value(t_p[k] + i, k);
				if (k == 0)			own_sprintf((char *)t_p[k], "StbTemp=%d",	device_info.t_standby / 10);
				else if (k == 1)	own_sprintf((char *)t_p[k], "WkTemp=%d",	device_info.t_work / 10);
				else if (k == 2)	own_sprintf((char *)t_p[k], "SlpTime=%d",	device_info.wait_time / 100);
				else if (k == 3)	own_sprintf((char *)t_p[k], "IdlTime=%d",	device_info.idle_time / 100);
				else if (k == 4)	own_sprintf((char *)t_p[k], "TempStp=%d",	device_info.t_step / 10);
				else if (k == 5)	own_sprintf((char *)t_p[k], "OffVolt=%d",	Get_gTurn_offv() / 10);
				else if (k == 6)	own_sprintf((char *)t_p[k], "Temp=%d",		Get_TemperatureShowFlag());
				else if (k == 7)	own_sprintf((char *)t_p[k], "Hand=%d",		device_info.handers);
				else if (k == 8)	own_sprintf((char *)t_p[k], "ZeroP_Ad=%d",	Get_gZerop_ad());
			}
			else
			{	// Setting value is not legal
				memset(t_p[k], 0, strlen((char *)t_p[k]));
				memcpy(t_p[k], gDef_set[k], strlen((char *)gDef_set[k]));
				is_illegality = 1;
			}
		}
		else
		{	// Wood has to find keywords
			memset(t_p[k], 0, strlen((char *)t_p[k]));
			memcpy(t_p[k], gDef_set[k], strlen((char *)gDef_set[k]));
			is_illegality = 1;
		}
	}
		
//	if (is_illegality || gCalib_flag)
//	{
	memset(str, 0x00, FILE_CONT);
	m = 0;
	for (k = 0; k < CONFIG_CONT; k++)
	{
		strcat((char *)str, (char *)t_p[k]);
		strcat((char *)str, (char *)gSet_range[k]);
	}
	m = strlen((char *)str);
	gDisk_buff[0x400 + root_addr*32 + 0x1C] = m % 256;
	gDisk_buff[0x400 + root_addr*32 + 0x1D] = m / 256;
	
	gRewriteflag[(p_file - ROOT_SECTOR + 0x200) / 0x400] = 1;
	memcpy(p_file, str,strlen((char *)str));
//		ReWriteFlash();
//	} 
	return is_illegality;
}


/*******************************************************************************
@@name  Config_Analysis
@@brief Configuration file analysis
@@param NULL
@@return NULL
*******************************************************************************/
uint8_t Config_Analysis(void)
{
	uint32_t k,m;
	uint8_t  str[FILE_CONT];
	uint16_t file_len;
	uint8_t *p_file;
	uint16_t root_addr;
	
	root_addr = 0;
	
	if ((p_file = SearchFile((uint8_t *)"CONFIG  TXT", &file_len, &root_addr)) != NULL)
	{
		if (Setting_Analysis(p_file, root_addr) != 0 || Get_gCalib_flag() != 0)
			ReWriteFlash();
	}
	else
	{
		if ((p_file = SearchFile((uint8_t *)"LOGOIN  BMP", &file_len, &root_addr)) != NULL)
		{
			memcpy(str, p_file, 254);
			memset(gDisk_buff, 0x00, 0x2600);
			memcpy(ROOT_SECTOR + 32, "LOGOIN  BMP", 0xC);
			memcpy(FILE_SECTOR + 512, str, 254);
			gDisk_buff[0x40B + 32] = 0x0;
			*(uint32_t *)(VOLUME_BASE + 32) = VOLUME;
			gDisk_buff[0x41A + 32] = 0x03;
			gDisk_buff[0x41C + 32] = 254;
		}
		else
			memset(gDisk_buff, 0x00, 0x2600);

		memcpy(ROOT_SECTOR, "CONFIG  TXT", 12);
		memcpy(FAT1_SECTOR, gFat_data, 6);
		memcpy(FAT2_SECTOR, gFat_data, 6);

		m = 0;
		for (k = 0; k < CONFIG_CONT; k++)
		{
			memcpy(FILE_SECTOR + m, gDef_set[k],strlen((char *)gDef_set[k]));
			m += strlen((char *)gDef_set[k]);
			memcpy(FILE_SECTOR + m, gSet_range[k],strlen((char *)gSet_range[k]));
			m += strlen((char *)gSet_range[k]);
		}
		
		gDisk_buff[0x40B] = 0x0;	// Attributes
		*(uint32_t*)VOLUME_BASE = VOLUME;
		gDisk_buff[0x41A] = 0x02;	// Cluster number
		gDisk_buff[0x41C] = m;		// File size
		ReWrite_All();
	}

	gVar[F_TYPE] = HEX;
	gVar[F_FLAG] = RDY;
	gVar[SEG_ST] = 0;
	gV32[OFFSET] = 0;
	gV32[COUNT]	 = 0;
	gV32[WR_CNT] = 0;
	gV32[RD_CNT] = 0;
	return 0;
}

/*******************************************************************************
@@name  Disk_SecWrite
@@brief PC Write a file when writing a disk
@@param	pbuffer = saved content
		diskaddr = address
@@return NULL
*******************************************************************************/
void Disk_SecWrite(uint8_t* pbuffer, uint32_t diskaddr)//PC Save the data call
{
	static uint16_t Config_flag = 0;
	static uint8_t txt_flag = 0;

	uint8_t i;
	uint8_t ver[20];
	uint8_t config_filesize = 0;
	
	if (diskaddr == 0x1000)
	{	// Write FAT1 sector
		if (memcmp(pbuffer, (uint8_t *)FAT1_SECTOR, 512))
			memcpy((uint8_t *)FAT1_SECTOR, pbuffer, 512);
	}
	else if (diskaddr == 0x2800)
	{	// Write FAT2 sector
		if (memcmp(pbuffer,(uint8_t *)FAT2_SECTOR, 512))
			memcpy((uint8_t*)FAT2_SECTOR, pbuffer, 512);
	}
	else if (diskaddr == 0x4000)
	{	// Write DIR sector
		if (memcmp(pbuffer,(uint8_t *)ROOT_SECTOR, 512))
		{
			memcpy((uint8_t*)ROOT_SECTOR, pbuffer, 512);
			gRewriteflag[1] = 1;
			for (i = 0;i < 16;i++)
			{
				memcpy((uint8_t *)ver, (uint8_t *)(pbuffer), 12);
				if (memcmp(ver,"CONFIG  TXT", 11) == 0)
				{
					Config_flag = pbuffer[0x1A];
					config_filesize = pbuffer[0x1C];
					txt_flag = 1;
					break;
				}
				pbuffer += 32;
			}
			if (config_filesize == 0 && txt_flag == 1)
			{
				txt_flag = 0;
				gRewriteflag[1] = 0;
				gRewriteflag[0] = 0;
			}
			else
				gRewriteflag[0] = 1;
		}
	}
	else if (diskaddr >= 0x8000 && diskaddr <= 0xA000)
	{	// Write FILE sector
		if (memcmp(pbuffer,(uint8_t*)(FILE_SECTOR + (diskaddr - 0x8000)), 512))
			memcpy((uint8_t*)(FILE_SECTOR + (diskaddr - 0x8000)), pbuffer, 512);

		if ((((diskaddr - 0x8000) / 0x200) + 2) == Config_flag)
		{	// Cluster number
			if ( ! Setting_Analysis((uint8_t *)(FILE_SECTOR + (diskaddr - 0x8000)), (Config_flag - 2)))
				ReWriteFlash();
			return ;
		}
		else
			gRewriteflag[((diskaddr - 0x8000 + 0x200) / 0x400) + 1] = 1;
	}
	ReWriteFlash();
}

/*******************************************************************************
@@name  Disk_SecRead
@@brief PC Write the file when reading the disk
@@param pbuffer Read the contents of the diskaddr address
@@return NULL
*******************************************************************************/
void Disk_SecRead(uint8_t* pbuffer, uint32_t disk_addr)
{
	uint8_t * src;

	Soft_Delay();

	if (disk_addr == 0x0000)
		src = (uint8_t *)BOOT_SEC;		// Read BOOT sector
	else if (disk_addr == 0x1000)
		src = (uint8_t *)FAT1_SECTOR;	// Read FAT1 sector
	else if (disk_addr == 0x2800)
		src = (uint8_t *)FAT2_SECTOR;	// Read FAT2 sector
	else if (disk_addr == 0x4000)
		src = (uint8_t *)ROOT_SECTOR;	// Read DIR sector
	else if (disk_addr >= 0x8000 && disk_addr <= 0xA000)
		// Read FILE sector
		src = (uint8_t *)(APP_BASE + 0x600 + (disk_addr - 0x8000));
	else
	{
		memset(pbuffer, 0, 512);
		return;
	}
	memcpy(pbuffer, src, 512);
}
/*******************************************************************************

*******************************************************************************/
uint8_t ReWriteFlash(void)
{
	uint32_t i, j;
	uint8_t result;
	uint16_t *f_buff;
	
	FLASH_Unlock();
	for (i = 0; i < 16; i++)
	{
		if (gRewriteflag[i])
		{
			gRewriteflag[i] = 0;
			FLASH_Erase(APP_BASE + i * 0x400);
			f_buff = (u16*)&gDisk_buff[i * 0x400];
			for (j = 0; j < 0x400; j += 2)
			{
				result = FLASH_Prog((uint32_t)(APP_BASE + i*0x400 + j), *f_buff++);
				if (result != FLASH_COMPLETE)
				{
					FLASH_Lock();
					return ERR;
				}
			}
			break;
		}
	}
	FLASH_Lock();
	return RDY;
}
/*******************************************************************************

*******************************************************************************/
uint8_t ReWrite_All(void)
{
	uint16_t i;
	uint8_t result;
	uint16_t *f_buff = (uint16_t *)gDisk_buff;

	FLASH_Unlock();
	for (i = 0; i < 9; i++)
		FLASH_Erase(APP_BASE + i * 0x400);
	for (i = 0; i < 0x2600; i += 2)
	{
		result = FLASH_Prog((uint32_t)(APP_BASE + i), *f_buff++);
		if (result != FLASH_COMPLETE)
			return ERR;
	}
	FLASH_Lock();
	return RDY;
}

/*******************************************************************************

*******************************************************************************/
void Erase(void)
{
	uint16_t i;
	FLASH_Unlock();
	for (i = 0; i < 9; i++)
		FLASH_Erase(APP_BASE + i * 0x400);
	FLASH_Lock();
}
/*******************************************************************************

*******************************************************************************/
void Read_Memory(uint32_t r_offset, uint32_t r_length)
{
	static uint32_t offset, length, block_offset;

	if (gVar[USB_ST] == TXFR_IDLE )
	{
		offset = r_offset * SECTOR_SIZE;
		length = r_length * SECTOR_SIZE;
		gVar[USB_ST] = TXFR_ONGOING;
	}
	if (gVar[USB_ST] == TXFR_ONGOING )
	{
		if (!gV32[RD_CNT])
		{
			Disk_SecRead(gBuff, offset);
			UserToPMABufferCopy(gBuff, ENDP1_TXADDR, BULK_MAX_PACKET_SIZE);
			gV32[RD_CNT] = SECTOR_SIZE - BULK_MAX_PACKET_SIZE;
			block_offset = BULK_MAX_PACKET_SIZE;
		}
		else
		{
			UserToPMABufferCopy(gBuff + block_offset, ENDP1_TXADDR, BULK_MAX_PACKET_SIZE);
			gV32[RD_CNT]  -= BULK_MAX_PACKET_SIZE;
			block_offset += BULK_MAX_PACKET_SIZE;
		}
		SetEPTxCount(ENDP1, BULK_MAX_PACKET_SIZE);
		SetEPTxStatus(ENDP1, EP_TX_VALID);
		offset += BULK_MAX_PACKET_SIZE;
		length -= BULK_MAX_PACKET_SIZE;

		CSW.dDataResidue -= BULK_MAX_PACKET_SIZE;
	}
	if (length == 0)
	{
		gV32[RD_CNT] = 0;
		block_offset = 0;
		offset = 0;
		Bot_State = BOT_DATA_IN_LAST;
		gVar[USB_ST] = TXFR_IDLE;
	}
}
/*******************************************************************************

*******************************************************************************/
void Write_Memory (uint32_t w_offset, uint32_t w_length)
{
	static uint32_t offset, length;
	uint32_t idx, temp =  gV32[WR_CNT] + 64;

	if (gVar[USB_ST] == TXFR_IDLE )
	{
		offset = w_offset * SECTOR_SIZE;
		length = w_length * SECTOR_SIZE;
		gVar[USB_ST] = TXFR_ONGOING;
	}
	if (gVar[USB_ST] == TXFR_ONGOING )
	{
		for (idx = 0 ; gV32[WR_CNT] < temp; gV32[WR_CNT]++)
			*(uint8_t *)(gBuff + gV32[WR_CNT]) = Bulk_Buff[idx++];

		offset += Data_Len;
		length -= Data_Len;

		if (!(length % SECTOR_SIZE))
		{
			gV32[WR_CNT] = 0;
			Disk_SecWrite(gBuff, offset - SECTOR_SIZE);
		}

		CSW.dDataResidue -= Data_Len;
		SetEPRxStatus(ENDP2, EP_RX_VALID); /* enable the next transaction*/
	}
	if (length == 0 || Bot_State == BOT_CSW_Send)
	{
		gV32[WR_CNT] = 0;
		Set_CSW (CSW_CMD_PASSED, SEND_CSW_ENABLE);
		gVar[USB_ST] = TXFR_IDLE;
	}
}

/*******************************************************************************

*******************************************************************************/
void Restore_Setting(void)
{
	uint8_t buf[10] = {0};// The displayed string
 
	Set_TemperatureShowFlag(0);
	device_info.wait_time = 18000;
	device_info.idle_time = 36000;
	device_info.t_step = 100;
	Set_gTurn_offv(100);
	device_info.handers = 0;
	device_info.t_work = 3000;
	device_info.t_standby = 2000;
	
	Set_gCalib_flag(1);
	Disk_BuffInit();
	Config_Analysis();
	Set_gCalib_flag(0);
	own_sprintf((char *)buf,"Initial success!");
	Delay_Ms(500); 
	Clear_Watchdog();
	Display_Str6(0,(char *)buf);
	Delay_Ms(1000);
	Clear_Watchdog(); 
}
