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

#include <Utils.h>
#include "glTypes.h"
#include "assert.h"

// helper class for plotting on the Video memory
class glVideoMemoryPlot : protected glVideoMemory
{
public:
	/*
	void Plot(P_t x, P_t y, const glARGB_t &color)	
	{ 
		assert(x < Width && y < Height);
		pVM[x + y*Width] = color; 
	}
	void Plot(const glPoint_t &p, const glARGB_t &color)	{ Plot(p.X, p.Y, color); }
	*/
	void Plot(const gl2DPoint_t &box, P_t x, P_t y, const glARGB_t &color)	
	{ 
		if (box.IsInside(x, y))
		{
			assert(x < Width && y < Height);
			pVM[x + y*Width] = color; 
		}
	}
	void Plot(const gl2DPoint_t &box, const glPoint_t &p, const glARGB_t &color)	{ Plot(box, p.X, p.Y, color); }
};

// gl object to report events
class glEvent
{
public:
	enum class eEventType
	{
		None,
		Slide,
		Click
	} EventType = eEventType::None;

	union uEvent
	{
		struct
		{
			sTPPoint::eSlideAction Action;
		} Slide;
		struct
		{
			P_t X, Y;
		} Click;
	} U;
};

/////////////////////////////////////////////////////////////////////////////////////////////
template<class T>
	class glChain;

class glLink
{
	// Allow glLink to access pNext and pPrev directly
	template<class T>
		friend class glChain;
public:
	//virtual void Init() = 0;

	virtual void UpdateLook(gl2DPoint_t &invalidRegion) = 0;
	virtual void UpdateState(sTPPoint &point) = 0;
	virtual bool IsInvalidated(gl2DPoint_t &invalidRegion) = 0;
	
	// buble event to top
	virtual void Event(glEvent event) = 0;
	
	glLink *Prev() { return pPrev; }
	glLink *Next() { return pNext; }

protected:
	glLink *pNext = nullptr, *pPrev = nullptr;
};

/////////////////////////////////////////////////////////////////////////////////////////////
// A double linked list
template<class T>
	class glChain
	{
	public:
		// Add a object
		void Add(T *obj)
		{
			if (!obj) return; // Check for nullptr
			
			glLink* pNewLinkObj = dynamic_cast<glLink*>(obj); // Attempt to cast to glLinkObject pointer
			if (!pNewLinkObj) 
			{
				assert(false && "Error: Object does not derive from glLinkObject. Cannot add to glLink.");
				return;
			}

			 // Update pointers
			if (!pHead) 
			{
				pHead = pNewLinkObj;
				pTail = pNewLinkObj;
			}
			else 
			{
				pTail->pNext = pNewLinkObj;
				pNewLinkObj->pPrev = pTail;
				pTail = pNewLinkObj;
			}
		}

		// Function to remove an object from the linked list
		void Remove(T *obj) 
		{
			if (!obj) return; // Check for nullptr

			glLink* pLinkObj = dynamic_cast<glLink*>(obj); // Attempt to cast to glLinkObject pointer
			if (!pLinkObj) {
				assert(false && "Error: Object does not derive from glLinkObject. Cannot remove from glLink.");
				return;
			}

			// Search for the object in the list
			glLink* pCurrent = pHead;
			while (pCurrent) {
				if (pCurrent == pLinkObj) {
					// Update pointers
					if (pCurrent->pPrev) {
						pCurrent->pPrev->pNext = pCurrent->pNext;
					}
					else {
						pHead = pCurrent->pNext;
					}
					if (pCurrent->pNext) {
						pCurrent->pNext->pPrev = pCurrent->pPrev;
					}
					else {
						pTail = pCurrent->pPrev;
					}
					delete pCurrent; // Free memory
					return;
				}
				pCurrent = pCurrent->pNext;
			}

			// If the object is not found
			assert(false && "Error: Object not found in glLink. Cannot remove.");
		}
		
		T *Head() {	return static_cast<T*>(pHead); }
		T *Tail() { return static_cast<T*>(pTail); }
	private:
		glLink* pHead = nullptr;
		glLink* pTail = nullptr;		
	};

