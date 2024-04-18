/*
 * Copyright 2024 S�ren Gullach
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
 * "S�ren Gullach <STM32_R7_OS@toolsbox.dk>"
 */

#pragma once

#include "stm32h7xx.h"

class hwSysTick
{
public:
	static void Init();

	static void Delay(uint32_t Delay_ms);
#ifdef DEBUG
	static void UnitTest();
#else	static void UnitTest() {};
#endif
	
	static uint32_t Ticks;
};

#define osDelay(ms) hwSysTick::Delay(ms)
#define osTicks_ms hwSysTick::Ticks

