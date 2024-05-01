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

#include <DSI.h>
#include <RCC.h>

// Turn on/off the DSI regulator and wait for the regulator to be ready
void hwDSI::Regulator(bool on)
{
	if (on)
	{
		// Enable the DSI regulator
		SetReg(DSI->WRPCR, DSI_WRPCR_REGEN);
		// Wait for the regulator to be ready
		uint32_t ticks = osTicks_ms;
		while ((DSI->WISR & DSI_WISR_RRS_Msk) == 0 && osTicks_ms - ticks < 1000) __asm("");
		assert(osTicks_ms - ticks < 1000);
		return;
	}
	// Disable the DSI regulator
	ClearReg(DSI->WRPCR, DSI_WRPCR_REGEN);
}

// Configure the DSI PLL, turn it ON and wait for its lock
void hwDSI::PLL(bool on)
{
	if (on)
	{
		// Disable PLL
		ClearReg(DSI->WRPCR, DSI_WRPCR_PLLEN);

		// Configure PLL parameters (NDIV, IDF, ODF)
		ModifyReg(DSI->WRPCR, 
			DSI_WRPCR_PLL_NDIV_Msk | DSI_WRPCR_PLL_IDF_Msk | DSI_WRPCR_PLL_ODF_Msk,
			NDIV << DSI_WRPCR_PLL_NDIV_Pos |			/*!< PLL Loop Division Factor */
			(uint32_t)IDF << DSI_WRPCR_PLL_IDF_Pos |	/*!< PLL Input Division Factor */
			(uint32_t)ODF << DSI_WRPCR_PLL_ODF_Pos);	/*!< PLL Output Division Factor */

		// Enable PLL
		SetReg(DSI->WRPCR, DSI_WRPCR_PLLEN);

		osDelay(1);
		// Wait for PLL lock
		uint32_t ticks = osTicks_ms;
		while ((DSI->WISR & DSI_WISR_PLLLS_Msk) == 0 && osTicks_ms - ticks < 1000) __asm("");
		assert(osTicks_ms - ticks < 1000);
		
		// Set the TX escape clock division factor 
		ModifyReg(DSI->CCR, DSI_CCR_TXECKDIV_Msk, TXClkEscape << DSI_CCR_TXECKDIV_Pos);
		// Set the timeout clock division factor 
		ModifyReg(DSI->CCR, DSI_CCR_TOCKDIV_Msk, TimeoutCkdiv << DSI_CCR_TOCKDIV_Pos);

		// Select DSI PLL as clock source
		// note RCC_D1CCIPR_DSISEL == RCC_D1CCIPR_DSISRC
		RCC->D1CCIPR &= ~RCC_D1CCIPR_DSISRC; // select DSI PLL
//		RCC->D1CCIPR |= RCC_D1CCIPR_DSISRC; // select PLL2Q
		
		// Calculate Byte Lane Clock based on clock source
		uint32_t ByteLaneClk;
		if ((RCC->D1CCIPR & RCC_D1CCIPR_DSISRC_Msk) == 0)
		{
			// Calculate Byte Lane Clock when DSI PLL is selected
			uint32_t INFIN = HSE_VALUE / (uint32_t)IDF;
			uint32_t Fvco = INFIN * (2 * NDIV); // FVCO is in the 1 to 2 GHz range
			assert(Fvco >= 1E9 && Fvco <= 2E9 && "Wrong FVCO");
			uint32_t FBCLK = Fvco / (2 * NDIV); // feedback clk
			assert(INFIN == FBCLK && "Wrong calc");
			uint32_t odf = 1UL << (0x3U & (uint32_t)ODF);
			//uint32_t PHI = Fvco / (2 * odf); // PHI is in the 62.5 MHz to 1 GHz range
			uint32_t PHI = (HSE_VALUE * NDIV) / ((uint32_t)IDF * odf); // PHI is in the 62.5 MHz to 1 GHz range
			assert(PHI >= 62.5E6 && PHI <= 1E9 && "Wrong PHI");
			ByteLaneClk = PHI / 8;
			
			unitIntervalx4 = (uint32_t)(4000000U / (PHI / 1000U));
			
			Printf("DSI Pll\n");
			Printf("  CLKIN==HSE %d\n", HSE_VALUE);
			Printf("  INFIN %d\n", INFIN);
			Printf("  Fvco %d\n", Fvco);
			Printf("  FBCLK %d\n", FBCLK);
			Printf("  PHI %d\n", PHI);
			Printf("  unitIntervalx4 %d\n", unitIntervalx4);
		}
		else
		{
			// Calculate Byte Lane Clock when PLL2Q is selected
			ByteLaneClk = hwSysClock::CC.PLL2.Q_clk;

			Printf("DSI PLL2Q\n");
		}
		// Set Byte Lane Clock and related parameters
		laneByteClk_KHz = ByteLaneClk/1000; 
		uint32_t Escape_div = (DSI->CCR & DSI_CCR_TXECKDIV_Msk) >> DSI_CCR_TXECKDIV_Pos;
		uint32_t Escape_ck = ByteLaneClk / Escape_div;
		Printf("  ByteLaneClk==PHI/8 %d laneByteClk %d kHz\n", ByteLaneClk, laneByteClk_KHz);
		Printf("  rxclkesc_ck %d\n", Escape_ck);
		return;		
	}
	// Disable PLL
	ClearReg(DSI->WRPCR, DSI_WRPCR_PLLEN);
}

