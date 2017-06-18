#include "USB_desc.h"
#include "USB_pwr.h"
#include "USB_bot.h"
#include "USB_prop.h"
#include "USB_lib.h"
#include "Bios.h"
#include "APP_Version.h"

uint32_t Max_Lun = 0;

DEVICE Device_Table = { EP_NUM, 1 };
DEVICE_PROP Device_Property = {
	MASS_init,
	MASS_Reset,
	MASS_Status_In,
	MASS_Status_Out,
	MASS_Data_Setup,
	MASS_NoData_Setup,
	MASS_Get_Interface_Setting,
	MASS_GetDeviceDescriptor,
	MASS_GetConfigDescriptor,
	MASS_GetStringDescriptor,
	0,
	0x40 // MAX PACKET SIZE
};
USER_STANDARD_REQUESTS User_Standard_Requests = {
	Mass_Storage_GetConfiguration,
	Mass_Storage_SetConfiguration,
	Mass_Storage_GetInterface,
	Mass_Storage_SetInterface,
	Mass_Storage_GetStatus,
	Mass_Storage_ClearFeature,
	Mass_Storage_SetEndPointFeature,
	Mass_Storage_SetDeviceFeature,
	Mass_Storage_SetDeviceAddress
};
ONE_DESCRIPTOR Device_Descriptor = {
	(uint8_t*)MASS_DeviceDescriptor,
	MASS_SIZ_DEVICE_DESC
};
ONE_DESCRIPTOR Config_Descriptor = {
	(uint8_t*)MASS_ConfigDescriptor,
	MASS_SIZ_CONFIG_DESC
};
ONE_DESCRIPTOR String_Descriptor[5] = {
	{(uint8_t*)MASS_StringLangID, MASS_SIZ_STRING_LANGID},
	{(uint8_t*)MASS_StringVendor, MASS_SIZ_STRING_VENDOR},
	{(uint8_t*)MASS_StringProduct, MASS_SIZ_STRING_PRODUCT},
	{(uint8_t*)MASS_StringSerial, MASS_SIZ_STRING_SERIAL},
	{(uint8_t*)MASS_StringInterface, MASS_SIZ_STRING_INTERFACE},
};

/*******************************************************************************
  MASS_init: Mass Storage init routine.
*******************************************************************************/
void MASS_init()
{
	Get_SerialNum(); // Update the serial number string descriptor with the data from the unique ID
	pInformation->Current_Configuration = 0;
	PowerOn();	   // Connect the device
	_SetISTR(0);	 // USB interrupts initialization. clear pending interrupts
	wInterrupt_Mask = IMR_MSK;
	_SetCNTR(wInterrupt_Mask); // set interrupts mask
	bDeviceState = UNCONNECTED;
}
/*******************************************************************************
  MASS_Reset: Mass Storage reset routine.
*******************************************************************************/
void MASS_Reset()
{
	Device_Info.Current_Configuration = 0; // Set the device as not configured
	pInformation->Current_Feature = MASS_ConfigDescriptor[7]; // Current Feature initialization
	SetBTABLE(BTABLE_ADDRESS);

	// Initialize Endpoint 0
	SetEPType(ENDP0, EP_CONTROL);
	SetEPTxStatus(ENDP0, EP_TX_NAK);
	SetEPRxAddr(ENDP0, ENDP0_RXADDR);
	SetEPRxCount(ENDP0, Device_Property.MaxPacketSize);
	SetEPTxAddr(ENDP0, ENDP0_TXADDR);
	Clear_Status_Out(ENDP0);
	SetEPRxValid(ENDP0);

	//Initialize Endpoint 1
	SetEPType(ENDP1, EP_BULK);
	SetEPTxAddr(ENDP1, ENDP1_TXADDR);
	SetEPTxStatus(ENDP1, EP_TX_NAK);
	SetEPRxStatus(ENDP1, EP_RX_DIS);

	// Initialize Endpoint 2
	SetEPType(ENDP2, EP_BULK);
	SetEPRxAddr(ENDP2, ENDP2_RXADDR);
	SetEPRxCount(ENDP2, Device_Property.MaxPacketSize);
	SetEPRxStatus(ENDP2, EP_RX_VALID);
	SetEPTxStatus(ENDP2, EP_TX_DIS);

	SetEPRxCount(ENDP0, Device_Property.MaxPacketSize);
	SetEPRxValid(ENDP0);

	// Set the device to response on default address
	SetDeviceAddress(0);

	bDeviceState = ATTACHED;

	CBW.dSignature = BOT_CBW_SIGNATURE;
	Bot_State = BOT_IDLE;
}

