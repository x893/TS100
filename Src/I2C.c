#include "stm32f10x.h"
#include "I2C.h"
#include "Bios.h"
#include "oled.h"
#include "S100V0_1.h"

#define HIGH		1
#define LOW			0

#define SDA_VAL		(GPIOB->IDR & SDA_PIN)
#define SCL_VAL		(GPIOB->IDR & SCL_PIN)

#define I2C_MORE		1
#define I2C_LAST		0
#define I2C_TIMEOUT		255

#define FAILURE			0
#define SUCCEED			1
#define I2C_DELAYTIME	2

static void Sim_I2C_Stop(void);
static void Sim_I2C_Start(void);
static uint8_t Sim_I2C_RD_Byte(uint8_t more);
static uint8_t Sim_I2C_WR_Byte(uint8_t data);

/*******************************************************************************
@@name  Delay_uS
@@brief  软件延时
@@param us 
@@return NULL
*******************************************************************************/
void Delay_uS(uint32_t us)
{
	volatile uint32_t cnt = us;
	while (cnt != 0)
		cnt--;
}

/*******************************************************************************
@@name  Sim_I2C_Set
@@brief  配置I2C
@@param pin:GPIO_Pin_	  status:电平状态
@@return NULL
*******************************************************************************/
#define SDA_High()			GPIOB->BSRR = SDA_PIN
#define SDA_Low()			GPIOB->BRR  = SDA_PIN
#define SCL_High()			GPIOB->BSRR = SCL_PIN
#define SCL_Low()			GPIOB->BRR  = SCL_PIN

/*******************************************************************************
@@name  Sim_I2C_Start
@@brief  开始
@@param NULL
@@return NULL
*******************************************************************************/
void Sim_I2C_Start(void)
{
	SCL_Low();
	SDA_High();
	SCL_High();
	Delay_uS(I2C_DELAYTIME);

	SDA_Low();
	Delay_uS(I2C_DELAYTIME);
	
	SCL_Low();
}

/*******************************************************************************
@@name  Sim_I2C_Stop
@@brief  停止
@@param NULL
@@return NULL
*******************************************************************************/
void Sim_I2C_Stop(void)
{
	SCL_Low();
	SDA_Low();
	Delay_uS(I2C_DELAYTIME);

	SCL_High();
	Delay_uS(I2C_DELAYTIME);

	SDA_High();
	Delay_uS(I2C_DELAYTIME);

	SCL_Low();
	Delay_uS(I2C_DELAYTIME);
}

/*******************************************************************************
@@name  Sim_I2C_WR_Byte
@@brief 向I2C写八位数据
@@param data要写入的数据
@@return SUCCEED:成功		 FAILURE:失败
*******************************************************************************/
uint8_t Sim_I2C_WR_Byte(u8 data)
{
	int i = 8;
	while (i != 0)
	{
		i--;
		//send out a bit by sda line.
		SCL_Low();					// sclk low	
		if(data & 0x80) SDA_High();	// send bit is 1
		else			SDA_Low();	// send bit is 0
		Delay_uS(I2C_DELAYTIME);

		SCL_High();			// SCL high
		Delay_uS(I2C_DELAYTIME);

		data <<=1;					// left shift 1 bit, MSB send first.
	}

	SCL_Low();
	SDA_High();

	for (i = I2C_TIMEOUT; i != 0; i--)
	{	// wait for sda low	to receive ack
		Delay_uS(I2C_DELAYTIME);
		if (!SDA_VAL)
		{
			SCL_High();
			Delay_uS(I2C_DELAYTIME);

			SCL_Low();
			Delay_uS(I2C_DELAYTIME);

			return SUCCEED;
		}
	}
	return FAILURE;
}

/*******************************************************************************
@@name  Sim_I2C_RD_Byte
@@brief 向I2C读八位数据
@@param more
@@return 读出的八位数据
*******************************************************************************/
uint8_t Sim_I2C_RD_Byte(uint8_t more)
{
	int i = 8, byte = 0;

	SDA_High();				// SDA set as input
	while (i != 0)
	{
		--i;
		SCL_Low();			// SCL low
		Delay_uS(I2C_DELAYTIME);

		SCL_High();			// SCL high
		Delay_uS(I2C_DELAYTIME);

		byte <<= 1;					//recv a bit
		if (SDA_VAL) byte |= 0x01;
	}

	SCL_Low();
	if (!more)	SDA_High();	// last byte, send nack.
	else		SDA_Low();	// send ack
	Delay_uS(I2C_DELAYTIME);

	SCL_High();				// SCL_HIGH();
	Delay_uS(I2C_DELAYTIME);

	SCL_Low();
	return byte;
}

/*******************************************************************************
@@name  I2C_PageWrite
@@brief 向 地址 deviceaddr 写入numbyte个字节的数据，写入的内容在pbuf
@@param pbuf 写入的内容，numbyte 为写入的字节数，deviceaddr为写入的地址
@@return NULL
*******************************************************************************/
void I2C_PageWrite(uint8_t * pbuf, uint16_t numbyte, uint8_t deviceaddr )
{
	Sim_I2C_Start();
	Sim_I2C_WR_Byte(deviceaddr << 1);

	while (numbyte !=  0)
	{
		--numbyte;
		Sim_I2C_WR_Byte(*pbuf++);
	}
	Sim_I2C_Stop();
}
/*******************************************************************************
@@name  I2C_PageRead
@@brief 向I2C读八位数据
@@param  pbuf 读出来的存放地址 numbyte为读出来的字节数 
		  deviceaddr设备地址 readaddr读取的内容地址
@@return 读出的八位数据
*******************************************************************************/
void I2C_PageRead(uint8_t* pbuf,  uint16_t numbyte, uint8_t deviceaddr, uint8_t readaddr)
{
	Sim_I2C_Start();
	Sim_I2C_WR_Byte(deviceaddr << 1);
	Sim_I2C_WR_Byte(readaddr);
	Sim_I2C_Start();
	Sim_I2C_WR_Byte((deviceaddr << 1) | 1);

	while (numbyte != 0)
	{
		--numbyte;
		if (numbyte == 0)	*pbuf++ = Sim_I2C_RD_Byte(I2C_LAST);
		else				*pbuf++ = Sim_I2C_RD_Byte(I2C_MORE);
	}
	Sim_I2C_Stop();
}
