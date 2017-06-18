#include "APP_Version.h"
#include "Flash.h"
#include "Bios.h"

/*******************************************************************************
 FLASH_Prog:
*******************************************************************************/
uint8_t FLASH_Prog(uint32_t Address, uint16_t Data)
{
	if (FLASH_WaitForLastOperation(WAIT_TIMES) != FLASH_TIMEOUT)
		FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);
	return FLASH_ProgramHalfWord(Address, Data);
}

/*******************************************************************************
 FLASH_Erase:
*******************************************************************************/
void FLASH_Erase(uint32_t Address)
{
	if (Address % FLASH_PAGE == 0)
	{  // FLASH Page start (1K/Page)
		if (FLASH_WaitForLastOperation(WAIT_TIMES) != FLASH_TIMEOUT)
			FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);
		FLASH_ErasePage(Address);
	}
}
