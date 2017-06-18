#include "APP_Version.h"
#include "usb_lib.h"
#include "Bios.h"
#include "HARDWARE.h"
#include "I2C.h"
#include "CTRL.h"

#define ADC1_DR_Address	((uint32_t)0x4001244C)

volatile uint32_t gTimeOut, gMs_timeout;
volatile uint32_t gTime[8];
volatile uint32_t gHeat_cnt = 0;
volatile uint16_t ADC1ConvertedValue[2];
volatile uint8_t  gSk = 0;

/*******************************************************************************
@@name  Get_AdcValue
@@brief Get the ADC-converted readings
@@param Converted AD
@@return NULL
*******************************************************************************/
uint16_t Get_TMP36(void)
{
	return ADC1ConvertedValue[0];
}
uint16_t Get_IRON(void)
{
	return ADC1ConvertedValue[1];
}
/*******************************************************************************
@@name  Set_HeatingTime
@@brief Set the heating time
@@param heating_time heating time
@@return NULL
*******************************************************************************/
void Clear_HeatingTime(void)
{
	gHeat_cnt = 0;
}
void Set_HeatingTime(uint32_t heating_time)
{
	gHeat_cnt = heating_time;
}

/*******************************************************************************
@@name  Get_HeatingTime
@@brief Read the heating time
@@param NULL 
@@return heating time
*******************************************************************************/
uint32_t Get_HeatingTime(void)
{
	return gHeat_cnt;
}

/*******************************************************************************
@@name  Delay_Ms
@@brief Delay program.
@@param Delay the number of milliseconds to wait
@@return NULL
*******************************************************************************/
void Delay_Ms(uint32_t ms)
{
	gMs_timeout = ms * 20;
	while (gMs_timeout)
		;
}

/*******************************************************************************
@@name  Delay_HalfMs
@@brief 0.5 millisecond per unit delay procedure.
@@param Delay waiting for 0.5 milliseconds
@@return NULL
*******************************************************************************/
void Delay_HalfMs(uint32_t ms)
{
	gMs_timeout = ms * 10;
	while(gMs_timeout)
		;
}

/*******************************************************************************
@@name  USB_Port
@@brief Set the USB device IO port
@@param State = ENABLE / DISABLE
@@return NULL
*******************************************************************************/
void USB_Port(uint8_t state)
{
	USB_DN_LOW();
	USB_DP_LOW();
	if (state == DISABLE)
	{
		USB_DN_OUT();
		USB_DP_OUT();
	}
	else
	{
		USB_DN_EN();
		USB_DP_EN();
	}
}

/*******************************************************************************

*******************************************************************************/
void RCC_Config(void)
{
	RCC_DeInit();
	FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);
	FLASH_SetLatency(FLASH_Latency_1);   // Flash 1 wait state for 48MHz
	RCC_CFGR_CFG();
	RCC_PLL_EN();
	RCC_HSICmd(ENABLE);
	RCC_PLLCmd(ENABLE);
	while(RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET)
		;
	RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);
	while(RCC_GetSYSCLKSource() != RCC_CFGR_SWS_PLL)
		;

	RCC_AHBPeriphClockCmd( RCC_AHBPeriph_SRAM | RCC_AHBPeriph_DMA1, ENABLE);
	RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_ADC1 | RCC_APB2Periph_ADC2, ENABLE );
	RCC_APB1PeriphClockCmd( RCC_APB1Periph_TIM2  | RCC_APB1Periph_TIM3, ENABLE);

	RCC_USBCLKConfig(RCC_USBCLKSource_PLLCLK_Div1);	   // USBCLK = 48MHz
}

/*******************************************************************************

*******************************************************************************/
void NVIC_Config(uint16_t tab_offset)
{
	NVIC_InitTypeDef nvic_init;

	NVIC_SetVectorTable(NVIC_VectTab_FLASH, tab_offset);

	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

	nvic_init.NVIC_IRQChannel					= USB_LP_CAN1_RX0_IRQn;
	nvic_init.NVIC_IRQChannelPreemptionPriority = 2;
	nvic_init.NVIC_IRQChannelSubPriority		= 0;
	nvic_init.NVIC_IRQChannelCmd				= ENABLE;
	NVIC_Init(&nvic_init);

	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);

	nvic_init.NVIC_IRQChannelPreemptionPriority	= 0;
	nvic_init.NVIC_IRQChannelSubPriority		= 1;
	nvic_init.NVIC_IRQChannelCmd				= ENABLE;

	nvic_init.NVIC_IRQChannel = TIM2_IRQn;
	NVIC_Init(&nvic_init);

	nvic_init.NVIC_IRQChannel = TIM3_IRQn;
	NVIC_Init(&nvic_init);
}

