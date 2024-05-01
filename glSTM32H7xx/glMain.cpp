
#include "glMain.h"
#include "MemoryDefs.h"

glColor_t RainbowColorGradient(float value) 
{
	glColor_t rgb;
	rgb.R = sin(value) * 127 + 128;
	rgb.G = sin(value + 2 * M_PI / 3) * 127 + 128;
	rgb.B = sin(value + 4 * M_PI / 3) * 127 + 128;
	rgb.A = 255;

	return rgb;
}