// Configure the D-PHY parameters in the DSI Host and the DSI Wrapper
void hwDSI::HostWrapper()
{
	// Configuring the D - PHY parameters in the DSI Wrapper
	// Set the bit period in high-speed mode DSI clock
	/*
	* Calculate the bit period in high-speed mode in unit of 0.25 ns (UIX4) 
	* The equation is : UIX4 = IntegerPart( (1000/F_PHY_Mhz) * 4 )          
	* Where : F_PHY_Mhz = (NDIV * HSE_Mhz) / (IDF * ODF)                    
	**/
	// DSI Wrapper PHY Configuration Register
//	uint32_t unitIntervalx4 = 4; //(4000000U * (uint32_t)IDF * ((1UL << (0x3U & (uint32_t)ODF)))) / ((HSE_VALUE / 1000U) * NDIV);
	ModifyReg(DSI->WPCR[0U], DSI_WPCR0_UIX4_Msk, unitIntervalx4 << DSI_WPCR0_UIX4_Pos);
	
	//	uint32_t Frequency = 10000;
		// Low-Power RX low-pass Filtering Tuning 
//	DSI->WPCR[1U] &= ~DSI_WPCR1_LPRXFT_Msk;
//	DSI->WPCR[1U] |= 10000 << DSI_WPCR1_LPRXFT_Pos;
	
	// Select video mode by resetting CMDM and DSIM bits 
//	DSI->MCR
	ClearReg(DSI->WCFGR, DSI_WCFGR_DSIM);

	// Select the color coding for the wrapper 
	ModifyReg(DSI->WCFGR, DSI_WCFGR_COLMUX_Msk, (uint32_t)ColorCoding << DSI_WCFGR_COLMUX_Pos);

}

// Configure the DSI Host flow control and DBI interface
void hwDSI::HostFlowControl()
{
	// DSI Host Low - Power Mode Configuration Register
	ModifyReg(DSI->LPMCR,
		DSI_LPMCR_LPSIZE_Msk | DSI_LPMCR_VLPSIZE_Msk, 
		// Low power largest packet size 
		LPLargestPacketSize << DSI_LPMCR_LPSIZE_Pos | 
		// Low power VACT largest packet size 
		LPVACTLargestPacketSize << DSI_LPMCR_VLPSIZE_Pos);

	// DSI Host Protocol Configuration Register
	// Set the DSI Host Protocol Configuration Register 
	ModifyReg(DSI->PCR,	
		DSI_PCR_CRCRXE_Msk | DSI_PCR_ECCRXE_Msk | DSI_PCR_BTAE_Msk | DSI_PCR_ETRXE_Msk | DSI_PCR_ETTXE_Msk,		
		DSI_PCR_BTAE);
	
	// DSI Host Generic VCID Register
	// Virtual channel ID, These bits configure the virtual channel ID for the LTDC interface traffic
	ModifyReg(DSI->GVCIDR, DSI_GVCIDR_VCID_Msk, VirtualChannelID << DSI_GVCIDR_VCID_Pos);

	// DSI Host Mode Configuration Register
	// Select video mode by resetting CMDM and DSIM bits 
	ClearReg(DSI->MCR, DSI_MCR_CMDM);

	// DSI Host Clock Lane Timer Configuration Register
	// Data lane timer configuration 
	uint32_t maxTime = (ClockLaneLP2HSTime > ClockLaneHS2LPTime) ? ClockLaneLP2HSTime :
		  ClockLaneHS2LPTime;
	ModifyReg(DSI->CLTCR, DSI_CLTCR_LP2HS_TIME_Msk | DSI_CLTCR_HS2LP_TIME_Msk, maxTime << DSI_CLTCR_LP2HS_TIME_Pos | maxTime << DSI_CLTCR_HS2LP_TIME_Pos);
	
	// Configuring the D-PHY parameters in the DSI Host
	// DSI Host PHY Configuration Register
	ModifyReg(DSI->PCONFR,
		DSI_PCONFR_NL_Msk | DSI_PCONFR_SW_TIME_Msk, 
		// Configure the number of active data lanes 
		(NumberOfLanes == 1 ? 0B00U : 0B01U) << DSI_PCONFR_NL_Pos | 
		// Configure the wait period to request HS transmission after a stop state 
		StopWaitTime << DSI_PCONFR_SW_TIME_Pos);
	
	// DSI Host Clock Lane Configuration Register
	ModifyReg(DSI->CLCR,
		DSI_CLCR_DPCC_Msk | DSI_CLCR_ACR_Msk, 
		(ClockLaneRunsInHighSpeedMode ? DSI_CLCR_DPCC : 0U) | 
		(AutomaticClockLaneControl ? DSI_CLCR_ACR : 0U));

	// DSI Host Data Lane Timer Configuration Register
	ModifyReg(DSI->DLTCR,
		(DSI_DLTCR_MRD_TIME_Msk | DSI_DLTCR_LP2HS_TIME_Msk | DSI_DLTCR_HS2LP_TIME_Msk),
		DataLaneMaxReadTime << DSI_DLTCR_MRD_TIME_Pos | 
		DataLaneLP2HSTime << DSI_DLTCR_LP2HS_TIME_Pos | 
		DataLaneHS2LPTime << DSI_DLTCR_HS2LP_TIME_Pos);
}

