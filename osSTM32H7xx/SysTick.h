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

