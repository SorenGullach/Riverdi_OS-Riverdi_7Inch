

#include <DSI.h>
#include <RCC.h>

// Turn on the DSI regulator and wait for the regulator to be ready
void hwDSI::Regulator(bool on)
{
	if (on)
	{
		// Enable
		SetReg(DSI->WRPCR, DSI_WRPCR_REGEN);
		uint32_t ticks = osTicks_ms;
		while ((DSI->WISR & DSI_WISR_RRS_Msk) == 0 && osTicks_ms - ticks < 1000) __asm("");
		assert(osTicks_ms - ticks < 1000);
		return;
	}
	// Disable
	ClearReg(DSI->WRPCR, DSI_WRPCR_REGEN);
}

// Configure the DSI PLL, turn it ON and wait for its lock
void hwDSI::PLL(bool on)
{
	if (on)
	{
		// Set NDIV & IDF & ODF
		ModifyReg(DSI->WRPCR, 
			DSI_WRPCR_PLL_NDIV_Msk | DSI_WRPCR_PLL_IDF_Msk | DSI_WRPCR_PLL_ODF_Msk,
			NDIV << DSI_WRPCR_PLL_NDIV_Pos | IDF << DSI_WRPCR_PLL_IDF_Pos | ODF << DSI_WRPCR_PLL_ODF_Pos);

		// Enable
		SetReg(DSI->WRPCR, DSI_WRPCR_PLLEN);

		osDelay(1);
		uint32_t ticks = osTicks_ms;
		while ((DSI->WISR & DSI_WISR_PLLLS_Msk) == 0 && osTicks_ms - ticks < 1000) __asm("");
		assert(osTicks_ms - ticks < 1000);
		
		uint32_t Fvco = (HSE_VALUE / IDF) * 2 * NDIV; // FVCO is in the 1 to 2 GHz range
		uint32_t PHI = Fvco / (2 * ODF); // PHI is in the 62.5 MHz to 1 GHz range
		uint32_t F_PHY_Mhz = (NDIV * HSE_VALUE) / (IDF * ODF);                  
		uint32_t UIX4 = ((1000 / F_PHY_Mhz) * 4);
		Printf("Fvco %d\n", Fvco);
		Printf("PHI %d\n", PHI);
		Printf("F_PHY_Mhz %d\n", F_PHY_Mhz);
		Printf("UIX4 %d\n", UIX4);
		return;		
	}
	// Disable
	ClearReg(DSI->WRPCR, DSI_WRPCR_PLLEN);
}