/*******************************************************************************

*******************************************************************************/
void GPIO_Config(void)
{
	GPIO_InitTypeDef gpio_init;

	//------ OLED_RST_PIN(PB9) ------------------------------------------------------------//
	gpio_init.GPIO_Speed = GPIO_Speed_50MHz;
	gpio_init.GPIO_Mode  = GPIO_Mode_Out_PP;

	gpio_init.GPIO_Pin = OLED_RST_PIN;
	GPIO_Init(GPIOB, &gpio_init);

	//------- Heating control bit PB4--------------------------------------------------------//
	// PB4=JNTRST
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_NoJTRST, ENABLE);

	gpio_init.GPIO_Pin = HEAT_PIN;
	GPIO_Init(GPIOB, &gpio_init);

	GPIOB->BSRR = (SDA_PIN | SCL_PIN);
	gpio_init.GPIO_Pin   = (SCL_PIN | SDA_PIN);
	gpio_init.GPIO_Mode  = GPIO_Mode_Out_OD;
	GPIO_Init(GPIOB, &gpio_init);

	//------ PA7 As the analog channel Ai7 input pin ----------------------------------------//
	gpio_init.GPIO_Pin  = GPIO_Pin_7;
	gpio_init.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_Init(GPIOA, &gpio_init);

	//------ PB0 As the analog channel Ai8 input pin ---------------------------------------//
	gpio_init.GPIO_Pin  = GPIO_Pin_0;
	GPIO_Init(GPIOB, &gpio_init);

	//---------- INPUT Voltage Detection Pin VB PB1(Ai9) ---------------------------------------//
	gpio_init.GPIO_Pin  = VB_PIN;
	GPIO_Init(GPIOB, &gpio_init);

	//-------- K1 = PA8, K2 = PA6 ----------------------------------------------------------//
	gpio_init.GPIO_Pin  = KEY1_PIN | KEY2_PIN;
	gpio_init.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_Init(GPIOA, &gpio_init);
}

/*******************************************************************************
@@name	Adc_Init
@@brief	Initialize AD
@@param	NULL
@@return NULL
*******************************************************************************/
void Adc_Init(void)
{
	uint32_t timeout = 10 * 0x1000;
	ADC_InitTypeDef   ADC_InitStructure;
	DMA_InitTypeDef   DMA_InitStructure;

	/* DMA1 channel1 configuration ---------------------------------------------*/
	DMA_DeInit(DMA1_Channel1);
	DMA_InitStructure.DMA_PeripheralBaseAddr = ADC1_DR_Address;
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)ADC1ConvertedValue;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
	DMA_InitStructure.DMA_BufferSize = 2;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
	DMA_Init(DMA1_Channel1, &DMA_InitStructure);

	/* Enable DMA1 channel1 */
	DMA_Cmd(DMA1_Channel1, ENABLE);

	// ADC1 configuration ------------------------------------------------------//
	ADC_DeInit(ADC1);
	ADC_InitStructure.ADC_Mode			   = ADC_Mode_Independent;
	ADC_InitStructure.ADC_ScanConvMode	   = ENABLE;
	ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
	ADC_InitStructure.ADC_ExternalTrigConv   = ADC_ExternalTrigConv_None;
	ADC_InitStructure.ADC_DataAlign		  = ADC_DataAlign_Right;
	ADC_InitStructure.ADC_NbrOfChannel	   = 2;
	ADC_Init(ADC1, &ADC_InitStructure);

	// ADC2 configuration ------------------------------------------------------//
	ADC_DeInit(ADC2);
	ADC_InitStructure.ADC_Mode			   = ADC_Mode_Independent;
	ADC_InitStructure.ADC_ScanConvMode	   = ENABLE;
	ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
	ADC_InitStructure.ADC_ExternalTrigConv   = ADC_ExternalTrigConv_None;
	ADC_InitStructure.ADC_DataAlign		  = ADC_DataAlign_Right;
	ADC_InitStructure.ADC_NbrOfChannel	   = 1;
	ADC_Init(ADC2, &ADC_InitStructure);

	// ADC1,2 regular channel7  channel9 and channel8 configuration ----------//
	ADC_RegularChannelConfig(ADC1, ADC_Channel_7, 2, ADC_SampleTime_239Cycles5);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_8, 1, ADC_SampleTime_239Cycles5);
	ADC_RegularChannelConfig(ADC2, ADC_Channel_9, 1, ADC_SampleTime_55Cycles5);

	/* Enable ADC1 DMA */
	ADC_DMACmd(ADC1, ENABLE);

	ADC_Cmd(ADC1, ENABLE);						/* Enable ADC1 */
	ADC_Cmd(ADC2, ENABLE);						/* Enable ADC2 */

	ADC_ResetCalibration(ADC1);				   /* Enable ADC1 reset calibaration register */
	while (ADC_GetResetCalibrationStatus(ADC1))
		if (!timeout--)
			return ;   /* Check the end of ADC1 reset calibration register */

	ADC_ResetCalibration(ADC2);				   /* Enable ADC2 reset calibaration register */
	timeout = 10 * 0x1000;
	while(ADC_GetResetCalibrationStatus(ADC2))
		if(!timeout--)
			return ;   /* Check the end of ADC2 reset calibration register */

	ADC_SoftwareStartConvCmd(ADC1, ENABLE);
	ADC_SoftwareStartConvCmd(ADC2, ENABLE);
}

