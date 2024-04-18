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
glARGB_t *glObjectAdr::pVM; 
// where to draw on graphic buffer
P_t glObjectAdr::yStart, glObjectAdr::yEnd;
// width & height of graphic buffer
P_t glObjectAdr::Width, glObjectAdr::Height; 


void glPlotLine::PlotLineLow(P_t x0, P_t y0, P_t x1, P_t y1, glARGB_t &color)
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
		pVM[x0 + y0* Width] = color;
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
void glPlotLine::PlotLineHigh(P_t x0, P_t y0, P_t x1, P_t y1, glARGB_t &color)
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
		pVM[x0 + y0* Width] = color;
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
void glPlotLine::PlotVLine(P_t x0, P_t y0, P_t y1, glARGB_t &color)
{
	while (y0 < y1)
	{
		pVM[x0 + y0* Width] = color;
		y0++;
	}
}
void glPlotLine::PlotHLine(P_t x0, P_t x1, P_t y0, glARGB_t &color)
{
	while (x0 < x1)
	{
		pVM[x0 + y0* Width] = color;
		x0++;
	}
}
void glPlotLine::PlotLine(P_t x0, P_t y0, P_t x1, P_t y1, glARGB_t color)
{
	if (x0 == x1)
	{
		if (y0 <= y1)
			PlotVLine(x0, y0, y1, color);
		else
			PlotVLine(x0, y1, y0, color);
		return;
	}
	if (y0 == y1)
	{
		if (x0 <= x1)
			PlotHLine(x0, x1, y0, color);
		else
			PlotHLine(x1, x0, y0, color);
		return;
	}
	if (abs(y1 - y0) < abs(x1 - x0))
	{
		if (x0 > x1) PlotLineLow(x1, y1, x0, y0, color);
		else PlotLineLow(x0, y0, x1, y1, color);
		return;
	}
	if (y0 > y1) PlotLineHigh(x1, y1, x0, y0, color);
	else PlotLineHigh(x0, y0, x1, y1, color);
}