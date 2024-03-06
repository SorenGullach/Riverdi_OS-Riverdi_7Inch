#pragma once

#include "stm32h7xx.h"

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
#define FLASH_START		0x08000000	//, LENGTH = 1M
#define DTCMRAM_START	0x20000000	//, LENGTH = 128K
#define ITCMRAM_START	0x00000000	//, LENGTH = 64K
#define ITCMRAM_LENGTH	0x0000FFFF	//, LENGTH = 64K
#define QSPI_START		0x90000000	//, LENGTH = 128000K
#define RAM_D1_START	0x24000000  //, LENGTH = 512K
#endif

#define SHARED_START	0x38000000	//, LENGTH = 64K  /* Shared SRAM4 0x38000000 - 0x3800FFFF */
#define BKPSRAM_START	0x38800000	//, LENGTH = 4K   /* Backup SRAM4 0x38800000 - 0x38800FFF */
