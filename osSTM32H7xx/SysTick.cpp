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

#include <SysTick.h>
#include <RCC.h>
#include <Utils.h>
#include <assert.h>

uint32_t hwSysTick::Ticks = 0;

void hwSysTick::Init()
{
	uint32_t load = hwSysClock::CPUClk();
	load /= 1000UL;
	int8_t div = load > SysTick_CALIB_TENMS_Msk ? 8 : 1;
	assert(((load / div) - 1) < SysTick_CALIB_TENMS_Msk);
	
	SysTick->CTRL = 0; // disable
	SysTick->LOAD = (load / div) - 1;
	SysTick->VAL = 0;
	//	SysTick_Config(load);		

	if (div == 8)
		SysTick->CTRL = SysTick_CTRL_ENABLE_Msk | SysTick_CTRL_TICKINT_Msk; // AHB/8
	else
		SysTick->CTRL = SysTick_CTRL_ENABLE_Msk | SysTick_CTRL_TICKINT_Msk | SysTick_CTRL_CLKSOURCE_Msk; // AHB
		
	NVIC_SetPriority(SysTick_IRQn, (1UL << __NVIC_PRIO_BITS) - 1UL);
	NVIC_EnableIRQ(SysTick_IRQn);
}

void hwSysTick::Delay(uint32_t Delay_ms)
{
	uint32_t tickstart = Ticks;
	while ((Ticks - tickstart) < Delay_ms) __asm("");
}

#ifdef DEBUG
void hwSysTick::UnitTest()
{
	Printf("UnitTest of SysTick start\n");
	Printf("Testing delay %u MHz\n", hwSysClock::SysTickClk());
	uint32_t ticks = hwSysTick::Ticks;
	hwSysTick::Delay(1000);
	ticks = hwSysTick::Ticks - ticks;
	assert(ticks >= 1000 && ticks <= 1001 && "Delay test");
	Printf("1000 msec delay ok %u ms\n", ticks);
	for (int i = 0; i < 10; i++)
	{
		hwSysTick::Delay(1000);
		Printf("%d sec delay\n", i);
	}
	Printf("UnitTest of SysTick end\n");
}
#endif

extern "C" void SysTick_Handler()
{
	hwSysTick::Ticks++;	
}
