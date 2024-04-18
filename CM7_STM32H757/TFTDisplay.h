#include <math.h>
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

/*
*This file handles the setup of the Riverdi 7" STM32 display
*/

#include "DSI.h"
#include "LTDC.h"
#include "MemoryDefs.h"
#include "Timers.h"
#include "TFT_Setup.h"
#include "glTypes.h"

class TFTDisplayEvents
{
public:
	static void Blanking()
	{
	}
};
class TFTDisplay: public hwLTDC
{
public:
	TFTDisplay()
		: TimBackLight(Timer15::eInputClk::Internal)
	{
	}
	void Init()
	{
		ResetDisplay();

		TimBackLight.Prescaler(TimBackLight.FindPrescalerValue(5000'00)); // 1MHz
		TimBackLight.Periode(999);
		TimBackLight.Mode(1, Timer::eMode::PWM1); // set CCR1 to PWM1
		TimBackLight.Compare(1, 200); // set CCR1 to xxx
		
		hwLTDC::Init();

		TimBackLight.Start();
	}

	void Intencity(uint16_t value)
	{
		if (value == 0)
		{
			TimBackLight.Stop();
			return;
		}
		TimBackLight.Compare(1, value);
		TimBackLight.Start();
	}

	void UnitTest(int testLevel)
	{
		Printf("UnitTest of Display\n");
		for (int i = 0; i < 5; i++)
		{
			hwDSI::UnitTest(true);
			osDelay(1000); // Delay for 1 second
		}
		hwDSI::UnitTest(false); // Turn off display test and enable LTDC
		
		Layer(1, NULL, 0, 0, hwLTDC::Width(), hwLTDC::Height());
		Layer(2, NULL, 0, 0, hwLTDC::Width(), hwLTDC::Height());
		
		for (uint16_t i = 0; i < 1; i++)
		{
			for (uint16_t r = 0; r <= 255; r++)
			{
				hwLTDC::BackgroundColor(r, 0, 0);
				osDelay(10);
			}
			for (uint16_t g = 0; g <= 255; g++)
			{
				hwLTDC::BackgroundColor(0, g, 0);
				osDelay(10);
			}
			for (uint16_t b = 0; b <= 255; b++)
			{
				hwLTDC::BackgroundColor(0, 0, b);
				osDelay(10);
			}
		}

		for (uint16_t x = 0; x <= 255; x++)
		{
			float value = x * (2 * M_PI / 256); // Calculate the color value
			glColor_t t = RainbowColorGradient(value);
			hwLTDC::BackgroundColor(t.R, t.G, t.B);
			osDelay(50);
		}
		osDelay(1000);
		hwLTDC::BackgroundColor(125, 125, 125);
		
		if (testLevel > 2)
			UnitTest1();
		
		Printf("UnitTest of Display end \n");
	}

	void UnitTest1()
	{
		Printf("UnitTest1 of Display\n");

		// Define the number of lines for the display
		uint32_t Height1 = hwLTDC::Height(); // layer 1 size
		uint32_t Width1 = hwLTDC::Width();
		const uint32_t Height2 = 100; // layer 2 size
		uint32_t Width2 = hwLTDC::Width(600);
		const uint32_t Y2 = (Height1 - Height2) / 2;
		const uint32_t X2 = (Width1 - Width2) / 2;
		
		//__RAM_ALIGNED(4) static glColor_t VM2[200*300+64];

		uint32_t size = SDRAM_SIZE / 2; // Width2*Height2*sizeof(glColor_t);
		assert(size > Width1*Height1*sizeof(glColor_t) && "SDRam too small");
		glColor_t *pVM1 = (glColor_t *)SDRAM_START;
		glColor_t *pVM2 = (glColor_t *)(SDRAM_START + size);
//		glColor_t *pVM2 = (glColor_t *)&VM2;
		
		Printf(" Display layer 1 x=%d,y=%d,w=%d H=%d Adr=0x%8X\n", 0, 0, Width1, Height1, (uint32_t)pVM1);
		Printf(" Display layer 2 x=%d,y=%d,w=%d H=%d Adr=0x%8X\n", X2, Y2, Width2, Height2, (uint32_t)pVM2);
			
		// back color for background
		BackgroundColor(255, 0, 0);

		// a read band at "X"
		float value = (500 * 2 * M_PI) / Width2; // Calculate the color value
		glColor_t t = RainbowColorGradient(value);
		DefaultColor(2, t.R, t.G, t.B, t.A);
		
		Layer(1, pVM1, 0, 0, 0 + Width1, 0 + Height1);
		Layer(2, pVM2, X2, Y2, X2 + Width2, Y2 + Height2);
		
		BlendingFactors(1, eBlendingFactor1::F1_CA, eBlendingFactor2::F2_CA, 100);
		BlendingFactors(2, eBlendingFactor1::F1_CA, eBlendingFactor2::F2_CA, 100);

#define TDELAY 100
		// Update pixel data for display
		uint32_t x = 0, y = 0;
		pVM1[x + y*Width1].A = 255; 
		pVM1[x + y*Width1].R = 255;
		pVM1[x + y*Width1].G = 0;
		pVM1[x + y*Width1].B = 0;
		x = Width1 - 1, y = 0;
		pVM1[x + y*Width1].A = 255; 
		pVM1[x + y*Width1].R = 0;
		pVM1[x + y*Width1].G = 255;
		pVM1[x + y*Width1].B = 0;
		for (uint32_t y = 0; y < Height1; y++)
		{
			uint32_t x = Width1 - y - 1;
			pVM1[x + y*Width1].A = 255; // cross line
			pVM1[x + y*Width1].R = 255;
			pVM1[x + y*Width1].G = 255;
			pVM1[x + y*Width1].B = 255;
		}
		osDelay(1000);
		for (uint32_t y = 0; y < Height1; y++)
		{
			uint32_t x = y;
			pVM1[x + y*Width1].A = 255; // cross line
			pVM1[x + y*Width1].R = 255;
			pVM1[x + y*Width1].G = 255;
			pVM1[x + y*Width1].B = 255;
		}
		osDelay(TDELAY);

		for (uint32_t y = 0; y < Height1; y++)
		{
			uint32_t x = 0;
			pVM1[x + y*Width1].A = 255; // right line
			pVM1[x + y*Width1].R = 0;
			pVM1[x + y*Width1].G = 0;
			pVM1[x + y*Width1].B = 255;
		}
		osDelay(TDELAY);
		for (uint32_t y = 0; y < Height1; y++)
		{
			uint32_t x = Width1 - 1;
			pVM1[x + y*Width1].A = 255; // left line
			pVM1[x + y*Width1].R = 255;
			pVM1[x + y*Width1].G = 0;
			pVM1[x + y*Width1].B = 0;
		}
		osDelay(TDELAY);

		for (uint32_t x = 0; x < Width1; x++)
		{
			uint32_t y = 0;
			pVM1[x + y*Width1].A = 255; // top line
			pVM1[x + y*Width1].R = 255;
			pVM1[x + y*Width1].G = 0;
			pVM1[x + y*Width1].B = 0;
		}
		osDelay(TDELAY);
		for (uint32_t x = 0; x < Width1; x++)
		{
			uint32_t y = Height1 - 1;
			pVM1[x + y*Width1].A = 255; // bottom line
			pVM1[x + y*Width1].R = 0;
			pVM1[x + y*Width1].G = 255;
			pVM1[x + y*Width1].B = 0;
		}
		osDelay(TDELAY);

		for (uint32_t i = 0; i < 2; i++)
		for (uint32_t x = 0; x < Width2; x++)
		{
			float value = (x * 2 * M_PI) / Width2; // Calculate the color value
			glColor_t t = RainbowColorGradient(value);
			for (uint32_t y = 0; y < Height2; y++) 
			{
				pVM2[x + y*Width2] = t;
			}
		}
		osDelay(TDELAY);

		Printf("UnitTest1 of Display end \n");
	}

private:
	Timer15 TimBackLight; // backlight PWM
	hwGPIO BackLight {GPIOE, 5, 4, hwGPIO::ePP::PushPull } // PE5 as AF4 TIM15_CH1
	;
	hwGPIO DisplayReset {GPIOA, 6, hwGPIO::eMode::Output, hwGPIO::ePP::PushPull } // A6 LCD_DISP
	;
	void ResetDisplay()
	{
		DisplayReset.Low();
		osDelay(20);
		DisplayReset.High();
		osDelay(2);
	}
}
;