/////////////////////////////////////////////////////////////////////////////////////////////
class glObjectAttributes 
{
public:
	glObjectAttributes(char const *name, gl2DPoint_t position, glColor_t color)
	{
		Name = name;
		Position = position;
		Color = color;
	}

	virtual gl2DPoint_t InvalidRegion() 
	{
		return Position;
	}

	// redraw your self
	virtual void Redraw(const gl2DPoint_t &invalidRegion) = 0;
	// Called upon touch change
	virtual void Touch(const sTPPoint &) = 0;

	// is object invalidated
	bool ImInvalidated = true;

protected:
	// attributes
	glColor_t Color;
	char const *Name = "glObject";
	gl2DPoint_t Position;
};

/*
/////////////////////////////////////////////////////////////////////////////////////////////
//class glMain;
// gl object base type
class glObject1 : public glObjectAttributes
{
	//friend glMain;
public:
	glObject(char const *name)
		: glObjectAttributes(name)
	{}
	
	// called down link to redraw
	void UpdateLook(gl2DPoint_t &invalidRegion)
	{
		Redraw(invalidRegion); // draw this object

		// Update next child
		if (pNextChild != nullptr)
			pNextChild->UpdateLook(invalidRegion);

		// Update next object
//		if (pNextObj != nullptr)
	//		pNextObj->UpdateLook(invalidRegion);
	}

	// called down link to update state
	void UpdateState(sTPPoint &point)
	{
		Touch(point); // send touch

		// Update next child
		if (pNextChild != nullptr)
			pNextChild->UpdateState(point);

		// Update next object
	//	if (pNextObj != nullptr)
		//	pNextObj->UpdateState(point);
	}

	// find last object
	glObject &LastSibling()
	{
		glObject *pObj = this;
		while (pObj->pNextSibling != nullptr)
			pObj = pObj->pNextSibling;
		return *pObj;
	}
	glObject &LastChild()
	{
		glObject *pObj = this;
		while (pObj->pNextChild != nullptr)
			pObj = pObj->pNextChild;
		return *pObj;
	}
	
	// add a next object
	void AddSibling(glObject *obj)
	{
		obj->pPrevSibling = this;
		obj->pNextSibling = pNextSibling;
		if (pNextSibling == nullptr)
		{
			pNextSibling = obj;
		}
		else
		{
			pNextSibling->pPrevSibling = obj;
			pNextSibling = obj;
		}
		obj->Init();
	}
	// add a child object
	void AddChild(glObject *obj)
	{
		obj->pPrevChild = this;
		obj->pNextChild = pNextChild;
		if (pNextChild == nullptr)
		{
			pNextChild = obj;
		}
		else
		{
			pNextChild->pPrevChild = obj;
			pNextChild = obj;
		}
		obj->Init();
	}

	virtual gl2DPoint_t InvalidRegion() 
	{
		return Position;
	}
	bool IsInvalidated(gl2DPoint_t &invalidRegion)
	{
		bool Invalid = false;
		if (ImInvalidated)
		{
			invalidRegion = invalidRegion.Union(InvalidRegion());
			Invalid = true; // all childern is also invalid
		}
		else if (pNextChild != nullptr)
		{
			// ask all childs
			Invalid = pNextChild->IsInvalidated(invalidRegion);
		}
		
		// always ask siblings
		if (pNextSibling != nullptr)
			Invalid |= pNextSibling->IsInvalidated(invalidRegion);
		
		return Invalid;
	}
	void InvalidatePage()
	{
		//Printf("%s Invalidate\n", Name);
		ImInvalidated = true;
		if (pNextChild != nullptr)
			pNextChild->Invalidate();
	}
	void Invalidate()
	{
		//Printf("%s Invalidate\n", Name);
		ImInvalidated = true;
		if (pNextChild != nullptr)
			pNextChild->Invalidate();
	}

	glEvent EventAction;
protected:
	// Initialize object
	virtual void Init() = 0;
	// redraw your self
	// return true if anything done
	virtual void Redraw(const gl2DPoint_t &invalidRegion) = 0;
	// Called upon touch change
	virtual void Touch(const sTPPoint &) = 0;

	// buble event to top
	virtual void Event(glEvent event)
	{
		if (pPrevChild != nullptr)
			pPrevChild->Event(event);
		else
		{
			Printf("%s Event\n", Name);
			EventAction = event;
		}
	}

	// is object invalidated
	bool ImInvalidated = true;

	// linking
	glObject *pNextSibling = nullptr, *pPrevSibling = nullptr;
	glObject *pNextChild = nullptr, *pPrevChild = nullptr;
};
*/
/////////////////////////////////////////////////////////////////////////////////////////////
class glObject : public glObjectAttributes, public glLink
{
public:
	glObject(char const *name, gl2DPoint_t position, glColor_t color)
		: glObjectAttributes(name, position, color) {}

