/********************* (C) COPYRIGHT 2015 e-Design Co.,Ltd. **********************
File Name :      CTRL.h
Version :        S100 APP Ver 2.11   
Description:
Author :         Celery
Data:            2015/07/07
History:
2015/07/07   ͳһ������
*******************************************************************************/
#ifndef _CTRL_H
#define _CTRL_H

#include "STM32F10x.h"
#include "Bios.h"

#define TEMPSHOW_TIMER            gTime[0]/*TEMPSHOW_TIMER*/
#define HEATING_TIMER             gTime[1]/*HEATING_TIMER*/
#define ENTER_WAIT_TIMER          gTime[2]/*ENTER_WAIT_TIMER*/
#define EFFECTIVE_KEY_TIMER       gTime[3]/*EFFECTIVE_KEY_TIMER*/
#define LEAVE_WAIT_TIMER          gTime[4]/*LEAVE_WAIT_TIMER*/
#define G6_TIMER                  gTime[5]/*SWITCH_SHOW_TIMER*/
#define UI_TIMER                  gTime[6]/*UI_TIMER ͼ���������ʱ�����*/
#define KD_TIMER                  gTime[7]/*������ʱ*/
//------------------------------ ��������------------------------------------//
#define   KEY_ST(KEY_PIN)      GPIO_ReadInputDataBit(GPIOA, KEY_PIN)
#define   NO_KEY    0x0
#define   KEY_V1    0x0100
#define   KEY_V2    0x0040
#define   KEY_CN    0X8000
#define   KEY_V3    0X0140

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
    WKT = 0,    //t_standby
    SDT,        //t_work
    WTT,        //wait_time
    IDT,        //idle_time
    STP,        //t_step
    TOV,        //turn of V
    DGC,        //dgc
    HD,         //handers
    WDJ,       //�ض�Ӌ
    EXW         //EX WORKS
} SET_OPT;

typedef struct {
    u16 set_opt;        //ֵ
    u8 settings[10];    //����ѡ��
}SYS_SET;

typedef struct {
    u8 ver[16];             //�汾��
    int t_standby;          // 200��C=1800  2520,�����¶�
    int t_work;             // 350��C=3362, �����¶�
    int t_step;             //���ò���
    u32 wait_time;          //3*60*100   3 mintute
    u32 idle_time;          //6*60*100   6 minute 
    u8 handers;             //0 right 1 left
} DEVICE_INFO_SYS;

extern DEVICE_INFO_SYS device_info;
extern u8 gHanders;

void Set_PrevTemp(s16 Temp);
u8 Get_CtrlStatus(void);
void Set_CtrlStatus(u8 status);
s16 Get_TempVal(void);
u16 Get_HtFlag(void);
void System_Init(void);
void Pid_Init(void);
u16 Pid_Realize(s16 temp);
u32 Heating_Time(s16 temp,s16 wk_temp);
void Status_Tran(void);
#endif
/******************************** END OF FILE *********************************/
