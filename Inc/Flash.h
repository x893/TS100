#ifndef __EXT_FLASH_H
#define __EXT_FLASH_H

#define PAGESIZE   256

#define WREN	0x06	// Write enable instruction
#define READ	0x03	// Read from Memory instruction
#define RDSR	0x05	// Read Status Register instruction
#define PP		0x02	// Write to Memory instruction
#define PW		0x0A	// Page write instruction

#define OK		0	// 操作完成
#define SEC_ERR	1	// 扇区读写错误
#define TMAX	100000	// 超时限制

#define WIP_Flag	0x01	// Write In Progress (WIP) flag
#define Dummy_Byte	0xA5

uint8_t   FLASH_Prog(uint32_t Address, uint16_t Data);
void FLASH_Erase(uint32_t Address);
void ExtFlashPageWR(uint8_t* pBuffer, uint32_t WriteAddr);
void ExtFlashDataRD(uint8_t* pBuffer, uint32_t ReadAddr, uint16_t Lenght);

#endif
