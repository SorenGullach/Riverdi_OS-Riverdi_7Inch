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
#include "assert.h"

#include "GPIO.h"
#include "Utils.h"
#include "DSI.h"
#include "TFT_Setup.h"

// https://indico.scc.kit.edu/event/460/contributions/5488/attachments/2862/4148/Datasheet_LDC-TFT_STM32.pdf
// https://www.st.com/resource/en/reference_manual/rm0399-stm32h745755-and-stm32h747757-advanced-armbased-32bit-mcus-stmicroelectronics.pdf

class hwLTDC_IRQs;
extern hwLTDC_IRQs *phwLTDC_IRQs;
class hwLTDC_IRQs
{
public:
	hwLTDC_IRQs()
	{
		phwLTDC_IRQs = this;
	}
	virtual void cbBlanking() = 0; // this function is called from the LTDC interrupt
};

class hwLTDC : public hwDSI
{
public:
	hwLTDC()
	{
	}
	void Init();

private:

	// 3. Configure the synchronous timings: VSYNC, HSYNC, vertical and horizontal back
	// porch, active data area and the front porch timings following the panel datasheet 
	// as described in the Section 33.4.1.
	void TimingParameters();

public:
	
	// A constant background color (RGB888) can programmed.
	// It is used for blending with the bottom layer.
	void BackgroundColor(uint8_t red, uint8_t green, uint8_t blue);
	// Every layer can have a default color in the format ARGB which is used outside the defined
	// layer window or when a layer is disabled.
	void DefaultColor(R_t<uint8_t, 1, 2> l, uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha); 
	// A color key (RGB) can be configured to be representative for a transparent pixel.
	void ColorKey(R_t<uint8_t, 1, 2> layer, uint8_t red, uint8_t green, uint8_t blue);
	// The blending is always active and the two layers can be blended following the blending factors
	void BlendingFactors(R_t<uint8_t, 1, 2> l, eBlendingFactor1 BF1, eBlendingFactor2 BF2, uint8_t constantAlpha);
			
	// init layer
	// 7. Configure the layer1 / 2 parameters by :
	// 8. Enable layer1 / 2 and if needed the CLUT in the LTDC_LxCR register.
	void Layer(R_t<uint8_t, 1, 2> l, void *pRGBData, uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);
	static uint16_t Height() {	return VACT; }
	static uint16_t Width() {	return HACT; }
	uint16_t Width(uint16_t w) 
	{	
		uint32_t BytesPerPixel = 4; // TODO depend on PixelFormat
		// LTDC Layerx Color Frame Buffer Length Register
		uint16_t LinePitch = w * BytesPerPixel;
		if ((LinePitch % 64) != 0)
		{
			LinePitch += 64 - (LinePitch % 64);
			assert((LinePitch % 64) == 0 && "Wrong line pitch");
		}
		return LinePitch / BytesPerPixel; 
	}
	void CurrentPos(uint16_t &x, uint16_t &y) 
	{ 
		x = (LTDC->CPSR&LTDC_CPSR_CXPOS_Msk) >> LTDC_CPSR_CXPOS_Pos;
		y = (LTDC->CPSR&LTDC_CPSR_CYPOS_Msk) >> LTDC_CPSR_CYPOS_Pos;
	}
	bool BlankingOverrun() 
	{ 
		uint16_t y = (LTDC->CPSR & LTDC_CPSR_CYPOS_Msk) >> LTDC_CPSR_CYPOS_Pos;
		return y >= VBP && y < VBP + VACT; 
	}
};
