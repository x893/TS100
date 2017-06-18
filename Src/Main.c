#include <string.h>
#include <stdio.h>
#include "APP_Version.h"
#include "Disk.h"
#include "Bios.h"
#include "USB_lib.h"
#include "I2C.h"
#include "Flash.h"
#include "MMA8652FC.h"
#include "UI.h"
#include "OLed.h"
#include "CTRL.h"
#include "HARDWARE.h"

/*******************************************************************************

*******************************************************************************/
int main(void)
{
	RCC_Config();
	GPIO_Config();
	Init_Timers();
	NVIC_Config(0x4000);

	USB_Port(DISABLE);
	Delay_Ms(200);
	USB_Port(ENABLE);
	USB_Init();

	Adc_Init();
	if (Get_CtrlStatus() != CONFIG)
		StartUp_Accelerated();

	System_Init();
	Disk_BuffInit();
	Config_Analysis();	// Start virtual U disk

	APP_Init();
	Init_Oled();
	Clear_Screen();

	Pid_Init();
	Set_gKey(NO_KEY);
	Start_Watchdog(3000);

	Set_Handers(device_info.handers);

	while (1)
	{
		Clear_Watchdog();
		if (Get_CtrlStatus() != CONFIG && LEAVE_WAIT_TIMER == 0)
		{
			Check_Accelerated();
			LEAVE_WAIT_TIMER = 50;
		}
		OLed_Display();	// Show Ctrl_status
		Status_Tran();	// According to the current state, with the key and control time conversion
	}
}
