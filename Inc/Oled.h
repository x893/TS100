#ifndef _OLED_SSD1306_H
#define _OLED_SSD1306_H

#include <stdint.h>

#define DEVICEADDR_OLED  0x3C

void Init_Oled(void);
void Sc_Pt(uint8_t contrast);
void Oled_DisplayOn(void);
void Oled_DisplayOff(void);
void Set_ShowPos(uint8_t x, uint8_t y);
void Clean_Char(uint8_t k, uint8_t wide);
uint8_t * Oled_DrawArea(uint8_t x0, uint8_t y0, uint8_t wide, uint8_t high, uint8_t * ptr);
uint8_t * Show_posi(uint8_t posi, uint8_t * ptr, uint8_t word_width);
void Write_Command(uint8_t Data);
void Write_Data(uint8_t Data);
uint8_t * Data_Command(uint16_t len, uint8_t * ptr);
void Reg_Command(uint8_t posi, uint8_t flag);
void Clear_Screen(void);
void Write_InitCommand_data(uint32_t Com_len, uint8_t* data);
void Display_BG(void);

#endif