/*******************************************************************************
@@name	Init_Timers
@@brief	Initialize timers
@@param NULL
@@return NULL
*******************************************************************************/
void Init_Timers(void)
{
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;

	TIM_TimeBaseStructure.TIM_Prescaler		= 48-1;	  		// (48MHz)/48 = 1MHz
	TIM_TimeBaseStructure.TIM_Period		= 10000 - 1;	// Interrupt per 10mS
	TIM_TimeBaseStructure.TIM_ClockDivision	= TIM_CKD_DIV1;
	TIM_TimeBaseStructure.TIM_CounterMode	= TIM_CounterMode_Up;
	TIM_TimeBaseInit	(TIM2, &TIM_TimeBaseStructure);
	TIM_ARRPreloadConfig(TIM2, ENABLE);
	TIM_ITConfig		(TIM2, TIM_IT_Update, ENABLE);
	TIM_Cmd				(TIM2, ENABLE);

	TIM_TimeBaseStructure.TIM_Prescaler		= 48-1;			// (48MHz)/48 = 1MHz
	TIM_TimeBaseStructure.TIM_Period		= 50-1;			// Interrupt per 50us
	TIM_TimeBaseStructure.TIM_ClockDivision	= TIM_CKD_DIV2;
	TIM_TimeBaseStructure.TIM_CounterMode	= TIM_CounterMode_Up;
	TIM_TimeBaseInit	(TIM3, &TIM_TimeBaseStructure);
	TIM_ARRPreloadConfig(TIM3, ENABLE);
	TIM_ITConfig		(TIM3, TIM_IT_Update, ENABLE);
	TIM_Cmd				(TIM3, ENABLE);
}

/*******************************************************************************
@@name	TIM2_ISR
@@brief	Timer 2 interrupt function scans the keyboard
@@param	NULL
@@return NULL
*******************************************************************************/
void TIM2_IRQHandler(void)
{
	int i;

	TIM_ClearITPendingBit(TIM2, TIM_IT_Update);

	for (i = 0; i < 8; i++)
		if (gTime[i] > 0)
			gTime[i]--;

	if (gSk++ == 2)
	{	// 20ms
		Key_Read();
		gSk = 0;
	}
}

/*******************************************************************************
@@name	TIM3_ISR
@@brief	Timer 3 interrupt function
@@param	NULL
@@return NULL
*******************************************************************************/
void TIM3_IRQHandler(void)
{
	static uint8_t heat_flag = 0;

	TIM_ClearITPendingBit(TIM3, TIM_IT_Update);

	if (gTimeOut > 0)
		gTimeOut--;
	if (gMs_timeout > 0)
		gMs_timeout--;

	if (gHeat_cnt > 0)
	{
		gHeat_cnt--;
		if (heat_flag)	HEAT_OFF();
		else			HEAT_ON();
		heat_flag = ~heat_flag;
	}

	if (gHeat_cnt == 0)
	{
		HEAT_OFF();
		heat_flag = 0;
	}
}
