#ifndef __VERSION_H
#define __VERSION_H

#define M_VER		'V','0'
#define S_VER		'0','1'
#define OEM_TYPE	"0"
#define APP_BASE	0x0800C000

/*----------------------------------------------------------------------------*/
#if		defined( TYPE_DS201 )
	#define PRODUCT_INFO	"DS201"
	#define SCH_VER			"1.7B"
#elif	defined( TYPE_DS202 )
	#include "DS202V1_6.h"
	#define FLASH_PAGE		0x0800
#elif	defined( TYPE_DS203)
	#define PRODUCT_INFO	"DS203"
	#define SCH_VER			"2.70"
#elif	defined( TYPE_S100 )
	#define PRODUCT_INFO	"TS100"
	#include "S100V0_1.h"
#endif

/*----------------------------------------------------------------------------*/
#define DFU_VER			{ M_VER, '.', S_VER, LCD_M, 0 }

#endif /* VERSION_H */
