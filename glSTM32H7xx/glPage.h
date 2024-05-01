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

// a GUI consists of pages
class glPage : public glObjectAttributes, public glLink
{
public:
	glPage(const char *name)
		: glObjectAttributes(name, gl2DPoint_t(), glColor_t())
	{
	}
//	virtual void Init() override
//	{
//	}

	// Add a object to the page
	void Add(glObject *page)
	{
		ChainObjects.Add(page);
	}

	void PosSize(gl2DPoint_t pos)
	{
		Position = pos;
	}
	// called down link to redraw
	void UpdateLook(gl2DPoint_t &invalidRegion)
	{
		Redraw(invalidRegion); // draw this object

		// Update objects
		if (ChainObjects.Head() != nullptr)
			ChainObjects.Head()->UpdateLook(invalidRegion);
	}

	// called down link to update state
	void UpdateState(sTPPoint &point)
	{
		Touch(point); // send touch

		// Update objects
		if (ChainObjects.Head() != nullptr)
			ChainObjects.Head()->UpdateState(point);
	}

	void Invalidate()
	{
		//Printf("%s Invalidate\n", Name);
		ImInvalidated = true; // this page
		if (ChainObjects.Head() != nullptr)
			ChainObjects.Head()->Invalidate(); // a all objects
	}
	
	bool IsInvalidated(gl2DPoint_t &invalidRegion)
	{
		bool Invalid = false;
		if (ImInvalidated)
		{
			invalidRegion = invalidRegion.Union(InvalidRegion());
			Invalid = true; // all childern is also invalid
		}
		else if (ChainObjects.Head() != nullptr)
		{
			// ask all objs
			Invalid = ChainObjects.Head()->IsInvalidated(invalidRegion);
		}
		return Invalid;
	}

	virtual void Redraw(const gl2DPoint_t &invalidRegion) final
	{
//		if (!IsInvalidated()) return false;
		
		glRectangleFill(Position.Intersection(invalidRegion), Color).Draw(invalidRegion);
		
		ImInvalidated = false;
	}
	virtual void Touch(const sTPPoint &point) final
	{
		if (InSlide)
		{
			if (point.TipAction == sTPPoint::eTipAction::Up)
				InSlide = false;
			Printf("%s %s\n", Name, InSlide ? "InSlide" : "");
			return;
		}
		if (point.SlideAction == sTPPoint::eSlideAction::Right && 
			EventAction.U.Slide.Action != sTPPoint::Right) 
		{
			InSlide = true;
			Printf("%s slide right\n", Name);
			EventAction.EventType = glEvent::eEventType::Slide;
			EventAction.U.Slide.Action = sTPPoint::Right;
			Event(EventAction);
		}
		else if (point.SlideAction == sTPPoint::eSlideAction::Left && 
			EventAction.U.Slide.Action != sTPPoint::Left) 
		{
			InSlide = true;
			Printf("%s slide left\n",Name);
			EventAction.EventType = glEvent::eEventType::Slide;
			EventAction.U.Slide.Action = sTPPoint::Left;
			Event(EventAction);
		}
	}
	// buble event to top
	void Event(glEvent event)
	{
		Printf("%s Event\n", Name);
		EventAction = event;
	}

	glEvent EventAction;
private:
	glChain<glObject> ChainObjects;
	bool InSlide = false;
};