	// called down link to redraw
	void UpdateLook(gl2DPoint_t &invalidRegion)
	{
		Redraw(invalidRegion); // draw this object

		// Update Childs
		if (ChainChilds.Head() != nullptr)
			ChainChilds.Head()->UpdateLook(invalidRegion);

		// Update objects
		if (pNext != nullptr)
			pNext->UpdateLook(invalidRegion);
	}

	// called down link to update state
	void UpdateState(sTPPoint &point)
	{
		Touch(point); // send touch

		// Update Childs
		if (ChainChilds.Head() != nullptr)
			ChainChilds.Head()->UpdateState(point);

		// Update objects
		if (pNext != nullptr)
			pNext->UpdateState(point);
	}
	
	void Invalidate()
	{
		//Printf("%s Invalidate\n", Name);
		ImInvalidated = true; // this obj
		if (ChainChilds.Head() != nullptr)
			ChainChilds.Head()->Invalidate(); // & all childs
	}
	
	bool IsInvalidated(gl2DPoint_t &invalidRegion)
	{
		bool Invalid = false;
		if (ImInvalidated)
		{
			invalidRegion = invalidRegion.Union(InvalidRegion());
			Invalid = true; // all childern is also invalid
		}
		else if (ChainChilds.Head() != nullptr)
		{
			// ask all childs
			Invalid = ChainChilds.Head()->IsInvalidated(invalidRegion);
		}
			
		if (pNext != nullptr)
		{
			// ask all objs
			Invalid |= pNext->IsInvalidated(invalidRegion);
		}
		return Invalid;
	}

	// buble event to top
	void Event(glEvent event)
	{
		if (pPrev != nullptr)
			pPrev->Event(event);
	}

private:
	glChain<glObject> ChainChilds;
};

/////////////////////////////////////////////////////////////////////////////////////////////
class glPixel : public glObject, private glVideoMemoryPlot
{
public:
	glPixel() 
		: glObject("Pixel", gl2DPoint_t(), glColor_t()) {}

	glPixel(P_t x, P_t y, glColor_t &color)
		: glObject("Pixel", gl2DPoint_t(x,y,x,y), color) {}

	glPixel(P_t x, P_t y, glColors color)
		: glObject("Pixel", gl2DPoint_t(x, y, x, y), glColor_t(color)) {}

	glPixel(glPoint_t &point, glColor_t &color)
		: glObject("Pixel", gl2DPoint_t(point, point), color) {}

	glPixel(glPoint_t &point, glColors color)
		: glObject("Pixel",	gl2DPoint_t(point,point), glColor_t(color)) {}
	
protected:
	// Initialize object
	//virtual void Init() override {}
	// redraw your self
	virtual void Redraw(const gl2DPoint_t &invalidRegion) override
	{
		//	if (!ImInvalidated) return false;
		
		Plot(invalidRegion, Position.LT, Color);
		ImInvalidated = false;
	}
	void Touch(const sTPPoint &) override {}
};

