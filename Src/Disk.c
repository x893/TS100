/********************* (C) COPYRIGHT 2015 e-Design Co.,Ltd. **********************
File Name :      Disk.c
Version :        S100 APP Ver 2.11   
Description:
Author :         Celery
Data:            2015/07/07
History:
2015/07/07   ͳһ������
*******************************************************************************/
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

#define Delay_mS    Delay_Ms
void Disk_SecWrite(u8* pBuffer, u32 DiskAddr);
void Disk_SecRead (u8* pBuffer, u32 DiskAddr);
void Soft_Delay(void);

uc8  BOOT_SEC[512] = {0xEB, 0x3C, 0x90, 0x4D, 0x53, 0x44, 0x4F, 0x53,
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

static u8   gDisk_buff[0x2600];
static u32  gDisk_var[(512 + 32 + 28)/4];     // ���̹���������
static u32 *gV32  = (u32*)&gDisk_var[512/4];
static u8  *gVar  = (u8*) &gDisk_var[512/4 + 8];

static u8  *gBuff = (u8*) &gDisk_var[0];
const u8   gFat_data[]= {0xF8,0xFF,0xFF,0xFF,0xFF,0xFF};//{0xF8,0XFF,0XFF,0xff,0X0f};//

const char *gKey_words[] = {"StbTemp","WkTemp","SlpTime","IdlTime","TempStp","OffVolt","Temp","Hand","ZeroP_Ad"};
const char *gDef_set[]   = {"StbTemp=200","WkTemp=300","SlpTime=180", "IdlTime=360","TempStp=10","OffVolt=10",
                            "Temp=0","Hand=0","ZeroP_Ad=239"};
const char *gSet_range[] = {"   #(100~400)\r\n","    #(100~400)\r\n","   #(60~999)\r\n","   #(100~999)\r\n",
                           "    #(1~25)\r\n","    #(5~19)\r\n","        #(0,1)\r\n","        #(0,1)\r\n","  #ReadOnly\r\n"};

static u8 gFile_con[512];
#define CONFIG_CONT 9
u8 gRewriteflag[16];

extern u8 gTemperatureshowflag;

#define ROW_CONT 35
#define FILE_CONT 254
/*******************************************************************************
������: Soft_Delay
��������:��ʱ
�������:NULL
���ز���:NULL
*******************************************************************************/
void Soft_Delay()
{
    int i,j;
    for(i = 0 ; i < 1000 ; i++)
        for(j = 0; j < 100; j++);
}
/*******************************************************************************
������: Set_Ver
��������:���ò���ֵ
�������:str ������k ����ָʾ
���ز���:NULL
*******************************************************************************/
void Set_Ver(u8  str[],u8 k)
{
    s16 set_ver;

    switch(k) {
    case 0:
        set_ver = (str[0] - 48) * 100 + (str[1] - 48) * 10 + (str[2] - 48);
        if(!gCalib_flag)    device_info.t_standby = set_ver * 10;
        break;
    case 1:
        set_ver = (str[0] - 48) * 100 + (str[1] - 48) * 10 + (str[2] - 48);
        if(!gCalib_flag)    device_info.t_work = set_ver * 10;
        break;
    case 2:
        if(str[3] <= '9' && str[3] >= '0') { //4λ��
            set_ver = (str[0] - 48) * 1000 + (str[1] - 48) * 100 + (str[2] - 48) * 10 + (str[3] - 48);
        } else if(str[2] <= '9' && str[2] >= '0') { //3λ��
            set_ver = (str[0] - 48) * 100 + (str[1] - 48) * 10 + (str[2] - 48);
        } else if(str[1] <= '9' && str[1] >= '0') { //2λ��
            set_ver = (str[0] - 48) * 10 + (str[1] - 48);
        }
        if(!gCalib_flag)    device_info.wait_time = set_ver * 100;
        break;
    case 3:
        if(str[3] <= '9' && str[3] >= '0') { //4λ��
            set_ver = (str[0] - 48) * 1000 + (str[1] - 48) * 100 + (str[2] - 48) * 10 + (str[3] - 48);
        } else if(str[2] <= '9' && str[2] >= '0') { //3λ��
            set_ver = (str[0] - 48) * 100 + (str[1] - 48) * 10 + (str[2] - 48);
        }
        if(!gCalib_flag)    device_info.idle_time = set_ver * 100;
        break;
    case 4:
        if(str[1] <= '9' && str[1] >= '0') { //2λ��
            set_ver = (str[0] - 48) * 10 + (str[1] - 48);
        } else {
            set_ver = str[0] - 48;
        }
        if(!gCalib_flag)    device_info.t_step = set_ver * 10;
        break;
    case 5:
        if(str[3] <= '9' && str[3] >= '0') { //4λ��
            set_ver = (str[0] - 48) * 1000 + (str[1] - 48) * 100 + (str[2] - 48) * 10 + (str[3] - 48);
        } else if(str[2] <= '9' && str[2] >= '0') { //3λ��
            set_ver = (str[0] - 48) * 100 + (str[1] - 48) * 10 + (str[2] - 48);
        } else if(str[1] <= '9' && str[1] >= '0') { //2λ��
            set_ver = (str[0] - 48) * 10 + (str[1] - 48);
        } else {
            set_ver = str[0] - 48;
        }
        if(!gCalib_flag)    gTurn_offv = set_ver * 10;
        break;
    case 6:
        set_ver = str[0] - 48;
        if(!gCalib_flag)    Set_TemperatureShowFlag(set_ver);
        break;
   case 7:
        set_ver = str[0] - 48;        
        if(!gCalib_flag)    device_info.handers = set_ver;
        break;    
    case 8:
        if(str[2] <= '9' && str[2] >= '0') { //3λ��
            set_ver = (str[0] - 48) * 100 + (str[1] - 48) * 10 + (str[2] - 48);
        } else if(str[1] <= '9' && str[1] >= '0') { //2λ��
            set_ver = (str[0] - 48) * 10 + (str[1] - 48);
        } else {
            set_ver = str[0] - 48;
        }
        if(!gCalib_flag)    gZerop_ad = set_ver;
        
        break;
    default:
        break;
    }
}
/*******************************************************************************
������: Cal_Val
��������:��������Ϸ���
�������:str ������k ����ָʾ flag ���У׼AD��־
���ز���:����0�Ƿ��� 1�Ϸ�
*******************************************************************************/
u8 Cal_Val(u8  str[],u8 k,u8 flag)
{
    u16 set_ver;
    
    switch(k) {
    case 0:
    case 1:
        if(gTemperatureshowflag == 0)//���϶�
        {
            if(str[2] > '9' || str[2] < '0' ||  //��λû��
               str[1] > '9' || str[1] < '0' ||  //ʮλû��
               str[0] > '4' ||                  //��λ����4    С��0
              (str[0] == '4' && (str[1] != '0' || str[2] != '0'))) 
                  return 0;
        }
        else//���϶�
        {
           if(str[2] > '9' || str[2] < '0' ||  //��λû��
              str[1] > '9' || str[1] < '0' ||  //ʮλû��
              str[0] > '8' || str[0] < '2' ||  //��λ����4    С��0
             (str[0] == '7' && str[1] > '5') ||
             (str[0] == '7' && str[1] == '5' && str[2]> '2') || 
             (str[0] == '2' && str[1] < '1') ||  
             (str[0] == '2' && str[1] == '1' && str[2]> '2')) 
                  return 0;
        }
        break;
    case 2:
        if(str[3] <= '9' && str[3] >= '0') {//4λ��
            if((str[2] > '9' && str[2] < '0') || (str[1] > '9' && str[1] < '0'))//ʮλ��λΪ��
                return 0;
        } else if(str[2] <= '9' && str[2] >= '0') { //3λ��
            if(str[1] > '9' && str[1] < '0')//ʮλΪ��
                return 0;
        } else if(str[1] <= '9' && str[1] >= '0'){//��λ��
            if(str[0] >'9' || str[0] < '6')//��λΪ�� С��60
                return 0;
        } else {
            return 0;
        }
        break;
    case 3:
        if(str[3] <= '9' && str[3] >= '0') {//4λ��
            if((str[2] > '9' && str[2] < '0') || (str[1] > '9' && str[1] < '0'))//ʮλ��λΪ��
                return 0;
        } else if(str[2] <= '9' && str[2] >= '0') { //3λ��
            if(str[0] >'9' || str[0] < '1')//ʮλΪ�� С��300
                return 0;
        } else {//��λ��
            return 0;
        }
        break;
    case 4://T_Step=10       #(5~25)
        if(str[1] <= '5' && str[1] >= '0'){//��λ��
            if(str[0] >'2' || str[0] < '0')//��λΪ�� С��60
                return 0;
        } else {
            if(str[0] < '0' ){
                 return 0;
            }
        }
        break;
    case 5://Turn_Off_v=10   #(5~19)
          if(str[1] <= '2' && str[1] >= '0'){//��λ��
              if(str[0] >'9' || str[0] < '0')//��λΪ�� С��60
                  return 0;
          } else {
              if(str[0] < '5'){
                   return 0;
              }
          }
        break;
    case 6://TempShowFlag=0  #(0,1)
        if(str[0] != '1' && str[0] != '0')
          return 0;
        break;
    case 7://handers=0  #(0,1)
        if(str[0] != '1' && str[0] != '0')
          return 0;
        break;
    case 8:
        if(str[2] <= '9' && str[2] >= '0') { //3λ��
            set_ver = (str[0] - 48) * 100 + (str[1] - 48) * 10 + (str[2] - 48);
        }else if(str[1] <= '9' && str[1] >= '0') { //2λ��
            set_ver = (str[0] - 48) * 10 + (str[1] - 48);
        } else {
            set_ver = str[0] - 48;
        }
        if(flag == 1){
            if(set_ver != gZerop_ad) return 0;
        }
        break;
    default:
        break;
    }
    return 1;
}
/*******************************************************************************
������: Disk_BuffInit
��������:�������ݳ�ʼ��
�������:NULL
���ز���:NULL
*******************************************************************************/
void Disk_BuffInit(void)
{
    memcpy(gDisk_buff, (u8*)APP_BASE,0x2600);
    memset(gRewriteflag,0,16);
}
/*******************************************************************************
������: Upper
��������:СдתΪ��д
�������:str��Ҫת�����ַ�����len �ַ�������
���ز���:NULL
*******************************************************************************/
void Upper(u8* str, u16 len)
{
    u16 i;
    for(i = 0; i < len; i++)
        if(str[i] >= 'a' && str[i] <= 'z')
            str[i] -= 32;
}
/*******************************************************************************
������: SearchFile
��������:�����ļ�
�������:pfilename �ļ�����pfilelen�ļ����ȣ�root_addr ���ҿ�ʼ��ַ
���ز���:�ļ����ڵ�ַ
*******************************************************************************/
u8* SearchFile(u8* pfilename, u16* pfilelen,u16* root_addr)
{
    u16 n,sector;
    u8  str_name[11];
    u8* pdiraddr;
    
    pdiraddr = ROOT_SECTOR;

    for(n = 0; n < 16; n++) {
        memcpy(str_name,pdiraddr,11);
        Upper(str_name,11);
        if(memcmp(str_name,pfilename,11) == 0) {
            memcpy((u8*)pfilelen,pdiraddr + 0x1C,2);
            memcpy((u8*)&sector,pdiraddr + 0x1A,2);
            return (u8*)FILE_SECTOR + (sector - 2) * 512;
        }
        
        pdiraddr += 32;
        root_addr++;
    }
    return NULL;
}

const u8 LOGO[] = {
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

u8 Setting_Analysis(u8* p_file , u16 root_addr)
{
    u32 i,j,k,m,flag;
    u8 t_p[CONFIG_CONT][ROW_CONT];
    u8 str[FILE_CONT];
    u8 is_illegality = 0;
    
    m = 0;
    j = 0;

    memset(t_p, 0x00, CONFIG_CONT * ROW_CONT);
    memcpy((u8*)gFile_con,p_file,512);
    for(k = 0; k < CONFIG_CONT; k++) { //ȡ��CONFIG_CONT   ��
        j = 0;
        for(i = m; i < strlen((char *)gFile_con); i++) { //����������ֵ�ַ�������λ��
            if(gFile_con[i] == 0x0D && gFile_con[i + 1] == 0x0A) break;
            else {
                if(j < ROW_CONT)  t_p[k][j++] = gFile_con[i];
                m++;
            }
        }
        t_p[k][j] = '\0';
        m = i + 2;
    }
    for(k = 0; k < CONFIG_CONT; k++) { //����CONFIG_CONT   ��
        if(memcmp(t_p[k],gKey_words[k],strlen(gKey_words[k])) == 0) { //�ҵ��ؼ���
            flag = 0;
            for(i = strlen(gKey_words[k]); i < strlen((char *)t_p[k]); i++) { //����ֵ�Ƿ�Ϸ�
                 if(t_p[k][i] >= '0' && t_p[k][i] <= '9') {
                    if(t_p[k][i] == '0') {
                        if(k == 6||k == 7){
                            flag = 1;
                            break;
                        }else {
                            flag = 0;
                            break;
                        }
                    }
                    flag = 1;
                    break;
                } else if((t_p[k][i] != 0x20) && (t_p[k][i] != 0x3d)) {//�ո�͵Ⱥ�
                    flag = 0;
                    break;
                }
            }
            if(flag && Cal_Val(t_p[k] + i,k,0)) { //����ֵ�Ϸ�
                Set_Ver(t_p[k] + i,k);
                if(k == 0)        own_sprintf((char *)t_p[k],"StbTemp=%d",device_info.t_standby/10);
                else if (k == 1)  own_sprintf((char *)t_p[k],"WkTemp=%d", device_info.t_work/10);
                else if (k == 2)  own_sprintf((char *)t_p[k],"SlpTime=%d",device_info.wait_time/100);
                else if (k == 3)  own_sprintf((char *)t_p[k],"IdlTime=%d",device_info.idle_time/100);
                else if (k == 4)  own_sprintf((char *)t_p[k],"TempStp=%d",device_info.t_step/10);
                else if (k == 5)  own_sprintf((char *)t_p[k],"OffVolt=%d",gTurn_offv/10);
                else if (k == 6)  own_sprintf((char *)t_p[k],"Temp=%d",Get_TemperatureShowFlag());
                else if (k == 7)  own_sprintf((char *)t_p[k],"Hand=%d",device_info.handers);
                else if (k == 8)  own_sprintf((char *)t_p[k],"ZeroP_Ad=%d",gZerop_ad);
            } else {//����ֵ���Ϸ�
                memset(t_p[k],0,strlen((char *)t_p[k]));
                memcpy(t_p[k],gDef_set[k],strlen((char *)gDef_set[k]));
                is_illegality = 1;
            }
        } else {//ľ���ҵ��ؼ���
            memset(t_p[k],0,strlen((char *)t_p[k]));
            memcpy(t_p[k],gDef_set[k],strlen((char *)gDef_set[k]));
            is_illegality = 1;
        }
    }
        
//        if(is_illegality || gCalib_flag){
        memset(str, 0x00, FILE_CONT);
        m = 0;
        for(k = 0; k < CONFIG_CONT; k++) {
            strcat((char *)str,(char *)t_p[k]);
            strcat((char *)str,(char *)gSet_range[k]);
        }
        m = strlen((char *)str);
        gDisk_buff[0x400 + root_addr*32 + 0x1C] = m % 256;
        gDisk_buff[0x400 + root_addr*32 + 0x1D] = m / 256;
    
        gRewriteflag[(p_file - ROOT_SECTOR + 0x200) / 0x400] = 1;
        memcpy(p_file, str,strlen((char *)str));
//            ReWriteFlsash();
//        } 
        return is_illegality;

}


/*******************************************************************************
������: Config_Analysis
��������:�����ļ�����
�������:NULL
���ز���:NULL
*******************************************************************************/
u8 Config_Analysis(void)
{
    u32 k,m;
    u8 str[FILE_CONT];
    u8 is_illegality;
    u16 file_len;
    u8* p_file;
    u16 root_addr;
    
    root_addr = 0;
    
    if(p_file = SearchFile("CONFIG  TXT",&file_len,&root_addr)){
        is_illegality = Setting_Analysis(p_file,root_addr);
        if(is_illegality || gCalib_flag){
            ReWriteFlsash();
        }
    } else {
        if(p_file = SearchFile("LOGOIN  BMP",&file_len,&root_addr)){
            memcpy(str,p_file,254);
            memset(gDisk_buff, 0x00, 0x2600);
            memcpy(ROOT_SECTOR + 32, "LOGOIN  BMP",0xC);
            memcpy(FILE_SECTOR + 512, str,254);
            gDisk_buff[0x40B + 32] = 0x0; //����
            *(u32*)(VOLUME_BASE + 32) = VOLUME;
            gDisk_buff[0x41A + 32] = 0x03;//�غ�
            gDisk_buff[0x41C + 32] = 254;//�ļ���С
        } else {
            memset(gDisk_buff, 0x00, 0x2600);
        }

        memcpy(ROOT_SECTOR, "CONFIG  TXT",0xC);
        memcpy(FAT1_SECTOR, gFat_data,6);
        memcpy(FAT2_SECTOR, gFat_data,6);

        m = 0;
        for(k = 0; k < CONFIG_CONT; k++) {
            memcpy(FILE_SECTOR + m, gDef_set[k],strlen((char *)gDef_set[k]));
            m += strlen((char *)gDef_set[k]);
            memcpy(FILE_SECTOR + m, gSet_range[k],strlen((char *)gSet_range[k]));
            m += strlen((char *)gSet_range[k]);
        }
        
        gDisk_buff[0x40B] = 0x0; //����
        *(u32*)VOLUME_BASE = VOLUME;
        gDisk_buff[0x41A] = 0x02;//�غ�
        gDisk_buff[0x41C] = m;//�ļ���С
        ReWrite_All();
    }   

    gVar[F_TYPE]   = HEX;
    gVar[F_FLAG]   = RDY;
    gVar[SEG_ST]   = 0;
    gV32[OFFSET]   = 0;
    gV32[COUNT]    = 0;
    gV32[WR_CNT]   = 0;
    gV32[RD_CNT]   = 0;
    return 0;
}
/*******************************************************************************
������: Disk_SecWrite
��������:PC �����ļ�ʱд����
�������:pbuffer �������� diskaddr ��ַ
���ز���:NULL
*******************************************************************************/
void Disk_SecWrite(u8* pbuffer, u32 diskaddr)//PC �������ݵ���
{
    u8 i;
    u8 is_illegality = 0;
    u8 ver[20];
    static u16 Config_flag = 0;
    static u8 txt_flag = 0;
    u8 config_filesize = 0;
    
    if(diskaddr == 0x1000) {                               // Write FAT1 sector
        if(memcmp(pbuffer,(u8*)FAT1_SECTOR, 512)) {
            memcpy((u8*)FAT1_SECTOR, pbuffer, 512);
        }
    } else if(diskaddr == 0x2800) {                        // Write FAT2 sector
        if(memcmp(pbuffer,(u8*)FAT2_SECTOR, 512)) {
            memcpy((u8*)FAT2_SECTOR, pbuffer, 512);
        }
    } else if(diskaddr == 0x4000) {                        // Write DIR sector
        if(memcmp(pbuffer,(u8*)ROOT_SECTOR, 512)) {
            memcpy((u8*)ROOT_SECTOR, pbuffer, 512);
            gRewriteflag[1] = 1;
            for(i = 0;i < 16;i++){
                memcpy((u8*)ver,(u8*)(pbuffer),12);
                if(memcmp(ver,"CONFIG  TXT",11) == 0){
                    Config_flag = pbuffer[0x1A];
                    config_filesize = pbuffer[0x1C];
                    txt_flag = 1;
                    break;
                }
                pbuffer += 32;
            }
            if(config_filesize == 0 && txt_flag == 1){
                txt_flag = 0;
                gRewriteflag[1] = 0;
                gRewriteflag[0] = 0;
            }else {
                gRewriteflag[0] = 1;
            }
        }
    } else if(diskaddr >= 0x8000 && diskaddr <= 0xA000) {                        // Write FILE sector
        if(memcmp(pbuffer,(u8*)(FILE_SECTOR + (diskaddr - 0x8000)), 512)) {
            memcpy((u8*)(FILE_SECTOR + (diskaddr - 0x8000)), pbuffer, 512);
        }
        if((((diskaddr - 0x8000)/0x200) + 2) == Config_flag){//�غ�
            is_illegality = Setting_Analysis((u8*)(FILE_SECTOR + (diskaddr - 0x8000)),(Config_flag - 2));
            if(!is_illegality) {
                ReWriteFlsash();
                return ;
            }else{
                return ;
            }
        }else{
            gRewriteflag[((diskaddr - 0x8000 + 0x200) / 0x400) + 1] = 1;
        }
    }
    ReWriteFlsash();
}

/*******************************************************************************
������: Disk_SecRead
��������:PC ��ȡ�ļ�ʱд����
�������:pbuffer ��ȡ���� diskaddr ��ַ
���ز���:NULL
*******************************************************************************/
void Disk_SecRead(u8* pbuffer, u32 disk_addr)
{
    Soft_Delay();
    if(disk_addr == 0x0000) {                               // Read BOOT sector
        memcpy(pbuffer, BOOT_SEC, 512);
    } else if(disk_addr == 0x1000) {                        // Read FAT1 sector
        memcpy(pbuffer, FAT1_SECTOR, 512);
    } else if(disk_addr == 0x2800) {                        // Read FAT2 sector
        memcpy(pbuffer, FAT2_SECTOR, 512);
    } else if(disk_addr == 0x4000) {                        // Read DIR sector
        memcpy(pbuffer, (u8*)(ROOT_SECTOR), 512);
    } else if(disk_addr >= 0x8000 && disk_addr <= 0xA000) {                        // Read FILE sector
        memcpy(pbuffer, (u8*)(APP_BASE + 0x600 + (disk_addr - 0x8000)), 512);
    } else {
        memset(pbuffer, 0, 512);//
    }
}
/*******************************************************************************
������: ReWriteFlsash
��������:дFlash
�������:NULL
���ز���:�Ƿ�д���־
*******************************************************************************/
u8 ReWriteFlsash(void)
{
    u32 i,j;
    u8  result;
    u16 *f_buff;
    
    FLASH_Unlock();
    for(i = 0; i < 16; i++) {
        if(gRewriteflag[i]) {
            gRewriteflag[i] = 0;
            FLASH_Erase(APP_BASE + i * 0x400);
            f_buff = (u16*)&gDisk_buff[i * 0x400];
            for(j = 0; j < 0x400; j += 2) {
                result = FLASH_Prog((u32)(APP_BASE + i*0x400 + j),*f_buff++);
                if(result != FLASH_COMPLETE){
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
������: ReWrite_All
��������:��������flash����д����
�������:NULL
���ز���:�Ƿ�д���־
*******************************************************************************/
u8 ReWrite_All(void)
{
    u16 i;
    u8  result;
    u16 *f_buff = (u16*)gDisk_buff;

    FLASH_Unlock();
    for(i = 0; i < 9; i++)FLASH_Erase(APP_BASE + i*0x400);
    for(i = 0; i < 0X2600; i += 2) {
        result = FLASH_Prog((u32)(APP_BASE + i),*f_buff++);
        if(result != FLASH_COMPLETE)  return ERR;
    }
    FLASH_Lock();
    return RDY;
}
/*******************************************************************************
������: Erase
��������:��ʽ��Flash
�������:NULL
���ز���:NULL
*******************************************************************************/
void Erase(void)
{
    u16 i;
    FLASH_Unlock();
    for(i = 0; i < 9; i++)FLASH_Erase(APP_BASE + i*0x400);

    FLASH_Lock();
}
/*******************************************************************************
������: Read_Memory
��������:����microSD���Ķ�������
�������:r_offset ƫ�� r_length����
���ز���:NULL
*******************************************************************************/
void Read_Memory(u32 r_offset, u32 r_length)
{
    static u32 offset, length, block_offset;

    if (gVar[USB_ST] == TXFR_IDLE ) {
        offset = r_offset * SECTOR_SIZE;
        length = r_length * SECTOR_SIZE;
        gVar[USB_ST] = TXFR_ONGOING;
    }
    if (gVar[USB_ST] == TXFR_ONGOING ) {
        if (!gV32[RD_CNT]) {
            Disk_SecRead(gBuff, offset);
            UserToPMABufferCopy(gBuff, ENDP1_TXADDR, BULK_MAX_PACKET_SIZE);
            gV32[RD_CNT] = SECTOR_SIZE - BULK_MAX_PACKET_SIZE;
            block_offset = BULK_MAX_PACKET_SIZE;
        } else {
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
    if (length == 0) {
        gV32[RD_CNT] = 0;
        block_offset = 0;
        offset = 0;
        Bot_State = BOT_DATA_IN_LAST;
        gVar[USB_ST] = TXFR_IDLE;
    }
}
/*******************************************************************************
������: Write_Memory
��������:����microSD����д������
�������:r_offset ƫ�� r_length����
���ز���:NULL
*******************************************************************************/
void Write_Memory (u32 w_offset, u32 w_length)
{
    static u32 offset, length;
    u32 idx, temp =  gV32[WR_CNT] + 64;

    if (gVar[USB_ST] == TXFR_IDLE ) {
        offset = w_offset * SECTOR_SIZE;
        length = w_length * SECTOR_SIZE;
        gVar[USB_ST] = TXFR_ONGOING;
    }
    if (gVar[USB_ST] == TXFR_ONGOING ) {
        for (idx = 0 ; gV32[WR_CNT] < temp; gV32[WR_CNT]++)
            *(u8 *)(gBuff + gV32[WR_CNT]) = Bulk_Buff[idx++];

        offset += Data_Len;
        length -= Data_Len;

        if (!(length % SECTOR_SIZE)) {
            gV32[WR_CNT] = 0;
            Disk_SecWrite(gBuff, offset - SECTOR_SIZE);
        }

        CSW.dDataResidue -= Data_Len;
        SetEPRxStatus(ENDP2, EP_RX_VALID); /* enable the next transaction*/
    }
    if ((length == 0) || (Bot_State == BOT_CSW_Send)) {
        gV32[WR_CNT] = 0;
        Set_CSW (CSW_CMD_PASSED, SEND_CSW_ENABLE);
        gVar[USB_ST] = TXFR_IDLE;
    }
}

/*******************************************************************************
������: Restore_Setting 
��������:�ָ������豸
�������:NULL
���ز���:NULL
*******************************************************************************/
void Restore_Setting(void)
{
    u8 buf[10] = {0};//��ʾ���ַ���
 
    gTemperatureshowflag = 0;
    device_info.wait_time = 18000;
    device_info.idle_time = 36000;
    device_info.t_step = 100;
    gTurn_offv = 100;
    device_info.handers = 0;
    device_info.t_work = 3000;
    device_info.t_standby = 2000;
    
    gCalib_flag = 1;
    Disk_BuffInit();
    Config_Analysis();
    gCalib_flag = 0;
    own_sprintf((char *)buf,"Initial success!");
    Delay_Ms(500); 
    Clear_Watchdog();
    Display_Str6(0,(char *)buf);
    Delay_Ms(1000);
    Clear_Watchdog(); 
}


/*********************************  END OF FILE  ******************************/