// 10. Configure the DSI Host LTDC interface
void hwDSI::HostLTDC()
{
	// DSI Host LTDC VCID Register
	ModifyReg(DSI->LVCIDR, DSI_LVCIDR_VCID_Msk, VirtualChannelID << DSI_LVCIDR_VCID_Pos);

	// DSI Host LTDC Color Coding Register
	// Select the color coding for the host 
	ModifyReg(DSI->LCOLCR, DSI_LCOLCR_COLC_Msk, (uint32_t)ColorCoding << DSI_LCOLCR_COLC_Pos);

	// Enable/disable the loosely packed variant to 18-bit configuration 
	if (ColorCoding == eColorCoding::RGB666)
	{
		ModifyReg(DSI->LCOLCR, DSI_LCOLCR_LPE_Msk, LooselyPacked ? DSI_LCOLCR_LPE : 0);
	}

	// Configure the polarity of control signals 
	ModifyReg(DSI->LPCR,
		DSI_LPCR_DEP_Msk | DSI_LPCR_VSP_Msk | DSI_LPCR_HSP_Msk, 
		(DEPolarityHigh ? 0 : DSI_LPCR_DEP) | (VSPolarityHigh ? 0 : DSI_LPCR_VSP) | (HSPolarityHigh ? 0 : DSI_LPCR_HSP));
}