// Configure the D - PHY parameters in the DSI Host and the DSI Wrapper
void hwDSI::HostWrapper()
{
	// Configuring the D - PHY parameters in the DSI Wrapper
	// Set the bit period in high-speed mode 
	// DSI clock
	/*
	* Calculate the bit period in high-speed mode in unit of 0.25 ns (UIX4) 
	* The equation is : UIX4 = IntegerPart( (1000/F_PHY_Mhz) * 4 )          
	* Where : F_PHY_Mhz = (NDIV * HSE_Mhz) / (IDF * ODF)                    
	**/
	uint32_t unitIntervalx4 = (4000000U * IDF * ((1UL << (0x3U & ODF)))) / ((HSE_VALUE / 1000U) * NDIV);
	ModifyReg(DSI->WPCR[0U], DSI_WPCR0_UIX4_Msk, unitIntervalx4 << DSI_WPCR0_UIX4_Pos);

	// Configuring the D-PHY parameters in the DSI Host

	// Configure the number of active data lanes 
	ModifyReg(DSI->PCONFR, DSI_PCONFR_NL_Msk, (NumberOfLanes == 1 ? 0U : 1U) << DSI_PCONFR_NL_Pos);
	
	// Clock lane configuration 
	ModifyReg(DSI->CLCR, DSI_CLCR_DPCC_Msk | DSI_CLCR_ACR_Msk, DSI_CLCR_DPCC | (AutomaticClockLaneControl ? DSI_CLCR_ACR : 0));
	
	// Data lane timer configuration 
	uint32_t maxTime = (ClockLaneLP2HSTime > ClockLaneHS2LPTime) ? ClockLaneLP2HSTime :
		  ClockLaneHS2LPTime;
	ModifyReg(DSI->CLTCR, DSI_CLTCR_LP2HS_TIME_Msk | DSI_CLTCR_HS2LP_TIME_Msk, maxTime << DSI_CLTCR_LP2HS_TIME_Pos | maxTime << DSI_CLTCR_HS2LP_TIME_Pos);
	
	ModifyReg(DSI->DLTCR,
		(DSI_DLTCR_MRD_TIME_Msk | DSI_DLTCR_LP2HS_TIME_Msk | DSI_DLTCR_HS2LP_TIME_Msk),
		DataLaneMaxReadTime << DSI_DLTCR_MRD_TIME_Pos | DataLaneLP2HSTime << DSI_DLTCR_LP2HS_TIME_Pos | DataLaneHS2LPTime << DSI_DLTCR_HS2LP_TIME_Pos);

	// Configure the wait period to request HS transmission after a stop state 
	ModifyReg(DSI->PCONFR, DSI_PCONFR_SW_TIME_Msk, StopWaitTime << DSI_PCONFR_SW_TIME_Pos);
	
	//	uint32_t Frequency = 10000;
		// Low-Power RX low-pass Filtering Tuning 
		//	DSI->WPCR[1U] &= ~DSI_WPCR1_LPRXFT;
		//	DSI->WPCR[1U] |= Frequency << 25U;
}

// Configure the DSI Host timing 
void hwDSI::HostTiming()
{
	// Set the TX escape clock division factor 
	ModifyReg(DSI->CCR, DSI_CCR_TXECKDIV_Msk, TXEscapeCkdiv << DSI_CCR_TXECKDIV_Pos);

	// Set the timeout clock division factor 
	ModifyReg(DSI->CCR, DSI_CCR_TOCKDIV_Msk, TimeoutCkdiv << DSI_CCR_TOCKDIV_Pos);
}

// Configure the DSI Host flow control and DBI interface
void hwDSI::HostFlowControl()
{
	// Set the DSI Host Protocol Configuration Register 
	ModifyReg(DSI->PCR,	
		DSI_PCR_CRCRXE_Msk | DSI_PCR_ECCRXE_Msk | DSI_PCR_BTAE_Msk | DSI_PCR_ETRXE_Msk | DSI_PCR_ETTXE_Msk,		
		DSI_PCR_BTAE);
	
	ModifyReg(DSI->GVCIDR, DSI_GVCIDR_VCID_Msk, VirtualChannelID << DSI_GVCIDR_VCID_Pos);
	
	//ConfigCommand
	//DSI->CMCR
	
	// Configure the tearing effect acknowledge request 
	//	DSI->CMCR &= ~DSI_CMCR_TEARE_Msk;
	//	DSI->CMCR |= TEAcknowledgeRequest;

}

// 10. Configure the DSI Host LTDC interface
void hwDSI::HostLTDC()
{
	ModifyReg(DSI->LCVCIDR, DSI_LVCIDR_VCID_Msk, VirtualChannelID << DSI_LVCIDR_VCID_Pos);

	// Select the color coding for the host 
	ModifyReg(DSI->LCOLCR, DSI_LCOLCR_COLC_Msk, (uint32_t)ColorCoding << DSI_LCOLCR_COLC_Pos);

	// Enable/disable the loosely packed variant to 18-bit configuration 
	if (ColorCoding == eRGBMode::RGB666)
	{
		ModifyReg(DSI->LCOLCR, DSI_LCOLCR_LPE_Msk, LooselyPacked ? DSI_LCOLCR_LPE : 0);
	}

	// Select the color coding for the wrapper 
	ModifyReg(DSI->WCFGR, DSI_WCFGR_COLMUX_Msk, (uint32_t)ColorCoding << DSI_WCFGR_COLMUX_Pos);

	// Configure the polarity of control signals 
	ModifyReg(DSI->LPCR,
		DSI_LPCR_DEP_Msk | DSI_LPCR_VSP_Msk | DSI_LPCR_HSP_Msk, 
		(DEPolarityHigh ? 0 : DSI_LPCR_DEP) | (VSPolarityHigh ? 0 : DSI_LPCR_VSP) | (HSPolarityHigh ? 0 : DSI_LPCR_HSP));
}

