
#include "stm32h7xx.h"
#include <assert.h>

#include <MemoryDefs.h>
#include <MPU.h>

extern"C" void SystemInit()
{

	/* FPU settings ------------------------------------------------------------*/
#if (__FPU_PRESENT == 1) && (__FPU_USED == 1)
	SCB->CPACR |= ((3UL << (10 * 2)) | (3UL << (11 * 2))); /* set CP10 and CP11 Full Access */
#endif

    /*SEVONPEND enabled so that an interrupt coming from the CPU(n) interrupt signal is
     detectable by the CPU after a WFI/WFE instruction.*/
	//SCB->SCR |= SCB_SCR_SEVONPEND_Msk;
	
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
