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

#include "glPrimitives.h"

// pointer to graphic buffer; 
glARGB_t *glVideoMemory::pVM; 
// where to draw on graphic buffer
//P_t glObjectAdr::yStart, glObjectAdr::yEnd;
// width & height of graphic buffer
P_t glVideoMemory::Width, glVideoMemory::Height;

void glPlot2DHelper::PlotLineLow(gl2DPoint_t invalidRegion, P_t x0, P_t y0, P_t x1, P_t y1, glARGB_t &color)
{
	int dx = x1 - x0;
	int dy = y1 - y0;
	int yi = 1;
	if (dy < 0)
	{
		yi = -1;
		dy = -dy;
	}
	int D = 2*dy - dx;

	while (x0 < x1)
	{
		Plot(invalidRegion, x0, y0, color);
		if (D > 0)
		{
			y0 += yi;
			D += 2*(dy - dx);
		}
		else
			D += 2*dy;
		x0++;
	}
}
void glPlot2DHelper::PlotLineHigh(gl2DPoint_t invalidRegion, P_t x0, P_t y0, P_t x1, P_t y1, glARGB_t &color)
{
	int dx = x1 - x0;
	int dy = y1 - y0;
	int xi = 1;
	if (dx < 0)
	{
		xi = -1;
		dx = -dx;
	}
	int D = 2*dx - dy;

	while (y0 < y1)
	{
		Plot(invalidRegion, x0, y0, color);
		if (D > 0)
		{
			x0 += xi;
			D += 2*(dx - dy);
		}
		else
			D += 2*dx;
		y0++;
	}
}
void glPlot2DHelper::PlotVLine(gl2DPoint_t invalidRegion, P_t x0, P_t y0, P_t y1, glARGB_t &color)
{
	while (y0 < y1)
	{
		Plot(invalidRegion, x0, y0, color);
		y0++;
	}
}
void glPlot2DHelper::PlotHLine(gl2DPoint_t invalidRegion, P_t x0, P_t x1, P_t y0, glARGB_t &color)
{
	while (x0 < x1)
	{
		Plot(invalidRegion, x0, y0, color);
		x0++;
	}
}
// Bresenham's line algorithm
// https://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm
void glPlot2DHelper::PlotLine(gl2DPoint_t invalidRegion, P_t x0, P_t y0, P_t x1, P_t y1, glARGB_t &color)
{
	if (x0 == x1)
	{
		if (y0 <= y1)
			PlotVLine(invalidRegion, x0, y0, y1, color);
		else
			PlotVLine(invalidRegion, x0, y1, y0, color);
		return;
	}
	if (y0 == y1)
	{
		if (x0 <= x1)
			PlotHLine(invalidRegion, x0, x1, y0, color);
		else
			PlotHLine(invalidRegion, x1, x0, y0, color);
		return;
	}
	if (abs(y1 - y0) < abs(x1 - x0))
	{
		if (x0 > x1) PlotLineLow(invalidRegion, x1, y1, x0, y0, color);
		else PlotLineLow(invalidRegion, x0, y0, x1, y1, color);
		return;
	}
	if (y0 > y1) PlotLineHigh(invalidRegion, x1, y1, x0, y0, color);
	else PlotLineHigh(invalidRegion, x0, y0, x1, y1, color);
}
/*
// https://web.archive.org/web/20120422045142/https://banu.com/blog/7/drawing-circles/
void glPlot2DHelper::PlotCircle(glPoint_t &center, P_t radius, glARGB_t color)
{
	// cos pi/4 = 185363 / 2^18 (approx) 
	int l = ((radius * 185363) >> 18)+1;
	
	// At x=0, y=radius 
	int y = radius; 
	
	int y2;
	int r2 = y2 = y * y;
	int ty = (2 * y) - 1;
	int y2_new = r2 + 3;
	
	for (P_t x = 0; x <= l; x++) 
	{
		y2_new -= (2 * x) - 3;

		if ((y2 - y2_new) >= ty) {
			y2 -= ty;
			y -= 1;
			ty -= 2;
		}
		
		Plot(center.X + x, center.Y + y, color);
		Plot(center.X + x, center.Y - y, color);
		Plot(center.X - x, center.Y + y, color);
		Plot(center.X - x, center.Y - y, color);

		Plot(center.X + y, center.Y + x, color);
		Plot(center.X + y, center.Y - x, color);
		Plot(center.X - y, center.Y + x, color);
		Plot(center.X - y, center.Y - x, color);
	}
}
*/
// https://en.wikipedia.org/wiki/Midpoint_circle_algorithm
void glPlot2DHelper::PlotCircle(gl2DPoint_t invalidRegion, glPoint_t &center, P_t radius, glARGB_t &color)
{
/*
t1 = r / 16
x = r
y = 0
Repeat Until x < y
	Pixel (x, y) and all symmetric pixels are colored (8 times)
	y = y + 1
	t1 = t1 + y
	t2 = t1 - x
	If t2 >= 0
		t1 = t2
		x = x - 1	 
	*/

	int t2,t1 = radius / 16;
	int x = radius;
	int y = 0;
	while (x>=y)
	{
		Plot(invalidRegion, center.X + x, center.Y + y, color);
		Plot(invalidRegion, center.X + x, center.Y - y, color);
		Plot(invalidRegion, center.X - x, center.Y + y, color);
		Plot(invalidRegion, center.X - x, center.Y - y, color);

		Plot(invalidRegion, center.X + y, center.Y + x, color);
		Plot(invalidRegion, center.X + y, center.Y - x, color);
		Plot(invalidRegion, center.X - y, center.Y + x, color);
		Plot(invalidRegion, center.X - y, center.Y - x, color);
		y = y + 1;
		t1 = t1 + y;
		t2 = t1 - x;
		if(t2 >= 0)
		{
			t1 = t2;
			x = x - 1;
		}
	}
}

