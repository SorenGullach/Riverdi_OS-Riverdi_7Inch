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

#include "glTypes.h"
#include "assert.h"

class glObjectAdr
{
public:
	// pointer to graphic buffer; 
	static glARGB_t *pVM; 
	// where to draw on graphic buffer
	static P_t yStart, yEnd;
	// width & height of graphic buffer
	static P_t Width, Height; 
};

// gl object base type
class glObject
{
public:
	// called down link to redraw
	void Update()
	{
		// draw this object
		if (ImInvalidated)
			Redraw();
		// Update child
		if (pNext != NULL)
			pNext->Update();
	}

	// add a child
	void Add(glObject &obj)
	{
		pNext = &obj;
		obj.pPrev = this;
	}
	
	virtual bool IsInvalidated()
	{
		if (ImInvalidated)
			return true; // all childern is also invalid
		if (pNext != NULL)
			return pNext->IsInvalidated();
		
		return false; // not invalidated
	}
	void Invalidate()
	{
		ImInvalidated = true;
		if (pNext != NULL)
			pNext->Invalidate();
	}
protected:
	// redraw your self
	virtual void Redraw() = 0;
	
	// is object invalidated
	bool ImInvalidated = true;

	// linking
	glObject *pNext = NULL, *pPrev = NULL;

};

class glPixel : public glPoint_t, public glColor_t, public glObject, protected glObjectAdr
{
public:
	glPixel() {}
	glPixel(P_t x, P_t y, glColor_t &color) 
		: glPoint_t(x, y)
		, glColor_t(color) {}
	glPixel(glPoint_t &point, glColor_t &color)
		: glPoint_t(point)
		, glColor_t(color) {}
	
	// redraw your self
	virtual void Redraw() override
	{
		assert(ImInvalidated && "Wrong call from somewhere !!");
		if (glPoint_t::Y < yStart && glPoint_t::Y < yEnd)
		{
			pVM[glPoint_t::Pos(Width)] = (glARGB_t)*this;
			ImInvalidated = false;
		}
	}
};

// help class
class glPlotLine : protected glObjectAdr
{
public:
	void PlotLine(P_t x0, P_t y0, P_t x1, P_t y1, glARGB_t color);
private:
	// Bresenham's line algorithm
	// https://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm
	void PlotLineLow(P_t x0, P_t y0, P_t x1, P_t y1, glARGB_t &color);
	void PlotLineHigh(P_t x0, P_t y0, P_t x1, P_t y1, glARGB_t &color);
	void PlotVLine(P_t x0, P_t y0, P_t y1, glARGB_t &color);
	void PlotHLine(P_t x0, P_t x1, P_t y0, glARGB_t &color);
};

class glLine : public glPlotLine, public glColor_t, public glObject
{
public:
	glLine() {}
	glLine(glPoint_t &start, glPoint_t &end, glColor_t &color)
		: glColor_t(color)
		, Start(start)
		, End(end) { }
	glLine(P_t x0, P_t y0, P_t x1, P_t y1, glColor_t &color)
		: glColor_t(color)
		, Start(x0, y0)
		, End(x1, y1) { }

	glPoint_t Start, End;
	
	// redraw your self
	virtual void Redraw() override
	{
		assert(ImInvalidated && "Wrong call from somewhere !!");
		
		PlotLine(Start.X, Start.Y, End.X, End.Y, (glARGB_t)*this);
		ImInvalidated = false;
	}

};
	
class glRectangle : public glColor_t, public glObject
{
public:
	glRectangle() {}
	glRectangle(glPoint_t &start, glPoint_t &end, glColor_t &color)
		: glColor_t(color)
		, Start(start)
		, End(end) {}
	glRectangle(P_t left, P_t top, P_t right, P_t bottom, glColor_t &color)
		: glColor_t(color)
		, Start(left,top)
		, End(right,bottom) {}
	
	glPoint_t Start, End;
	
	// redraw your self
	virtual void Redraw() override
	{
		assert(ImInvalidated && "Wrong call from somewhere !!");
		
		L.PlotLine(Start.X, Start.Y, End.X, Start.Y, (glARGB_t)*this);
		L.PlotLine(End.X, Start.Y, End.X, End.Y, (glARGB_t)*this);
		L.PlotLine(End.X, End.Y, Start.X, End.Y, (glARGB_t)*this);
		L.PlotLine(Start.X, End.Y, Start.X, Start.Y, (glARGB_t)*this);
		ImInvalidated = false;
	}
private:
	glPlotLine L;
};

class glCircle : public glObject
{
public:
	glCircle() {};
	glCircle(P_t x, P_t y, P_t r, glColor_t &color) 
		: Center(x, y)
		, Radius(r)
		, Color(color) {};

	glPoint_t Center;
	P_t Radius;
	glColor_t Color;
	operator glARGB_t()	{ return Color; }
	
	// redraw your self
	virtual void Redraw() override
	{
		assert(ImInvalidated && "Wrong call from somewhere !!");
		
//		PlotCircle();
		ImInvalidated = false;
	}
	
};