// 11. Configure the DSI Host for video mode 
void hwDSI::VideoMode()
{
	// Select video mode by resetting CMDM and DSIM bits 
	ClearReg(DSI->MCR, DSI_MCR_CMDM);
	ClearReg(DSI->WCFGR, DSI_WCFGR_DSIM);

	// Configure the command transmission mode 
	ModifyReg(DSI->VMCR, DSI_VMCR_LPCE_Msk, LPCommandEnable ? DSI_VMCR_LPCE : 0);

	// Low power largest packet size 
	ModifyReg(DSI->LPMCR, DSI_LPMCR_LPSIZE_Msk, LPLargestPacketSize << DSI_LPMCR_LPSIZE_Pos);

	// Low power VACT largest packet size 
	ModifyReg(DSI->LPMCR, DSI_LPMCR_VLPSIZE_Msk, LPVACTLargestPacketSize << DSI_LPMCR_VLPSIZE_Pos);

	// Enable LP transition in HFP period 
	ModifyReg(DSI->VMCR, DSI_VMCR_LPHFPE_Msk, LPHorizontalFrontPorchEnable ? DSI_VMCR_LPHFPE : 0);

	// Enable LP transition in HBP period 
	ModifyReg(DSI->VMCR, DSI_VMCR_LPHBPE_Msk, LPHorizontalBackPorchEnable ? DSI_VMCR_LPHBPE : 0);

	// Enable LP transition in VACT period 
	ModifyReg(DSI->VMCR, DSI_VMCR_LPVAE_Msk, LPVerticalActiveEnable ? DSI_VMCR_LPVAE : 0);

	// Enable LP transition in VFP period 
	ModifyReg(DSI->VMCR, DSI_VMCR_LPVFPE_Msk, LPVerticalFrontPorchEnable ? DSI_VMCR_LPVFPE : 0);

	// Enable LP transition in VBP period 
	ModifyReg(DSI->VMCR, DSI_VMCR_LPVBPE_Msk, LPVerticalBackPorchEnable ? DSI_VMCR_LPVBPE : 0);

	// Enable LP transition in vertical sync period 
	ModifyReg(DSI->VMCR, DSI_VMCR_LPVSAE_Msk, LPVerticalSyncActiveEnable ? DSI_VMCR_LPVSAE : 0);

	// Enable the request for an acknowledge response at the end of a frame 
	ModifyReg(DSI->VMCR, DSI_VMCR_FBTAAE_Msk, FrameBTAAcknowledgeEnable ? DSI_VMCR_FBTAAE : 0);

	// Configure the video mode transmission type 
	ModifyReg(DSI->VMCR, DSI_VMCR_VMT_Msk, (uint32_t)Mode << DSI_VMCR_VMT_Pos);

	// Configure the video packet size 
	ModifyReg(DSI->VPCR, DSI_VPCR_VPSIZE_Msk, PacketSize << DSI_VPCR_VPSIZE_Pos);

	// Set the chunks number to be transmitted through the DSI link 
	ModifyReg(DSI->VCCR, DSI_VCCR_NUMC_Msk, NumberOfChunks << DSI_VCCR_NUMC_Pos);

	// Set the size of the null packet 
	ModifyReg(DSI->VNPCR, DSI_VNPCR_NPSIZE_Msk, NullPacketSize << DSI_VNPCR_NPSIZE_Pos);

	// Define the video horizontal timing ----------------------------------------------
	// Set the total line time (HLINE=HSA+HBP+HACT+HFP) in lane byte clock cycles 
	assert((HACT + HSA + HBP + HFP) == 1414);
	ModifyReg(DSI->VLCR, DSI_VLCR_HLINE_Msk, HorizontalLine << DSI_VLCR_HLINE_Pos);
	
	// Set the Horizontal Synchronization Active (HSA) in lane byte clock cycles 
	ModifyReg(DSI->VHSACR, DSI_VHSACR_HSA_Msk, HorizontalSyncActive << DSI_VHSACR_HSA_Pos);
	
	// Set the Horizontal Back Porch (HBP) in lane byte clock cycles 
	ModifyReg(DSI->VHBPCR, DSI_VHBPCR_HBP_Msk, HorizontalBackPorch << DSI_VHBPCR_HBP_Pos);

	// Define the vertical line configuration ------------------------------------------
	// Set the Vertical Synchronization Active (VSA) 
	ModifyReg(DSI->VVSACR, DSI_VVSACR_VSA_Msk, VSA << DSI_VVSACR_VSA_Pos);

	// Set the Vertical Back Porch (VBP)
	ModifyReg(DSI->VVBPCR, DSI_VVBPCR_VBP_Msk, VBP << DSI_VVBPCR_VBP_Pos);

	// Set the Vertical Front Porch (VFP)
	ModifyReg(DSI->VVFPCR, DSI_VVFPCR_VFP_Msk, VFP << DSI_VVFPCR_VFP_Pos);

	// Set the Vertical Active period
	ModifyReg(DSI->VVACR, DSI_VVACR_VA_Msk, VACT << DSI_VVACR_VA_Pos);
}

