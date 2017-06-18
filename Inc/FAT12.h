#ifndef __FAT12_H
#define __FAT12_H

#include "stm32f10x_lib.h"

u8   ReadFileSec(uint8_t* Buffer, uint16_t* Cluster);
uint8_t   ReadDiskData(uint8_t* pBuffer, uint32_t ReadAddr, uint16_t Lenght);
uint8_t   NextCluster(uint16_t* Cluster);
uint8_t   ProgFileSec(uint8_t* Buffer, uint16_t* Cluster);
uint8_t   ProgDiskPage(uint8_t* Buffer, uint32_t ProgAddr);
uint8_t   SeekBlank(uint8_t* Buffer, uint16_t* Cluster);
uint8_t   SetCluster(uint8_t* Buffer, uint16_t* Cluster);
uint8_t   OpenFileRd(uint8_t* Buffer, uint8_t* FileName, uint16_t* Cluster, uint32_t* pDirAddr);
uint8_t   OpenFileWr(uint8_t* Buffer, uint8_t* FileName, uint16_t* Cluster, uint32_t* pDirAddr);
uint8_t   CloseFile(uint8_t* Buffer, uint32_t Lenght, uint16_t* Cluster, uint32_t* pDirAddr);
void ExtFlash_PageWrite(uint8_t* pBuffer, uint32_t WriteAddr, uint8_t Mode);

#endif
