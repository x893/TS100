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
*******************************************************************************/
void Delay_uS(uint32_t us)
{
	volatile uint32_t cnt = us;
	while (cnt != 0)
		cnt--;
}

/*******************************************************************************
@@name  Sim_I2C_Set
*******************************************************************************/
#define SDA_High()			GPIOB->BSRR = SDA_PIN
#define SDA_Low()			GPIOB->BRR  = SDA_PIN
#define SCL_High()			GPIOB->BSRR = SCL_PIN
#define SCL_Low()			GPIOB->BRR  = SCL_PIN

/*******************************************************************************
@@name  Sim_I2C_Start
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
@@name	I2C_PageWrite
@@brief	Write the numbyte bytes of data to the address deviceaddr,
		writing the contents at pbuf
@@param	pbuf the contents of the write,
		numbyte for the number of bytes written,
		deviceaddr for the write address
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
@@brief Read 8-bit data to I2C
@@param	pbuf Read out the storage address
		numbyte The number of bytes read out
		deviceaddr Device address
		readaddr Read the content address
@@return Read out the eight data
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