// 11. Configure the DSI Host for video mode 
void hwDSI::VideoMode()
{
	DSI->VMCR =
	// Configure the command transmission mode,
		(LPCommandEnable ? DSI_VMCR_LPCE : 0) |
	// Enable LP transition in HFP period 
		(LPHorizontalFrontPorchEnable ? DSI_VMCR_LPHFPE : 0) |
	// Enable LP transition in HBP period 
		(LPHorizontalBackPorchEnable ? DSI_VMCR_LPHBPE : 0) |
	// Enable LP transition in VACT period 
		(LPVerticalActiveEnable ? DSI_VMCR_LPVAE : 0) |
	// Enable LP transition in VFP period 
		(LPVerticalFrontPorchEnable ? DSI_VMCR_LPVFPE : 0) |
	// Enable LP transition in VBP period 
		(LPVerticalBackPorchEnable ? DSI_VMCR_LPVBPE : 0) |
	// Enable LP transition in vertical sync period 
		(LPVerticalSyncActiveEnable ? DSI_VMCR_LPVSAE : 0) |
	// Enable the request for an acknowledge response at the end of a frame 
		(FrameBTAAcknowledgeEnable ? DSI_VMCR_FBTAAE : 0) |
	// Configure the video mode transmission type 
		((uint32_t)Mode << DSI_VMCR_VMT_Pos);
	
	// Configure the video packet size 
	DSI->VPCR = PacketSize;
	// Set the chunks number to be transmitted through the DSI link 
	DSI->VCCR = NumberOfChunks;
	// Set the size of the null packet 
	DSI->VNPCR = NullPacketSize;
	
	// Define the video horizontal timing ----------------------------------------------
	uint32_t TotalW = HSW + HBP + HACT + HFP, TotalH = VSH + VBP + VACT + VFP;
	uint8_t Fps = 60;
	uint32_t LcdClock_kHz = (TotalH*TotalW * Fps) / 1000;
	// Set the Horizontal Synchronization Active (HSW) in lane byte clock cycles 
	Printf("    Video horizontal timing: \n");
	Printf("       Calculated Pixel clock %d Hz\n", TotalH*TotalW * Fps);
	Printf("       laneByteClk %d kHz LcdClock %d kHz Ratio %d\n", laneByteClk_KHz, LcdClock_kHz, (laneByteClk_KHz*1000) / LcdClock_kHz);
	uint32_t HorizontalSyncActive = (HSW * laneByteClk_KHz) / LcdClock_kHz;
	DSI->VHSACR = HorizontalSyncActive;
	// Set the Horizontal Back Porch (HBP) in lane byte clock cycles 
	uint32_t HorizontalBackPorch = (HBP * laneByteClk_KHz) / LcdClock_kHz;
	DSI->VHBPCR = HorizontalBackPorch;
	// Set the total line time (HLINE=HSW+HBP+HACT+HFP) in lane byte clock cycles 
	uint32_t HorizontalLine = (TotalW * laneByteClk_KHz) / LcdClock_kHz;
	DSI->VLCR = HorizontalLine;
	Printf("       HorizontalSyncActive %d HorizontalBackPorch %d HorizontalLine %d\n", HorizontalSyncActive, HorizontalBackPorch, HorizontalLine);
	
	// Define the vertical line configuration ------------------------------------------
	// Set the Vertical Synchronization Active (VSH) 
	DSI->VVSACR = VSH;
	// Set the Vertical Back Porch (VBP)
	DSI->VVBPCR = VBP;
	// Set the Vertical Front Porch (VFP)
	DSI->VVFPCR = VFP;
	// Set the Vertical Active period
	DSI->VVACR = VACT;
}