// help class
class glPlot2DHelper : public glVideoMemoryPlot
{
public:
	void PlotLine(gl2DPoint_t invalidRegion, P_t x0, P_t y0, P_t x1, P_t y1, glARGB_t &color);
	void PlotCircle(gl2DPoint_t invalidRegion, glPoint_t &center, P_t radius, glARGB_t &color);
	void PlotCircleFill(gl2DPoint_t invalidRegion, glPoint_t &center, P_t radius, glARGB_t &color);
	void PlotArc(gl2DPoint_t invalidRegion, gl2DPoint_t box, glPoint_t &center, P_t radius, glARGB_t &color);
	void PlotFillArc(gl2DPoint_t invalidRegion, gl2DPoint_t box, glPoint_t &center, P_t radius, glARGB_t &color);
private:
	void PlotLineLow(gl2DPoint_t invalidRegion, P_t x0, P_t y0, P_t x1, P_t y1, glARGB_t &color);
	void PlotLineHigh(gl2DPoint_t invalidRegion, P_t x0, P_t y0, P_t x1, P_t y1, glARGB_t &color);
	void PlotVLine(gl2DPoint_t invalidRegion, P_t x0, P_t y0, P_t y1, glARGB_t &color);
	void PlotHLine(gl2DPoint_t invalidRegion, P_t x0, P_t x1, P_t y0, glARGB_t &color);
};

class glLine : private glPlot2DHelper, public glObject
{
public:
	glLine()
		: glObject("Line", gl2DPoint_t(), glColor_t()) {}
	
	glLine(glPoint_t &start, glPoint_t &end, glColor_t &color)
		: glObject("Line", gl2DPoint_t(start,end), color) {}

	glLine(glPoint_t &start, glPoint_t &end, glColors color)
		: glObject("Line", gl2DPoint_t(start, end), glColor_t(color)) {}

	glLine(P_t x0, P_t y0, P_t x1, P_t y1 , glColor_t &color)
		: glObject("Line", gl2DPoint_t(x0,y0,x1,y1), color) {}

	glLine(P_t x0, P_t y0, P_t x1, P_t y1, glColors color)
		: glObject("Line", gl2DPoint_t(x0, y0, x1, y1), glColor_t(color)) {}

protected:
	// Initialize object
//	virtual void Init() override {}
	// redraw your self 
	// return true if anything done
	virtual void Redraw(const gl2DPoint_t &invalidRegion) override
	{
		//	if (!ImInvalidated) return false;
		
		PlotLine(invalidRegion, Position.L(), Position.T(), Position.R(), Position.B(), Color);
		ImInvalidated = false;
	}
	void Touch(const sTPPoint &) override {}
};
	
class glRectangle : protected glPlot2DHelper, public glObject
{
public:
	glRectangle()
		: glObject("Rectangle", gl2DPoint_t(), glColor_t()) {}

	glRectangle(gl2DPoint_t rect, glColor_t &color)
		: glObject("Rectangle", rect, color) {}

	glRectangle(gl2DPoint_t rect, glColors color)
		: glObject("Rectangle", rect, glColor_t(color)) {}

	glRectangle(glPoint_t start, glPoint_t end, glColor_t &color)
		: glObject("Rectangle", gl2DPoint_t(start, end), color) {}

	glRectangle(glPoint_t start, glPoint_t end, glColors color)
		: glObject("Rectangle", gl2DPoint_t(start, end), glColor_t(color)) {}

	glRectangle(P_t l, P_t t, P_t r, P_t b, glColor_t &color)
		: glObject("Rectangle", gl2DPoint_t(l,t,r,b), color) {}

	glRectangle(P_t l, P_t t, P_t r, P_t b, glColors color)
		: glObject("Rectangle", gl2DPoint_t(l, t, r, b), glColor_t(color)) {}


