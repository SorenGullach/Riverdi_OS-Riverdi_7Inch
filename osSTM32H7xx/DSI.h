#pragma once

#include "stm32h7xx.h"
#include <GPIO.h>
#include <SysTick.h>

// https://www.st.com/resource/en/application_note/an4860-introduction-to-dsi-host-on-stm32-mcus-and-mpus-stmicroelectronics.pdf

class hwDSI
{
public:
	hwDSI()
	{
	}
	void Init();

	void UnitTest(bool on)
	{
		if (!on)
		{
			DSI->VMCR &= ~(DSI_VMCR_PGE | DSI_VMCR_PGO);
			DSI->WCR |= DSI_WCR_LTDCEN;
			DSI->WCR |= DSI_WCR_DSIEN;
			return;
		}
		
		DSI->WCR &= ~DSI_WCR_LTDCEN;
		if (DSI->VMCR & DSI_VMCR_PGO)
			DSI->VMCR &= ~DSI_VMCR_PGO;
		else
			DSI->VMCR |= DSI_VMCR_PGO;
		DSI->VMCR |= DSI_VMCR_PGE;
	}
private:
	void Regulator(bool on);
	void PLL(bool on);
	void HostWrapper();
	void HostTiming();
	void HostFlowControl();
	void HostLTDC();
	void VideoMode();
	void ShortWrite(uint32_t ChannelID, uint32_t Mode, uint32_t Param1, uint32_t Param2);
	void SendDCSCommands();

#include <TFT_Setup.h>

};