// Send short DCS commands
void hwDSI::ShortWrite(
	uint32_t ChannelID,
	uint32_t Type,
	uint32_t Param1,
	uint32_t Param2)
{
	// Get tick 
	uint32_t tickstart = osTicks_ms;

	//	Printf("DSI cmd 0x%02X 0x%02X 0x%02X\n", Type, Param1, Param2);
		// Wait for Command FIFO not full
	while ((DSI->GPSR & DSI_GPSR_CMDFE) == 0/* && (DSI->GPSR & DSI_GPSR_PWRFE) == 0*/)
	{
		//	osDelay(100);
		//	Printf("Waiting\n");
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
	/*
		// Wait for Command FIFO not full
		//	while ((DSI->GPSR & DSI_GPSR_CMDFE) == 0 && (DSI->GPSR & DSI_GPSR_CMDFF) != 0)
			{
				if ((DSI->GPSR & DSI_GPSR_CMDFE) != 0)
					Printf("Command FIFO Empty\n");
			if ((DSI->GPSR & DSI_GPSR_CMDFF) != 0)
				Printf("Command FIFO Full\n");
		if ((DSI->GPSR & DSI_GPSR_PWRFE) != 0)
			Printf("Payload Write FIFO Empty\n");
		if ((DSI->GPSR & DSI_GPSR_PWRFF) != 0)
			Printf("Payload Write FIFO Full\n");
		if ((DSI->GPSR & DSI_GPSR_PRDFE) != 0)
			Printf("Payload Read FIFO Empty\n");
		if ((DSI->GPSR & DSI_GPSR_PRDFF) != 0)
			Printf("Payload Read FIFO Full\n");
		if ((DSI->GPSR & DSI_GPSR_RCB) != 0)
			Printf("Read Command Busy\n");
	}
	*/
	//Printf("OK DSI 0x%02X 0x%02X 0x%02X\n", Type, Param1, Param2);
}
void hwDSI::SendDCSCommands()
{
	DSI->CMCR = 0;
#define DSI_DCS_SHORT_PKT_WRITE_P0  0x00000005U //!< DCS short write, no parameters      
#define DSI_DCS_SHORT_PKT_WRITE_P1  0x00000015U //!< DCS short write, one parameter      
#define DSI_GEN_SHORT_PKT_WRITE_P0  0x00000003U //!< Generic short write, no parameters  
#define DSI_GEN_SHORT_PKT_WRITE_P1  0x00000013U //!< Generic short write, one parameter  
#define DSI_GEN_SHORT_PKT_WRITE_P2  0x00000023U //!< Generic short write, two parameters 

#define DSI_EXIT_SLEEP_MODE       0x11U
#define DSI_SOFT_RESET            0x01U
#define DSI_SET_DISPLAY_OFF       0x28U
#define DSI_SET_DISPLAY_ON        0x29U
	
	ShortWrite(0, DSI_DCS_SHORT_PKT_WRITE_P0, DSI_SOFT_RESET, 0x0);
	osDelay(120);

	//	ShortWrite(0, DSI_GEN_SHORT_PKT_WRITE_P2, 0x87, 0x5A); // ??
	//	ShortWrite(0, DSI_GEN_SHORT_PKT_WRITE_P2, 0xB0, 0x80); // enable VCOM buffer
	ShortWrite(0, DSI_GEN_SHORT_PKT_WRITE_P2, 0xB2, 0x50); // Normally black. &  Enable 2 lane MIPI interface
	/*
	ShortWrite(0, DSI_GEN_SHORT_PKT_WRITE_P2, 0x80, 0x4B); // Gamma voltage setting.
	ShortWrite(0, DSI_GEN_SHORT_PKT_WRITE_P2, 0x81, 0xFF); // Gamma voltage setting.
	ShortWrite(0, DSI_GEN_SHORT_PKT_WRITE_P2, 0x82, 0x1A); // Gamma voltage setting.
	ShortWrite(0, DSI_GEN_SHORT_PKT_WRITE_P2, 0x83, 0x88); // Gamma voltage setting.
	ShortWrite(0, DSI_GEN_SHORT_PKT_WRITE_P2, 0x84, 0x8F); // Gamma voltage setting.
	ShortWrite(0, DSI_GEN_SHORT_PKT_WRITE_P2, 0x85, 0x35); // Gamma voltage setting.
	ShortWrite(0, DSI_GEN_SHORT_PKT_WRITE_P2, 0x86, 0xB0); // Gamma voltage setting.
	osDelay(50);
*/
	ShortWrite(0, DSI_DCS_SHORT_PKT_WRITE_P0, DSI_EXIT_SLEEP_MODE, 0x0);
	osDelay(120);

	ShortWrite(0, DSI_DCS_SHORT_PKT_WRITE_P0, DSI_SET_DISPLAY_ON, 0x0);
	osDelay(120); 

}

// Initialize DSI hardware
void hwDSI::Init()
{
	// 1 Enable the DSI clock
	RCC->APB3ENR |= RCC_APB3ENR_DSIEN;
	
	Printf("DSI version 0x%X\n", DSI->VR);
	
	// Turn on the DSI regulator and wait for the regulator to be ready
	Regulator(true);

	// Configure the DSI PLL, turn it ON and wait for its lock
	// Configure the DSI Host timing 
	PLL(true);
	
	// Configure the DSI Host flow control and DBI interface
	HostFlowControl();

	//  Configure the DSI Host LTDC interface
	HostLTDC();

	// Configure the D-PHY parameters in the DSI Host and the DSI Wrapper
	HostWrapper();

	// Configure the DSI Host for video mode 
	VideoMode();
	// Configure the DSI Host for adapted command mode
	//AdaptedCommandMode();
	
	// Enable the D-PHY setting the DEN bit of the DSI_PCTLR
	// Enable the D-PHY clock lane setting the CKEN bit of the DSI_PCTLR
	DSI->PCTLR |= (DSI_PCTLR_CKE | DSI_PCTLR_DEN); // D-PHY clock and digital enable

	// Enable the DSI Host setting the EN bit of the DSI_CR
	DSI->CR |= DSI_CR_EN;
	
	// Optionally, send DCS commands through the APB generic interface to configure the display
	SendDCSCommands();
	
	// Enable the DSI Wrapper setting the DSIEN bit of the DSI_WCR
	DSI->WCR |= DSI_WCR_DSIEN;
	
	// Enable the LTDC in the LTDC
	// Start the LTDC flow through the DSI Wrapper (DSI_WCR.LTDCEN = 1)
		// a) In video mode, the data streaming starts as soon as the LTDC is enabled
		// b) In adapted command mode, the frame buffer update is launched as soon as bit DSI_WCR.LTDCEN is set
	
}
