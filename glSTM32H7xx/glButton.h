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

class glButton : public glObject
{
public:
	glButton()
		: glObject("Button", gl2DPoint_t(), glColor_t()) {}
	glButton(gl2DPoint_t pos)
		: glObject("Button", pos, glColor_t(glColors::BLUE)) 
	{
		BorderWidth = std::min(pos.Width(),pos.Height()) / 10;
	}
	glButton(gl2DPoint_t pos, P_t borderWidth)
		: glObject("Button", pos, glColor_t(glColors::BLUE)) 
	{
		assert(borderWidth < pos.Height() / 2);
		assert(borderWidth < pos.Width() / 2);
		BorderWidth = borderWidth;
	}

	void BackColor(glColor_t color)	{ _BackColor = color; }
	void PressedColor(glColor_t color)	{ _PressedColor = color; }

protected:
	// Initialize object
//	virtual void Init() override {}

	void Touch(const sTPPoint &point) override
	{
		if (Position.IsInside(point.X, point.Y)) 
		{
			if (point.TipAction == sTPPoint::eTipAction::Tip && !Pressed)
			{
				Pressed = true;
				Invalidate();
			}
			if (point.TipAction != sTPPoint::eTipAction::Tip && Pressed)
			{
				Click = true;
				Pressed = false;
				Invalidate();
			}
		}
		else
		{
			if (Pressed)
			{
				Pressed = false;
				Invalidate();
			}
		}
	}

	// redraw your self
	// return true if anything done
	virtual void Redraw(const gl2DPoint_t &invalidRegion) override
	{
//		if (!ImInvalidated) return false;

		if (Pressed)
			glRectangleFill(Position.Inflate(-BorderWidth), _PressedColor).Draw(invalidRegion);
		else
			glRectangleFill(Position.Inflate(-BorderWidth), _BackColor).Draw(invalidRegion);

		for (P_t i = 0; i < BorderWidth; i++)
		{
			glRectangleRound(Position.Inflate(-i), 2*BorderWidth - i, BorderColor).Draw(invalidRegion);
		}
		
		ImInvalidated = false;
	}
	
	gl2DPoint_t InvalidRegion() override
	{
		return Position.Inflate(-BorderWidth);
	}

	P_t BorderWidth = 0;
	glColor_t _BackColor = { 0, 0, 255 };
	glColor_t _PressedColor = { 0, 255, 0 };
	glColor_t BorderColor = { 255, 255, 255 };
	bool Click=false, Pressed = false;
};