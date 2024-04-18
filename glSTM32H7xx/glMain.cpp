
#include "glMain.h"
#include "MemoryDefs.h"

void glMain::cbBlanking() 
{
	if (!IsInvalidated())
		return;
	
	yEnd = yStart + ls;
	if (yEnd >= Height)
		yEnd = Height;
	glObject::Update();
	yStart = yEnd;
	if (yStart >= Height)
		yStart = 0;
	
//	Display.CurrentPos(cx,cy);

	if (Display.BlankingOverrun() && ls>10)
		ls /= 2;
	else if(ls<Height/2)
		ls++;
}

glColor_t RainbowColorGradient(float value) 
{
	glColor_t rgb;
	rgb.R = sin(value) * 127 + 128;
	rgb.G = sin(value + 2 * M_PI / 3) * 127 + 128;
	rgb.B = sin(value + 4 * M_PI / 3) * 127 + 128;
	rgb.A = 255;

	return rgb;
}



