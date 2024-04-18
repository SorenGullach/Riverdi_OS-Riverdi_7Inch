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

#pragma once

#include "stm32h7xx.h"
#include <GPIO.h>
#include <SysTick.h>
#include <TFT_Setup.h>

// https://www.st.com/resource/en/application_note/an4860-introduction-to-dsi-host-on-stm32-mcus-and-mpus-stmicroelectronics.pdf

class hwDSI
{
public:
	hwDSI()
	{
	}
	void Init();

	void UnitTest(bool on)
	{
		if (!on)
		{
			DSI->VMCR &= ~(DSI_VMCR_PGE | DSI_VMCR_PGO);
			DSI->WCR |= DSI_WCR_LTDCEN;
			DSI->WCR |= DSI_WCR_DSIEN;
			return;
		}
		
		DSI->WCR &= ~DSI_WCR_LTDCEN;
		if (DSI->VMCR & DSI_VMCR_PGO)
			DSI->VMCR &= ~DSI_VMCR_PGO;
		else
			DSI->VMCR |= DSI_VMCR_PGO;
		DSI->VMCR |= DSI_VMCR_PGE;
	}
private:
	void Regulator(bool on);
	void PLL(bool on);
	void HostWrapper();
	void HostTiming();
	void HostFlowControl();
	void HostLTDC();
	void VideoMode();
	void ShortWrite(uint32_t ChannelID, uint32_t Mode, uint32_t Param1, uint32_t Param2);
	void SendDCSCommands();
	
	uint32_t laneByteClk_KHz = 62000; // see RCC clock tree (PLL setup)
	uint32_t unitIntervalx4 = 8;
};