	virtual void Draw(const gl2DPoint_t &invalidRegion)
	{
		PlotLine(invalidRegion, Position.LT.X, Position.LT.Y, Position.RB.X, Position.LT.Y, Color);
		PlotLine(invalidRegion, Position.RB.X, Position.LT.Y, Position.RB.X, Position.RB.Y, Color);
		PlotLine(invalidRegion, Position.RB.X, Position.RB.Y, Position.LT.X, Position.RB.Y, Color);
		PlotLine(invalidRegion, Position.LT.X, Position.RB.Y, Position.LT.X, Position.LT.Y, Color);
	}

	void Inflate(int d)
	{
		Position = Position.Inflate(d);		
	}
protected:
	// Initialize object
//	virtual void Init() override {}
	// redraw your self
	// return true if anything done
	virtual void Redraw(const gl2DPoint_t &invalidRegion) override
	{
		//	if (!ImInvalidated) return false;

		Draw(invalidRegion);
		
		ImInvalidated = false;
	}
	void Touch(const sTPPoint &) override {}
};

class glRectangleFill : public glRectangle
{
public:
	glRectangleFill(
		gl2DPoint_t rect, 
		glColor_t &color)
		: glRectangle(rect, color) { Name = "RectangleFill"; }
	glRectangleFill(
		gl2DPoint_t rect, 
		glColors color)
		: glRectangle(rect, color) { Name = "RectangleFill"; }
	glRectangleFill(
		glPoint_t &start, 
		glPoint_t &end, 
		glColor_t &color)
		: glRectangle(start, end, color) { Name = "RectangleFill"; }
	glRectangleFill(
		glPoint_t &start, 
		glPoint_t &end, glColors color)
		: glRectangle(start, end, color) { Name = "RectangleFill"; }
	glRectangleFill(P_t left, P_t top, P_t right, P_t bottom, glColor_t &color)
		: glRectangle(left, top, right, bottom, color) { Name = "RectangleFill"; }
	glRectangleFill(P_t left, P_t top, P_t right, P_t bottom, glColors color)
		: glRectangle(left, top, right, bottom, color) { Name = "RectangleFill"; }

	void Draw(const gl2DPoint_t &invalidRegion) override
	{
		for (P_t x = Position.LT.X; x <= Position.RB.X; x++)
			for (P_t y = Position.LT.Y; y <= Position.RB.Y; y++)
				Plot(invalidRegion, x, y, Color);
	}
};

class glCircle : protected glPlot2DHelper, public glObject
{
public:
	glCircle()
		: glObject("Circel", gl2DPoint_t(), glColor_t()) {}
	glCircle(P_t x, P_t y, P_t radius, glColor_t &color) 
		: glObject("Circel", gl2DPoint_t(x - radius, y - radius, x + radius, y + radius), color)
		, Center(x, y)
		, Radius(radius) {}
	glCircle(P_t x, P_t y, P_t radius, glColors color) 
		: glObject("Circel", gl2DPoint_t(x - radius, y - radius, x + radius, y + radius), glColor_t(color))
		, Center(x, y)
		, Radius(radius) {}

	glCircle(glPoint_t &center, P_t radius, glColor_t &color) 
		: glObject("Circel", gl2DPoint_t(center.Move(-radius, -radius), center.Move(radius, radius)), color)
		, Center(center)
		, Radius(radius) {}

	glCircle(glPoint_t &center, P_t radius, glColors color) 
		: glObject("Circel", gl2DPoint_t(center.Move(-radius, -radius), center.Move(radius, radius)),glColor_t(color))
		, Center(center)
		, Radius(radius) {}

	virtual void Draw(const gl2DPoint_t &invalidRegion)
	{
		PlotCircle(invalidRegion, Center, Radius, Color);
	}

protected:
	// Initialize object
//	virtual void Init() override {}
	// redraw your self
	// return true if anything done
	virtual void Redraw(const gl2DPoint_t &invalidRegion) override
	{
		//	if (!ImInvalidated) return false;
		
		Draw(invalidRegion);
		
		ImInvalidated = false;
	}
	void Touch(const sTPPoint &) override {}

