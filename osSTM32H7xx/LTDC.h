#pragma once

#include "stm32h7xx.h"
#include <GPIO.h>

class hwLTDC
{
public:
	hwLTDC()
	{
	}
	void Init(void *pRGBData1, void *pRGBData2);
	
private:

	// 3. Configure the synchronous timings: VSYNC, HSYNC, vertical and horizontal back
	// porch, active data area and the front porch timings following the panel datasheet 
	// as described in the Section 33.4.1.
	void SynchronousTimings();
	// 4. Configure the synchronous signals and clock polarity in the LTDC_GCR register.
	// Configure the HS, VS, DE and PC polarity 
	void ClockPolarity();
	// 5. If needed, configure the background color in the LTDC_BCCR register.
	void BackgroundColor(uint8_t red, uint8_t blue, uint8_t green);

	// 7. Configure the layer1 / 2 parameters by :
	// 8. Enable layer1 / 2 and if needed the CLUT in the LTDC_LxCR register.
	void Layer(R_t<uint8_t, 1, 2> layer, void *pRGBData);

#include <TFT_Setup.h>

};
