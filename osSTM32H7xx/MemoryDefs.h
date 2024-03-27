#pragma once

#include "stm32h7xx.h"

#define __RAM __attribute__((section(".ramdata"))) 
#define __RAMP __attribute__((section(".ramdata"),aligned(1))) 
#define __RAMFUNC __attribute__((section(".fastcode"))) 
#define __SHARED __attribute__(( section(".shared"),aligned(4) ))
#define __BACKUP __attribute__(( section(".backup"),aligned(4) )) 

#ifdef CORE_CM7
#define __DTCMRAM __attribute__((section(".dtcmram"))) 
#define __ITCMRAM __attribute__((section(".itcmram"))) 
#endif

#ifdef CORE_CM4
#endif

#ifdef CORE_CM4
#define FLASH_START 0x08100000 //, LENGTH = 1M
#define RAM_D2_START 0x10000000 //, LENGTH = 288K
#endif

# ifdef CORE_CM7
#define FLASH_START		FLASH_BANK1_BASE	//, LENGTH = 1M
//#define FLASH_END		0x081FFFFFUL		//, LENGTH = 1M
#define DTCMRAM_START	D1_DTCMRAM_BASE		//, LENGTH = 128K
#define DTCMRAM_END		0x2001FFFFUL		//, LENGTH = 128K
#define ITCMRAM_START	D1_ITCMRAM_BASE		//, LENGTH = 64K
#define ITCMRAM_END		0x0000FFFFUL		//, LENGTH = 64K
#define QSPI_START		QSPI_BASE			//, LENGTH = 128000K
#define QSPI_END		0x97FFFFFF			//, LENGTH = 128000K
#define RAM_D1_START	FLASH_BANK1_BASE	//, LENGTH = 512K
#endif

#define SHARED_START	D3_SRAM_BASE	//, LENGTH = 64K  /* Shared SRAM4 0x38000000 - 0x3800FFFF */
#define SHARED_END		0x3800FFFF	//, LENGTH = 64K  /* Shared SRAM4 0x38000000 - 0x3800FFFF */
#define BKPSRAM_START	D3_BKPSRAM_BASE	//, LENGTH = 4K   /* Backup SRAM4 0x38800000 - 0x38800FFF */
#define BKPSRAM_END		0x38800FFF	//, LENGTH = 4K   /* Backup SRAM4 0x38800000 - 0x38800FFF */