	glPoint_t Center;
	P_t Radius;
};

class glCircleFill : public glCircle
{
public:
	glCircleFill(P_t x, P_t y, P_t radius, glColor_t &color) 
		: glCircle(x, y, radius, color) { Name = "CircleFill"; }
	glCircleFill(P_t x, P_t y, P_t radius, glColors color) 
		: glCircle(x, y, radius, color) { Name = "CircleFill"; }

	glCircleFill(glPoint_t &center, P_t radius, glColor_t &color) 
		: glCircle(center, radius, color) {Name = "CircleFill"; }
	glCircleFill(glPoint_t &center, P_t radius, glColors color) 
		: glCircle(center, radius, color) {Name = "CircleFill"; }

	void Draw(const gl2DPoint_t &invalidRegion) override
	{
		PlotCircleFill(invalidRegion, Center, Radius, Color);
	}
};

class glArc : public glCircle
{
public:
	glArc() { Name = "glArc"; }
	glArc(gl2DPoint_t box, glPoint_t center, P_t radius, glColor_t &color) 
		: glCircle(center, radius, color)
		{ Position = box; Name = "glArc"; }
	glArc(gl2DPoint_t box, glPoint_t center, P_t radius, glColors color) 
		: glCircle(center, radius, color)
		{ Position = box; Name = "glArc"; }
	
	virtual void Draw(const gl2DPoint_t &invalidRegion) override
	{
		PlotArc(invalidRegion, Position, Center, Radius, Color);
	}
};

class glArcFill : public glArc
{
public:
	glArcFill() { Name = "glArcFill"; }
	glArcFill(gl2DPoint_t box, glPoint_t center, P_t radius, glColor_t &color) 
		: glArc(box, center, radius, color) {}
	glArcFill(gl2DPoint_t box, glPoint_t center, P_t radius, glColors color) 
		: glArc(box, center, radius, color) {}

	void Draw(const gl2DPoint_t &invalidRegion) override
	{
		PlotFillArc(invalidRegion, Position, Center, Radius, Color);
	}
};

class glRectangleRound : public glRectangle
{
public:
	glRectangleRound(gl2DPoint_t rect, P_t radius, glColor_t &color)
		: glRectangle(rect, color) { Radius = radius; }

	glRectangleRound(gl2DPoint_t rect, P_t radius, glColors color)
		: glRectangle(rect, color) { Radius = radius; }

	glRectangleRound(glPoint_t &start, glPoint_t &end, P_t radius, glColor_t &color)
		: glRectangle(start, end, color) { Radius = radius; }

	glRectangleRound(glPoint_t &start, glPoint_t &end, P_t radius, glColors color)
		: glRectangle(start, end, color) { Radius = radius; }
	
	glRectangleRound(P_t left, P_t top, P_t right, P_t bottom, P_t radius, glColor_t &color)
		: glRectangle(left, top, right, bottom, color) { Radius = radius; }
	
	glRectangleRound(P_t left, P_t top, P_t right, P_t bottom, P_t radius, glColors color)
		: glRectangle(left, top, right, bottom, color) { Radius = radius; }

