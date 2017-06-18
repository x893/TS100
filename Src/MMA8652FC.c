#include <stdio.h>
#include <string.h>
#include "APP_Version.h"
#include "Bios.h"
#include "Oled.h"
#include "MMA8652FC.h"
#include "I2C.h"
#include "CTRL.h"
#include "UI.h"
//------------------------------------------------------------------//

typedef struct {
	uint8_t		gactive, gShift;
} MMA_Context_t;
MMA_Context_t MMA_Context;

/*******************************************************************************
@@name  Get_MmaActive
@@brief Obtain the acceleration sensor in a static state
@@param NULL
@@return Acceleration sensor status
*******************************************************************************/
uint8_t Get_MmaActive(void)
{
	return MMA_Context.gactive;
}
/*******************************************************************************
@@name  Get_MmaActive
*******************************************************************************/
uint8_t Get_MmaShift(void)
{
	return MMA_Context.gShift;
}

/*******************************************************************************
@@name  MMA_RegWrite
@@brief Write Data to Reg address
@@param Reg set the address in the Data data
@@return  success
*******************************************************************************/
int MMA_RegWrite(uint8_t reg, uint8_t data)
{
	uint8_t tx_data[4];

	tx_data[0] = reg;
	tx_data[1] = data;
	I2C_PageWrite(tx_data, 2, DEVICE_ADDR);
	return 1;
}
/*******************************************************************************
@@name  MMA_RegRead
@@brief Read data from setup Reg
@@param Reg set the address, stored in gMmatxdata
@@return  success;
*******************************************************************************/
uint8_t MMA_RegRead(uint8_t reg)
{
	uint8_t tx_data[4];
	tx_data[0] = reg;

	I2C_PageRead(tx_data, 1, DEVICE_ADDR, reg);
	return tx_data[0];
}

/*******************************************************************************
@@name  MMA_RegRead
@@brief Read data from setup Reg
@@param NULL
@@return NULL
*******************************************************************************/
void StartUp_Accelerated(void)
{
	MMA_RegWrite(CTRL_REG1, 0);
	MMA_RegWrite(XYZ_DATA_CFG_REG, FULL_SCALE_8G);
	MMA_RegWrite(CTRL_REG1, DataRateValue);
	MMA_RegWrite(CTRL_REG2, 0);
	MMA_RegWrite(CTRL_REG1, ACTIVE_MASK );
}

/*******************************************************************************
@@name  Read_ZYXDr
@@brief Read XYZ direction
@@param NULL
@@return X, Y, X
*******************************************************************************/
static int Read_ZYXDr(int16_t *xyz)
{
	int i;
	int16_t value;
	uint8_t reg = X_MSB_REG;
	// Poll the ZYXDR status bit and wait for it to set
	if ((MMA_RegRead(STATUS_REG) & ZYXDR_BIT) != 0)
	{	// Read 12/10-bit XYZ results using a 6 byte IIC access
		for (i = 0; i < 3; i++)
		{
			value  = (uint16_t)MMA_RegRead(reg++) << 8;
			value |= (uint16_t)MMA_RegRead(reg++);
			value <<= 4;
			*xyz++ = value >> 8;
		}
		return 1;
	}
	return 0;
}

/*******************************************************************************
@@name  Check_Accelerated
@@brief Check if the acceleration sensor is moving
*******************************************************************************/

void Check_Accelerated(void)
{
	static int16_t xyz_prev[3];
	int16_t xyz[3];
	int16_t v, i, d = 0;

	MMA_Context.gactive = 0;
	MMA_Context.gShift = 0;

	if (Read_ZYXDr(xyz))
	{	// Read data, update data
		for (i = 0; i < 3; i++)
		{
			v = xyz[i] - xyz_prev[i];
			if (v < 0) v = -v;
			d += v;
			xyz_prev[i] = xyz[i];
		}

		if (d > 16) MMA_Context.gactive = 1;
		if (d > 32) MMA_Context.gShift = 1;
	}
}
