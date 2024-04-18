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

#include <Utils.h>
#include <MemoryDefs.h>
#include <SysTick.h>
#include <RCC.h>
#include <PWR.h>
#include <FMC.h>
#include <GPIO.h>

#include <stdio.h>
#include <stdint.h>

// create an IO pin for an LED connected to GPIOJ, pin 10
hwGPIO Led(GPIOJ, 10, hwGPIO::eMode::Output, hwGPIO::ePP::PushPull);

// Uncomment to enable dual boot functionality
//#define DUALBOOT

// Function declaration for a function stored in ITCM (Instruction Tightly Coupled Memory)
__RAM_FUNC void SomeITCMFunc(int param)
{
	assert(/*(uint32_t)&SomeITCMFunc >= ITCMRAM_START &&*/ (uint32_t)&SomeITCMFunc < (ITCMRAM_END) && "Its not in ITCM area");
	Printf("SomeITCMFunc address 0x%08X param %d \n", &SomeITCMFunc, param);
}

// Function declaration for toggling the LED
__RAM_FUNC void ToggleLed()
{
	assert(/*(uint32_t)&ToggleLed >= ITCMRAM_START &&*/ (uint32_t)&ToggleLed < (ITCMRAM_END) && "Its not in ITCM area");
	Led.Toggle();
	//	Printf("Led %s\n", Led.IsOn() ? "On" : "Off");
}

///////////////////////////////////
hwFMC FMCController;

//#define DISPLAY_TEST
#define GRAPHIC_TEST
#ifdef DISPLAY_TEST
///////////////////////////////////
// for the Display
#include "TFTDisplay.h"
TFTDisplay Display;
///////////////////////////////////
#endif

#ifdef GRAPHIC_TEST
///////////////////////////////////
// for the SGGL
#include "glMain.h"
glMain Graphic;
///////////////////////////////////
#endif

int main()
{
	DumpMemInfo();
	// Print chip information
	DumpChipInfo();

	// Call a function stored in ITCM
	SomeITCMFunc(10);
    
	// Initialize SysTick timer
	hwSysTick::Init();

#ifdef DUALBOOT
	// Wait until the secondary core (CM4) boots and waits
	Printf("Waiting for CM4 to start\n");
	WaitForCM4();
#endif

	// Initialize hardware
	Printf("HW Init CM7\n");
	hwPWR::SupplyConfiguration(hwPWR::eSupplyConfigurations::C1); // Riverdi 7" uses this
	hwSysClock::Setup(); // Setup RCC clock
	hwSysClock::DumpClocks(); // Print clock information

	// Update system core clock
	hwSysClock::SystemCoreClockUpdate();

	// Reinitialize SysTick with new clock settings
	hwSysTick::Init();
	// Initialize FMC
	FMCController.Init();

#ifdef DISPLAY_TEST
	// Initialize the display with VM buffer
	Display.Init();
	Display.Intencity(400); // Set display intensity
#endif
	
#ifdef GRAPHIC_TEST
	Graphic.Init();
#endif
	
	Printf("HW Init done CM7\n");
	// Test SDRAM
	//FMCController.UnitTest();
#ifdef DISPLAY_TEST
	// Test the display
	Display.UnitTest(3);
#endif
	
#ifdef GRAPHIC_TEST
	// Test Graphic libary
	Graphic.UnitTest();
#endif

#ifdef DUALBOOT
	// Start CM4 core
	//Printf("Releasing CM4\n");
	//ReleaseCM4();
	//Printf("CM4 Running\n");
#endif

	uint32_t var = 3;

	Printf("Running %lu\n", var);

//	static uint32_t DValue = 200;
	while (1)
	{
		// Toggle the LED
		ToggleLed();
		osDelay(1000); // Delay for x milliseconds
		var++;
		if (var > 599) var = 0; // Reset var if it exceeds 599
	}
}
;
	