void hwDSI::ShortWrite(
	uint32_t ChannelID,
	uint32_t Type,
	uint32_t Param1,
	uint32_t Param2)
{
	// Get tick 
	uint32_t tickstart = osTicks_ms;

	// Wait for Command FIFO not full
	while ((DSI->GPSR & DSI_GPSR_CMDFE) == 0 && (DSI->GPSR & DSI_GPSR_PWRFE) == 0)
	{
		// Check for the Timeout 
		if ((osTicks_ms - tickstart) > 1000)
		{
			Printf("Error DSI 0x%02X 0x%02X 0x%02X\n", Type, Param1, Param2);
//			return;
			break;
		}
	}

	// Configure the packet to send a short DCS command with 0 or 1 parameter 
	// Update the DSI packet header with new information 
	DSI->GHCR = (Type << DSI_GHCR_DT_Pos) | (ChannelID << DSI_GHCR_VCID0_Pos) | (Param1 << DSI_GHCR_WCLSB0_Pos) | (Param2 << DSI_GHCR_WCMSB_Pos);
	switch (Type & 0xF0)
	{
	case 0x00:
	case 0x10:
		Printf("OK DSI 0x%02X 0x%02X\n", Type, Param1);
		break;
	case 0x20:
		Printf("OK DSI 0x%02X 0x%02X 0x%02X\n", Type, Param1, Param2);
		break;
	}
}
void hwDSI::SendDCSCommands()
{
#define DSI_DCS_SHORT_PKT_WRITE_P0  0x00000005U //!< DCS short write, no parameters      
#define DSI_DCS_SHORT_PKT_WRITE_P1  0x00000015U //!< DCS short write, one parameter      
#define DSI_GEN_SHORT_PKT_WRITE_P0  0x00000003U //!< Generic short write, no parameters  
#define DSI_GEN_SHORT_PKT_WRITE_P1  0x00000013U //!< Generic short write, one parameter  
#define DSI_GEN_SHORT_PKT_WRITE_P2  0x00000023U //!< Generic short write, two parameters 

#define DSI_EXIT_SLEEP_MODE       0x11U
#define DSI_SOFT_RESET            0x01U
#define DSI_SET_DISPLAY_OFF       0x28U
#define DSI_SET_DISPLAY_ON        0x29U
	
	ShortWrite(0, DSI_DCS_SHORT_PKT_WRITE_P1, DSI_SOFT_RESET, 0x0);
	osDelay(120);

	ShortWrite(0, DSI_GEN_SHORT_PKT_WRITE_P2, 0x87, 0x5A); // ??
	ShortWrite(0, DSI_GEN_SHORT_PKT_WRITE_P2, 0xB0, 0x80); // enable VCOM buffer
	ShortWrite(0, DSI_GEN_SHORT_PKT_WRITE_P2, 0xB2, 0x50); // Normally black. &  Enable 2 lane MIPI interface
	ShortWrite(0, DSI_GEN_SHORT_PKT_WRITE_P2, 0x80, 0x4B); // Gamma voltage setting.
	ShortWrite(0, DSI_GEN_SHORT_PKT_WRITE_P2, 0x81, 0xFF); // Gamma voltage setting.
	ShortWrite(0, DSI_GEN_SHORT_PKT_WRITE_P2, 0x82, 0x1A); // Gamma voltage setting.
	ShortWrite(0, DSI_GEN_SHORT_PKT_WRITE_P2, 0x83, 0x88); // Gamma voltage setting.
	ShortWrite(0, DSI_GEN_SHORT_PKT_WRITE_P2, 0x84, 0x8F); // Gamma voltage setting.
	ShortWrite(0, DSI_GEN_SHORT_PKT_WRITE_P2, 0x85, 0x35); // Gamma voltage setting.
	ShortWrite(0, DSI_GEN_SHORT_PKT_WRITE_P2, 0x86, 0xB0); // Gamma voltage setting.
	osDelay(50);

	ShortWrite(0, DSI_DCS_SHORT_PKT_WRITE_P1, DSI_EXIT_SLEEP_MODE, 0x0);
	osDelay(120);

	ShortWrite(0, DSI_DCS_SHORT_PKT_WRITE_P1, DSI_SET_DISPLAY_ON, 0x0);
	osDelay(120); 

}

