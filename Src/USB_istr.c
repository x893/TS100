#include "USB_type.h"
#include "USB_regs.h"
#include "USB_pwr.h"
#include "USB_istr.h"
#include "USB_init.h"
#include "USB_int.h"
#include "USB_bot.h"

volatile u16 wIstr;  /* ISTR register last read value */
volatile u8 bIntPackSOF = 0;  /* SOFs received between 2 consecutive packets */

void (*pEpInt_IN[7])(void) = {
	EP1_IN_Callback,
	EP2_IN_Callback,
	EP3_IN_Callback,
	EP4_IN_Callback,
	EP5_IN_Callback,
	EP6_IN_Callback,
	EP7_IN_Callback,
};
void (*pEpInt_OUT[7])(void) = {
	EP1_OUT_Callback,
	EP2_OUT_Callback,
	EP3_OUT_Callback,
	EP4_OUT_Callback,
	EP5_OUT_Callback,
	EP6_OUT_Callback,
	EP7_OUT_Callback,
};

/*******************************************************************************
  USB_Istr: ISTR events interrupt service routine
*******************************************************************************/
void USB_LP_CAN1_RX0_IRQHandler(void)
{
	wIstr = _GetISTR();

#if (IMR_MSK & ISTR_RESET)
	if (wIstr & ISTR_RESET & wInterrupt_Mask)
	{
		Device_Property.Reset();
		_SetISTR((u16)CLR_RESET);
	}
#endif

#if (IMR_MSK & ISTR_DOVR)
	if (wIstr & ISTR_DOVR & wInterrupt_Mask)
	{
		_SetISTR((u16)CLR_DOVR);
	}
#endif

#if (IMR_MSK & ISTR_ERR)
	if (wIstr & ISTR_ERR & wInterrupt_Mask)
	{
		_SetISTR((u16)CLR_ERR);
	}
#endif

#if (IMR_MSK & ISTR_WKUP)
	if (wIstr & ISTR_WKUP & wInterrupt_Mask)
	{
		Resume(RESUME_EXTERNAL);
		_SetISTR((u16)CLR_WKUP);
	}
#endif

#if (IMR_MSK & ISTR_SUSP)
	if (wIstr & ISTR_SUSP & wInterrupt_Mask)
	{ // check if SUSPEND is possible
		if (fSuspendEnabled)  Suspend();
		else				  Resume(RESUME_LATER); // if not possible then resume after xx ms
		_SetISTR((u16)CLR_SUSP); // clear of the ISTR bit must be done after setting of CNTR_FSUSP
	}
#endif

#if (IMR_MSK & ISTR_SOF)
	if (wIstr & ISTR_SOF & wInterrupt_Mask)
	{
		_SetISTR((u16)CLR_SOF);
		bIntPackSOF++;
	}
#endif

#if (IMR_MSK & ISTR_ESOF)
	if (wIstr & ISTR_ESOF & wInterrupt_Mask)
	{
		Resume(RESUME_ESOF);	  // request without change of the machine state
		_SetISTR((u16)CLR_ESOF);  // resume handling timing is made with ESOFs
	}
#endif

#if (IMR_MSK & ISTR_CTR)
	if (wIstr & ISTR_CTR & wInterrupt_Mask)
	{
		CTR_LP();
	}
#endif

}

/*******************************************************************************
  EP1_IN_Callback: EP1 IN Callback Routine
*******************************************************************************/
void EP1_IN_Callback(void)
{
	Mass_Storage_In();
}
/*******************************************************************************
  EP2_OUT_Callback: EP2 OUT Callback Routine.
*******************************************************************************/
void EP2_OUT_Callback(void)
{
	Mass_Storage_Out();
}
