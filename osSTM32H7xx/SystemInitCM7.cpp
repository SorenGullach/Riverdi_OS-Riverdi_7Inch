
#include "stm32h7xx.h"
#include <assert.h>

#include <MemoryDefs.h>
#include <MPU.h>

extern void EnableICache();
extern void EnableDCache();

extern void *_sifastcode, *_sfastcode, *_efastcode;

extern "C" void SystemInit()
{
	// copy ramfunctions to itcm
	void **pSource, **pDest;
	for (pSource = &_sifastcode, pDest = &_sfastcode; pDest < &_efastcode; pSource++, pDest++)
		*pDest = *pSource;

	// FPU settings
#if (__FPU_PRESENT == 1) && (__FPU_USED == 1)
	SCB->CPACR |= ((3UL << (10 * 2)) | (3UL << (11 * 2))); /* set CP10 and CP11 Full Access */
#endif

	if ((DBGMCU->IDCODE & 0xFFFF0000U) < 0x20000000U)
	{
		/* if stm32h7 revY*/
		/* Change  the switch matrix read issuing capability to 1 for the AXI SRAM target (Target 7) */
		*((__IO uint32_t*)0x51008108) = 0x000000001U;
	}
	
	// Enable I-Cache
	SCB_EnableICache();
	// Enable D-Cache
	SCB_EnableDCache();

	// change SHARED to unbuffered
	hwMPU::EnableRegion(0,
		SHARED_START,
		hwMPU::eSize::S64KB,
		false, // executeNever
		true, // shareable
		false, // cacheable
		false, // bufferable
		hwMPU::eAccessPermission::PRW_URW,
		hwMPU::eTexLevel::Level0);
}

