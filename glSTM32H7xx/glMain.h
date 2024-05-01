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
#include "glButton.h"
#include "TFTDisplay.h"
#include "TPDisplay.h"

#include "glPage.h"

class glMain : protected glVideoMemory, private cbDisplay
{
public:
	void Init()
	{
		Display.Init(this); // the display
		Touch.Init(this); // for touch control
		
		// Find the size of the display
		glVideoMemory::Width = Display.Width(); glVideoMemory::Height = Display.Height();
		// Display buffer
		glVideoMemory::pVM = (glARGB_t *)SDRAM_START;

		// back color for background
		glColor_t bg = glColor_t(glColors::GREEN);
		Display.BackgroundColor(bg.R, bg.G, bg.B);
		// layer
		Display.Layer(1, pVM, 0, 0, 0 + Width, 0 + Height);
		// blending
		Display.BlendingFactors(1, eBlendingFactor1::F1_CA, eBlendingFactor2::F2_CA, 255);
		glColor_t df = glColor_t(glColors::RED);
		Display.DefaultColor(1, df.A, df.R,df.G,df.B);
		glColor_t ck = glColor_t(glColors::BLACK);
		Display.ColorKey(1, ck.R,ck.G,ck.B);
	}
	
	void AddPage(glPage *page)
	{
		page->PosSize(gl2DPoint_t(0, 0, Display.Width() - 1, Display.Height() - 1));
		ChainPages.Add(page);
		//page->Init();
		
		if (pCurrentPage == nullptr)
			pCurrentPage = ChainPages.Head();
	}
	
private:
	// Display to draw on
	TFTDisplay Display;
	// Touch control
	CTPDisplay Touch;
	
	glChain<glPage> ChainPages;
	glPage *pCurrentPage = nullptr;

public:
	void cbTouch(sTPPoint &point) override
	{
		if (pCurrentPage == nullptr) return;
		pCurrentPage->UpdateState(point);
		
		switch (pCurrentPage->EventAction.EventType)
		{
		case glEvent::eEventType::Click:
			break;
		case glEvent::eEventType::Slide:
			if (pCurrentPage->EventAction.U.Slide.Action == sTPPoint::Right && pCurrentPage->Prev() != nullptr)			
			{
				pCurrentPage->EventAction.U.Slide.Action = sTPPoint::None; 
				pCurrentPage = (glPage *)(pCurrentPage->Prev());
				//Printf("%s Currentpage\n", pCurrentPage->Name);
				pCurrentPage->Invalidate();
			}
			if (pCurrentPage->EventAction.U.Slide.Action == sTPPoint::Left && pCurrentPage->Next() != nullptr)			
			{
				pCurrentPage->EventAction.U.Slide.Action = sTPPoint::None; 
				pCurrentPage = (glPage *)(pCurrentPage->Next());
				//Printf("%s Currentpage\n", pCurrentPage->Name);
				pCurrentPage->Invalidate();
			}
			break;
		case glEvent::eEventType::None:
			break;
		}
		pCurrentPage->EventAction.EventType = glEvent::eEventType::None;
	}

	// lines to draw on each update
//	P_t ls = 10;
	// where to draw on graphic buffer
//	P_t yStart, yEnd;

	void cbBlanking() override
	{
		if (pCurrentPage == nullptr) return;
		
		gl2DPoint_t InvalidRegion;
		if (!pCurrentPage->IsInvalidated(InvalidRegion))
			return;
/*
		yEnd = yStart + ls;
		if (yEnd >= Height)
			yEnd = Height;

		if (yStart > InvalidRegion.LT.Y)
			InvalidRegion.LT.X = yStart;
		if (yEnd < InvalidRegion.RB.Y)
			InvalidRegion.RB.Y = yEnd;
*/		
		pCurrentPage->UpdateLook(InvalidRegion);
		/*	
				yStart = yEnd;
				if (yStart >= Height)
					yStart = 0;
	
			if (Display.BlankingOverrun() && ls > 10)
				ls /= 2;
		else if (ls < Height / 2)
			ls++;
*/			
	}
};

