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

#include "glPrimitives.h"
//#include "glText.h"
#include "TFTDisplay.h"
#include "LTDC.h"

class glMain : public glObject, protected glObjectAdr, hwLTDC_IRQs
{
protected:
	virtual void cbBlanking() override;
public:
	glMain()
	{
	}

	void Init()
	{
		Display.Init();
		
		// Find the number of lines for the display
		Height = Display.Height(); // layer 1 size
		Width = Display.Width();
		// Display buffer
		pVM = (glARGB_t *)SDRAM_START;

		// back color for background
		Display.BackgroundColor(0, 0, 255);
		// layer
		Display.Layer(1, pVM, 0, 0, 0 + Width, 0 + Height);
		// blending
		Display.BlendingFactors(1, eBlendingFactor1::F1_CA, eBlendingFactor2::F2_CA, 255);
		Display.DefaultColor(1, 255, 0, 0, 255);
		Display.ColorKey(1, 0, 0, 0);
	}
	
	// redraw your self
	virtual void Redraw() override
	{
		// here we have noting to do
	}

	glColor_t C = { 255, 0, 0, 255 };
	glColor_t C1 = { 0,255, 0, 255 };
	glColor_t C2 = { 0,0,255, 255 };
	glPixel p1 = { 100, 100, C };
	glPixel p2 = { 100, 101, C };
	glPixel p3 = { 100, 102, C };
	glPixel p4 = { 100, 103, C };
	glLine l1;
	glLine l2;
	glLine l3 = { 100, 104, 200, 104, C1 };
	glLine l4 = { 100, 104, 100, 200, C2 };
	glRectangle r1 = { 110, 110, 400, 400, C };

	void UnitTest()
	{
		l1 = glLine(0, 0, Width, Height, C);
		l2 = glLine(Width, 0, 0, Height, C1);
		Add(p1);
		p1.Add(p2);
		p2.Add(p3);
		p3.Add(p4);
		p4.Add(l1);
		l1.Add(l2);
		l2.Add(l3);
		l3.Add(l4);
		l4.Add(r1);
	}
	
private:
	// Display to draw on
	TFTDisplay Display;
	// lines to draw on each update
	P_t ls = 10;
};