	virtual void Draw(const gl2DPoint_t &invalidRegion)
	{
		assert(Radius <= Position.Width() && Radius <= Position.Height());
		PlotLine(invalidRegion, Position.L() + Radius, Position.T(), Position.R() - Radius, Position.T(), Color);
		PlotLine(invalidRegion, Position.R(), Position.T() + Radius, Position.R(), Position.B() - Radius, Color);
		PlotLine(invalidRegion, Position.R() - Radius, Position.B(), Position.L() + Radius, Position.B(), Color);
		PlotLine(invalidRegion, Position.L(), Position.B() - Radius, Position.L(), Position.T() + Radius, Color);

		gl2DPoint_t b(Position.L(), Position.T(), Position.L() + Radius, Position.T() + Radius);
		glPoint_t p(Position.L() + Radius, Position.T() + Radius);
		glArc(b, p, Radius, Color).Draw(invalidRegion);

		b = gl2DPoint_t(Position.R() - Radius, Position.T(), Position.R(), Position.T() + Radius);
		p = glPoint_t(Position.R() - Radius, Position.T() + Radius);
		glArc(b, p, Radius, Color).Draw(invalidRegion);

		b = gl2DPoint_t(Position.R() - Radius, Position.B() - Radius, Position.R(), Position.B());
		p = glPoint_t(Position.R() - Radius, Position.B() - Radius);
		glArc(b, p, Radius, Color).Draw(invalidRegion);

		b = gl2DPoint_t(Position.L(), Position.B() - Radius, Position.L() + Radius, Position.B());
		p = glPoint_t(Position.L() + Radius, Position.B() - Radius);
		glArc(b, p, Radius, Color).Draw(invalidRegion);
	}
private:
	P_t Radius = 5;
};

class glRectangleRoundFill : public glRectangle
{
public:
	glRectangleRoundFill(gl2DPoint_t rect, P_t radius, glColor_t &color)
		: glRectangle(rect, color) { Name = "RectangleRoundFill"; Radius = radius; }

	glRectangleRoundFill(gl2DPoint_t rect, P_t radius, glColors color)
		: glRectangle(rect, color) { Radius = radius; }

	glRectangleRoundFill(glPoint_t &start, glPoint_t &end, P_t radius, glColor_t &color)
		: glRectangle(start, end, color) { Name = "RectangleRoundFill"; Radius = radius; }

	glRectangleRoundFill(glPoint_t &start, glPoint_t &end, P_t radius, glColors color)
		: glRectangle(start, end, color) { Name = "RectangleRoundFill"; Radius = radius; }

	glRectangleRoundFill(P_t left, P_t top, P_t right, P_t bottom, P_t radius, glColor_t &color)
		: glRectangle(left, top, right, bottom, color) { Name = "RectangleRoundFill"; Radius = radius; }

	glRectangleRoundFill(P_t left, P_t top, P_t right, P_t bottom, P_t radius, glColors color)
		: glRectangle(left, top, right, bottom, color) { Name = "RectangleRoundFill"; Radius = radius; }

	virtual void Draw(const gl2DPoint_t &invalidRegion)
	{
		assert(Radius <= Position.Width() && Radius <= Position.Height());
		glRectangleFill(Position.L(), Position.T() + Radius, Position.R(), Position.B() - Radius, Color).Draw(invalidRegion);
		
		glRectangleFill(Position.L() + Radius, Position.T(), Position.R() - Radius, Position.T() + Radius, Color).Draw(invalidRegion);
		glRectangleFill(Position.L() + Radius, Position.B() - Radius, Position.R() - Radius, Position.B(), Color).Draw(invalidRegion);
	
		gl2DPoint_t b(Position.L(), Position.T(), Position.L() + Radius, Position.T() + Radius);
		glPoint_t p(Position.L() + Radius, Position.T() + Radius);
		glArcFill(b, p, Radius, Color).Draw(invalidRegion);

		b = gl2DPoint_t(Position.R() - Radius, Position.T(), Position.R(), Position.T() + Radius);
		p = glPoint_t(Position.R() - Radius, Position.T() + Radius);
		glArcFill(b, p, Radius, Color).Draw(invalidRegion);

		b = gl2DPoint_t(Position.R() - Radius, Position.B() - Radius, Position.R(), Position.B());
		p = glPoint_t(Position.R() - Radius, Position.B() - Radius);
		glArcFill(b, p, Radius, Color).Draw(invalidRegion);

		b = gl2DPoint_t(Position.L(), Position.B() - Radius, Position.L() + Radius, Position.B());
		p = glPoint_t(Position.L() + Radius, Position.B() - Radius);
		glArcFill(b, p, Radius, Color).Draw(invalidRegion);
	}
private:
	P_t Radius = 5;
};