void glPlot2DHelper::PlotCircleFill(gl2DPoint_t invalidRegion, glPoint_t &center, P_t radius, glARGB_t &color)
{
	for (int y = -radius; y <= radius; y++)
		for (int x = -radius; x <= radius; x++)
			if ((x * x) + (y * y) <= (radius * radius))
				Plot(invalidRegion, center.X + x, center.Y + y, color);
}

void glPlot2DHelper::PlotArc(gl2DPoint_t invalidRegion, gl2DPoint_t box, glPoint_t &center, P_t radius, glARGB_t &color)
{
	int t2, t1 = radius / 16;
	int x = radius;
	int y = 0;
	gl2DPoint_t IVR = invalidRegion.Intersection(box);
	while (x >= y)
	{
		Plot(IVR, center.X + x, center.Y + y, color);
		Plot(IVR, center.X + x, center.Y - y, color);
		Plot(IVR, center.X - x, center.Y + y, color);
		Plot(IVR, center.X - x, center.Y - y, color);

		Plot(IVR, center.X + y, center.Y + x, color);
		Plot(IVR, center.X + y, center.Y - x, color);
		Plot(IVR, center.X - y, center.Y + x, color);
		Plot(IVR, center.X - y, center.Y - x, color);
		y = y + 1;
		t1 = t1 + y;
		t2 = t1 - x;
		if (t2 >= 0)
		{
			t1 = t2;
			x = x - 1;
		}
	}
}

void glPlot2DHelper::PlotFillArc(gl2DPoint_t invalidRegion, gl2DPoint_t box, glPoint_t &center, P_t radius, glARGB_t &color)
{
	gl2DPoint_t IVR = invalidRegion.Intersection(box);
	for (int y = -radius; y <= radius; y++)
		for (int x = -radius; x <= radius; x++)
			if ((x * x) + (y * y) <= (radius * radius))
				Plot(IVR, center.X + x, center.Y + y, color);
}