void hwDSI::Init()
{
	// 1 Enable the DSI clock
	RCC->APB3ENR |= RCC_APB3ENR_DSIEN;
	
	// 5 Turn on the DSI regulator and wait for the regulator to be ready
	Regulator(true);

	// 6 Configure the DSI PLL, turn it ON and wait for its lock
	PLL(true);
	
	// 7 Configure the D - PHY parameters in the DSI Host and the DSI Wrapper
	HostWrapper();

	// 8 Configure the DSI Host timing 
	HostTiming();

	// 9 Configure the DSI Host flow control and DBI interface
	HostFlowControl();

	// 10. Configure the DSI Host LTDC interface
	HostLTDC();
	
	// 11. Configure the DSI Host for video mode 
	VideoMode();
	// 11. Configure the DSI Host for adapted command mode
	//AdaptedCommandMode();
	
	// 12. Enable the D-PHY setting the DEN bit of the DSI_PCTLR
	// 13. Enable the D-PHY clock lane setting the CKEN bit of the DSI_PCTLR
	DSI->PCTLR |= (DSI_PCTLR_CKE | DSI_PCTLR_DEN); // D-PHY clock and digital enable

	// 14. Enable the DSI Host setting the EN bit of the DSI_CR
	DSI->CR |= DSI_CR_EN;
	// 15. Enable the DSI Wrapper setting the DSIEN bit of the DSI_WCR
	DSI->WCR |= DSI_WCR_DSIEN;
	// 16. Optionally, send DCS commands through the APB generic interface to configure the display
	SendDCSCommands();
	
	// 17. Enable the LTDC in the LTDC
	// 18. Start the LTDC flow through the DSI Wrapper (DSI_WCR.LTDCEN = 1)
		// a) In video mode, the data streaming starts as soon as the LTDC is enabled
		// b) In adapted command mode, the frame buffer update is launched as soon as bit DSI_WCR.LTDCEN is set
	
}