/*******************************************************************************
  Mass_Storage_SetConfiguration: Handle the SetConfiguration request.
*******************************************************************************/
void Mass_Storage_SetConfiguration(void)
{
	if (pInformation->Current_Configuration != 0)
	{
		bDeviceState = CONFIGURED; // Device configured
		ClearDTOG_TX(ENDP1);
		ClearDTOG_RX(ENDP2);
		Bot_State = BOT_IDLE;	  // set the Bot state machine to the IDLE state
	}
}
/*******************************************************************************
  Mass_Storage_ClearFeature: Handle the ClearFeature request.
*******************************************************************************/
void Mass_Storage_ClearFeature(void)
{
	/* when the host send a CBW with invalid signature or invalid length the two
		Endpoints (IN & OUT) shall stall until receiving a Mass Storage Reset	*/
	if (CBW.dSignature != BOT_CBW_SIGNATURE)
		Bot_Abort(BOTH_DIR);
}
/*******************************************************************************
  Mass_Storage_SetConfiguration: Udpade the device state to addressed.
*******************************************************************************/
void Mass_Storage_SetDeviceAddress (void)
{
	bDeviceState = ADDRESSED;
}
/*******************************************************************************
  MASS_Status_In: Mass Storage Status IN routine.
*******************************************************************************/
void MASS_Status_In(void)
{
}
/*******************************************************************************
  MASS_Status_Out: Mass Storage Status OUT routine.
*******************************************************************************/
void MASS_Status_Out(void)
{
}
/*******************************************************************************
  MASS_Data_Setup: Handle the data class specific requests.
*******************************************************************************/
RESULT MASS_Data_Setup(uint8_t RequestNo)
{
	uint8_t * (*CopyRoutine)(u16);

	CopyRoutine = NULL;
	if ((Type_Recipient == (CLASS_REQUEST | INTERFACE_RECIPIENT))
	&&	(RequestNo == GET_MAX_LUN) && (pInformation->USBwValue == 0)
	&&	(pInformation->USBwIndex == 0) && (pInformation->USBwLength == 0x01)
		)
	{
		CopyRoutine = Get_Max_Lun;
	}
	else
		return USB_UNSUPPORT;
	if (CopyRoutine == NULL)
		return USB_UNSUPPORT;
	pInformation->Ctrl_Info.CopyData = CopyRoutine;
	pInformation->Ctrl_Info.Usb_wOffset = 0;
	(*CopyRoutine)(0);
	return USB_SUCCESS;
}
/*******************************************************************************
  MASS_NoData_Setup: Handle the no data class specific requests.
*******************************************************************************/
RESULT MASS_NoData_Setup(uint8_t RequestNo)
{
	if ((Type_Recipient == (CLASS_REQUEST | INTERFACE_RECIPIENT))
	&&	(RequestNo == MASS_STORAGE_RESET) && (pInformation->USBwValue == 0)
	&&	(pInformation->USBwIndex == 0) && (pInformation->USBwLength == 0x00)
		)
	{
		ClearDTOG_TX(ENDP1);  // Initialize Endpoint 1
		ClearDTOG_RX(ENDP2);  // Initialize Endpoint 2
		CBW.dSignature = BOT_CBW_SIGNATURE; // intialise the CBW signature to enable the clear feature
		Bot_State = BOT_IDLE;
		return USB_SUCCESS;
	}
	return USB_UNSUPPORT;
}
/*******************************************************************************
  MASS_Get_Interface_Setting: Test the interface and the alternate setting
							  according to the supported one.
*******************************************************************************/
RESULT MASS_Get_Interface_Setting(uint8_t Interface, uint8_t AlternateSetting)
{
	if (AlternateSetting > 0)	return USB_UNSUPPORT; // in this application we don't have AlternateSetting
	else if (Interface > 0)		return USB_UNSUPPORT; // in this application we have only 1 interfaces
	return USB_SUCCESS;
}
/*******************************************************************************
  MASS_GetDeviceDescriptor: Get the device descriptor.
*******************************************************************************/
uint8_t *MASS_GetDeviceDescriptor(u16 Length)
{
	return Standard_GetDescriptorData(Length, &Device_Descriptor);
}
/*******************************************************************************
   MASS_GetConfigDescriptor: Get the configuration descriptor.
*******************************************************************************/
uint8_t *MASS_GetConfigDescriptor(u16 Length)
{
	return Standard_GetDescriptorData(Length, &Config_Descriptor);
}

/*******************************************************************************
  MASS_GetStringDescriptor: Get the string descriptors according to
							the needed index.
*******************************************************************************/
uint8_t *MASS_GetStringDescriptor(u16 Length)
{
	uint8_t wValue0 = pInformation->USBwValue0;
	if (wValue0 > 5)
		return NULL;
	return Standard_GetDescriptorData(Length, &String_Descriptor[wValue0]);
}
/*******************************************************************************
  Get_Max_Lun: Handle the Get Max Lun request.
*******************************************************************************/
uint8_t *Get_Max_Lun(u16 Length)
{
	if (Length == 0)
	{
		pInformation->Ctrl_Info.Usb_wLength = LUN_DATA_LENGTH;
		return 0;
	}
	return ((uint8_t*)(&Max_Lun));
}
/*******************************************************************************
Get_SerialNum :   Create the serial number string descriptor.
*******************************************************************************/
static uint8_t * SaveSerial(uint8_t * dst, uint32_t serial)
{
	int i;
	for (i = 0; i <4; i++)
	{
		dst += 2;
		*dst = (uint8_t)(serial);
		serial >>= 8;
	}
	return dst;
}

void Get_SerialNum(void)
{
	if (SERIAL_NO1 != 0)
		SaveSerial(
			SaveSerial(
				SaveSerial(
					MASS_StringSerial,
					SERIAL_NO1
				),
				SERIAL_NO2
			),
			SERIAL_NO3
		);
}
