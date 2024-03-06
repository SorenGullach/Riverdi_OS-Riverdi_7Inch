
#include "HSEM.h"
#include <stddef.h>
#include <stdio.h>

#ifdef CORE_CM7
// Use interrupt line 1 for CM7 Master
#define HSEM_IRQHandler HSEM1_IRQHandler 
#define HSEM_IER HSEM->C1IER
#define HSEM_MISR HSEM->C1MISR
#define HSEM_ICR HSEM->C1ICR
#endif
#ifdef CORE_CM4
// Use interrupt line 2 for CM4 Master
#define HSEM_IRQHandler HSEM2_IRQHandler
#define HSEM_IER HSEM->C2IER
#define HSEM_MISR HSEM->C2MISR
#define HSEM_ICR HSEM->C2ICR
#endif

// array to hold callback for interrupts
hwHSEMControl::HSEMCallBack hwHSEMControl::HSEMCallBacks[HSEM_SEMID_MAX + 1];

extern "C" void HSEM_IRQHandler()
{
	// Disable Interrupts
	HSEM_IER &= ~(HSEM_MISR);
	
	uint32_t SemMask = HSEM_MISR; 
	for (size_t i = 0; i <= HSEM_SEMID_MAX; i++, SemMask >>= 1)
	{
		if ((SemMask & 0x01) && hwHSEMControl::HSEMCallBacks[i].Control != nullptr)
			hwHSEMControl::HSEMCallBacks[i].Control->ModifiedNotified(hwHSEMControl::HSEMCallBacks[i].Data);
	}			
	// Clear Flags
	HSEM_ICR = HSEM_MISR;
}
