/*
 * Copyright 2024 Søren Gullach
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * You must include the following attribution in all copies or substantial portions of the Software:
 * "Søren Gullach <STM32_R7_OS@toolsbox.dk>"
 */

#include "stm32h7xx.h"
#include <assert.h>

#include <MemoryDefs.h>
#include <MPU.h>
#include <RCC.h>

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
	
	hwSysClock::Reset();
	
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
	
	/*
	 * Disable the FMC bank1 (enabled after reset).
	 * This, prevents CPU speculation access on this bank which blocks the use of FMC during
	 * 24us. During this time the others FMC master (such as LTDC) cannot use it!
	 */
	FMC_Bank1_R->BTCR[0] = 0x000030D2;
}

