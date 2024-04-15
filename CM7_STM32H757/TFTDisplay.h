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

#include <DSI.h>
#include <LTDC.h>
#include <Timers.h>
#include <TFT_Setup.h>

class TFTDisplay
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
		
		LCD_DSI.Init();
		LCD_LTDC.Init();

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

	// A constant background color (RGB888) can programmed.
	// It is used for blending with the bottom layer.
	void BackgroundColor(uint8_t r, uint8_t g, uint8_t b)
	{
		LCD_LTDC.BackgroundColor(r, g, b);
	}
	// The blending is always active and the two layers can be blended following the blending factors
	void BlendingFactors(R_t<uint8_t, 1, 2> layer, eBlendingFactor1 BF1, eBlendingFactor2 BF2, uint8_t constantAlpha)
	{
		LCD_LTDC.BlendingFactors((uint8_t)layer, BF1, BF2, constantAlpha);
	}
	// Every layer can have a default color in the format ARGB which is used outside the defined
	// layer window or when a layer is disabled.
	void DefaultColor(R_t<uint8_t, 1, 2> layer, uint8_t r, uint8_t g, uint8_t b, uint8_t alpha)
	{
		LCD_LTDC.DefaultColor((uint8_t)layer, r, g, b, alpha);
	}
	// A color key (RGB) can be configured to be representative for a transparent pixel.
	void ColorKey(R_t<uint8_t, 1, 2> layer, uint8_t r, uint8_t g, uint8_t b)
	{
		LCD_LTDC.ColorKey((uint8_t)layer, r, g, b);
	}
	
	void Layer(R_t<uint8_t, 1, 2> layer, void *pRGBData, uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
	{
		LCD_LTDC.Layer(layer, pRGBData, x0, y0, x1, y1);
	}
	
	// Define a structure to represent colors in ARGB format
	struct ARGB_t
	{
		uint8_t B, G, R, A;
	}__PACKED; 
	static_assert(sizeof(ARGB_t) == 4, "Wrong size");

#define M_PI 3.1415
	ARGB_t rainbowColorGradient(float value) 
	{
		ARGB_t rgb;
		rgb.R = sin(value) * 127 + 128;
		rgb.G = sin(value + 2 * M_PI / 3) * 127 + 128;
		rgb.B = sin(value + 4 * M_PI / 3) * 127 + 128;
		rgb.A = 255;

		return rgb;
	}

	void UnitTest()
	{
		Printf("UnitTest of Display\n");
		for (int i = 0; i < 5; i++)
		{
			LCD_DSI.UnitTest(true);
			osDelay(1000); // Delay for 1 second
		}
		LCD_DSI.UnitTest(false); // Turn off display test and enable LTDC
		
		Layer(1, NULL, 0, 0, LCD_LTDC.Width(), LCD_LTDC.Height());
		Layer(2, NULL, 0, 0, LCD_LTDC.Width(), LCD_LTDC.Height());
		
		for (uint16_t i = 0; i < 1; i++)
		{
			for (uint16_t r = 0; r <= 255; r++)
			{
				LCD_LTDC.BackgroundColor(r, 0, 0);
				osDelay(10);
			}
			for (uint16_t g = 0; g <= 255; g++)
			{
				LCD_LTDC.BackgroundColor(0, g, 0);
				osDelay(10);
			}
			for (uint16_t b = 0; b <= 255; b++)
			{
				LCD_LTDC.BackgroundColor(0, 0, b);
				osDelay(10);
			}
		}

		for (uint16_t x = 0; x <= 255; x++)
		{
			float value = x * (2 * M_PI / 256); // Calculate the color value
			ARGB_t t = rainbowColorGradient(value);
			LCD_LTDC.BackgroundColor(t.R, t.G, t.B);
			osDelay(50);
		}
		osDelay(1000);
		LCD_LTDC.BackgroundColor(125, 125, 125);
		
		Printf("UnitTest of Display end \n");
	}

	void UnitTest1()
	{
		Printf("UnitTest1 of Display\n");

		// Define the number of lines for the display
		uint32_t Height1 = LCD_LTDC.Height(); // layer 1 size
		uint32_t Width1 = LCD_LTDC.Width();
		const uint32_t Height2 = 200; // layer 2 size
		const uint32_t Width2 = 600;
		const uint32_t Y2 = (Height1 - Height2) / 2;
		const uint32_t X2 = (Width1 - Width2) / 2;
		
		__RAM_ALIGNED(4) static ARGB_t VM2[Height2*Width2];

//		uint32_t size = Width1*Height1*sizeof(ARGB_t);
		ARGB_t *pVM1 = (ARGB_t *)SDRAM_START;
		ARGB_t *pVM2 = (ARGB_t *)&VM2;
		
		Printf(" Display layer 1 x=%d,y=%d,w=%d H=%d Adr=0x%8X\n", 0, 0, Width1, Height1, (uint32_t)pVM1);
		Printf(" Display layer 2 x=%d,y=%d,w=%d H=%d Adr=0x%8X\n", X2, Y2, Width2, Height2, (uint32_t)pVM2);
			
		BackgroundColor(255, 0, 0);
		//		DefaultColor(1, 0, 255, 0, 255);
		Layer(1, pVM1, 0, 0, 0 + Width1, 0 + Height1);
		Layer(2, pVM2, X2, Y2, X2 + Width2, Y2 + Height2);
		BlendingFactors(1, eBlendingFactor1::F1_CA, eBlendingFactor2::F2_CA, 200);
		BlendingFactors(2, eBlendingFactor1::F1_CA, eBlendingFactor2::F2_CA, 200);

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
			ARGB_t t = rainbowColorGradient(value);
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

	hwDSI LCD_DSI;
	hwLTDC LCD_LTDC;
}
;