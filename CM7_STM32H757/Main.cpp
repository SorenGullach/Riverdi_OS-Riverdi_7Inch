
#include <Utils.h>
#include <MemoryDefs.h>
#include <SysTick.h>
#include <RCC.h>
#include <PWR.h>
#include <GPIO.h>

//#include <StartupSemafor.h>

#include <stdio.h>
#include <stdint.h>

// create IO pins
hwGPIO Led(GPIOJ, 10, hwGPIO::eMode::Output, hwGPIO::ePP::PushPull);

//#define DUALBOOT

__RAMFUNC void SomeITCMFunc(int param)
{
	assert((int)&SomeITCMFunc >= ITCMRAM_START && (int)&SomeITCMFunc <= (ITCMRAM_START + ITCMRAM_LENGTH) && "Its not in ITCM area");
	Printf("SomeITCMFunc address 0x%08X param %d \n", &SomeITCMFunc, param);
}

int main()
{
	SomeITCMFunc(10);
	
	hwSysTick::Init(1, 1000); // Start systick
	//hwSysTick::UnitTest();

//	hwSysClock::DumpClocks();

#ifdef DUALBOOT		
	// Wait until CM4 boots and waits
	Printf("Waiting for CM4 to start\n");
	WaitForCM4();
#endif
	// init HW
	Printf("HW Init CM7\n");
	hwPWR::SupplyConfiguration (hwPWR::eSupplyConfigurations::C1); // Riverdi 7" use this
	// setup RCC clock
	hwSysClock::Setup();
	hwSysClock::DumpClocks();

	hwSysClock::SystemCoreClockUpdate();
	
	hwSysTick::Init(1, 1000); // Start systick, with new clocks
	hwSysTick::UnitTest();
	
	Printf("HW Init done CM7\n");
	
#ifdef DUALBOOT			
	// start CM4
//	Printf("Releasing CM4\n");
//	ReleaseCM4();
//	Printf("CM4 Running\n");
#endif
//	HSEMTest Test;
	
	static uint32_t var = 3;
	
	Printf("Running %lu\n", var);
	while (1)
	{
		Led.Toggle();
		hwSysTick::Delay(1000);
		var++;
	}
}

