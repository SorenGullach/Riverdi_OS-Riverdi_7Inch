
#include <LTDC.h>
#include "stm32h7xx.h"
#include <assert.h>
#include <Utils.h>

// 3. Configure the synchronous timings: VSYNC, HSYNC, vertical and horizontal back
	// porch, active data area and the front porch timings following the panel datasheet 
	// as described in the Section 33.4.1.
void hwLTDC::SynchronousTimings()
{
	// Set Synchronization size W,H
	assert(HSA == 70);
	assert(VSA == 10);
	ModifyReg(LTDC->SSCR,
		LTDC_SSCR_HSW_Msk | LTDC_SSCR_VSH_Msk,
		((HSA - 1) << LTDC_SSCR_HSW_Pos) | ((VSA - 1) << LTDC_SSCR_VSH_Pos));
	
	// Set Accumulated Back porch H,V 
	assert(HSA + HBP == 230);
	assert(VSA + VBP == 33);
	ModifyReg(LTDC->BPCR,
		LTDC_BPCR_AHBP_Msk | LTDC_BPCR_AVBP_Msk,
		((HSA + HBP - 1) << LTDC_BPCR_AHBP_Pos) | ((VSA + VBP - 1) << LTDC_BPCR_AVBP_Pos));

	// Set Accumulated Active Width W,H
	assert(HSA + HBP + HACT == 1254);
	assert(VSA + VBP + VACT == 633);
	ModifyReg(LTDC->AWCR,
		LTDC_AWCR_AAW_Msk | LTDC_AWCR_AAH_Msk,
		((HSA + HBP + HACT - 1) << LTDC_AWCR_AAW_Pos) | ((VSA + VBP + VACT - 1) << LTDC_AWCR_AAH_Pos));

	// Set Total Width W,H
	assert(HSA + HBP + HACT + HFP == 1414);
	assert(VSA + VBP + VACT + VFP == 645);
	ModifyReg(LTDC->TWCR,
		LTDC_TWCR_TOTALW_Msk | LTDC_TWCR_TOTALH_Msk,
		((HSA + HBP + HACT + HFP - 1) << LTDC_TWCR_TOTALW_Pos) | ((VSA + VBP + VACT + VFP - 1) << LTDC_TWCR_TOTALH_Pos));
}

// 4. Configure the synchronous signals and clock polarity in the LTDC_GCR register.
// Configure the HS, VS, DE and PC polarity 
void hwLTDC::ClockPolarity()
{
	LTDC->GCR &= ~(LTDC_GCR_HSPOL_Msk | LTDC_GCR_VSPOL_Msk | LTDC_GCR_DEPOL_Msk | LTDC_GCR_PCPOL_Msk);
	LTDC->GCR |= (uint32_t)(HSPolarityHigh ? LTDC_GCR_HSPOL : 0 | VSPolarityHigh ? LTDC_GCR_VSPOL : 0 | \
	                                   !DEPolarityHigh ? LTDC_GCR_DEPOL : 0 | PCPolarityInverted ? LTDC_GCR_PCPOL : 0);
}

// 5. If needed, configure the background color in the LTDC_BCCR register.
void hwLTDC::BackgroundColor(uint8_t red, uint8_t blue, uint8_t green)
{
	ModifyReg(LTDC->BCCR,
		LTDC_BCCR_BCBLUE_Msk | LTDC_BCCR_BCGREEN_Msk | LTDC_BCCR_BCRED_Msk, 
		(blue << LTDC_BCCR_BCBLUE_Pos) | (green << LTDC_BCCR_BCGREEN_Pos) | (red << LTDC_BCCR_BCRED_Pos));
}

// 7. Configure the layer1 / 2 parameters by :
// 8. Enable layer1 / 2 and if needed the CLUT in the LTDC_LxCR register.
void hwLTDC::Layer(R_t<uint8_t, 1, 2> layer, void *pRGBData)
{
	LTDC_Layer_TypeDef *pLTDCLayer = LTDC_Layer1;
	if (layer == 2)
		pLTDCLayer = LTDC_Layer2;

	if (pRGBData == NULL) // disable
	{
		ModifyReg(pLTDCLayer->BFCR,
			LTDC_LxBFCR_BF1_Msk | LTDC_LxBFCR_BF2_Msk,
			((uint32_t)0 << LTDC_LxBFCR_BF1_Pos) | ((uint32_t)0 << LTDC_LxBFCR_BF2_Pos));
	
		ModifyReg(pLTDCLayer->CR, LTDC_LxCR_LEN_Msk, 0);
		return;
	}
	
	// – programming the layer window horizontal and vertical position in the
	// LTDC_LxWHPCR and LTDC_WVPCR registers.The layer window must be in the active data area.
	uint32_t XOffset = (LTDC->BPCR & LTDC_BPCR_AHBP_Msk) >> LTDC_BPCR_AHBP_Pos;
	ModifyReg(pLTDCLayer->WHPCR,
		LTDC_LxWHPCR_WHSTPOS_Msk | LTDC_LxWHPCR_WHSPPOS_Msk, 
		((Layers[layer].WindowX0 + XOffset + 1U) << LTDC_LxWHPCR_WHSTPOS_Pos) | ((Layers[layer].WindowX1 + XOffset) << LTDC_LxWHPCR_WHSPPOS_Pos));

	uint32_t YOffset = (LTDC->BPCR & LTDC_BPCR_AVBP_Msk) >> LTDC_BPCR_AVBP_Pos;
	ModifyReg(pLTDCLayer->WVPCR,
		LTDC_LxWVPCR_WVSTPOS_Msk | LTDC_LxWVPCR_WVSPPOS_Msk, 
		((Layers[layer].WindowY0 + YOffset + 1U) << LTDC_LxWVPCR_WVSTPOS_Pos) | ((Layers[layer].WindowY1 + YOffset) << LTDC_LxWVPCR_WVSPPOS_Pos));

	// – programming the pixel input format in the LTDC_LxPFCR register
	ModifyReg(pLTDCLayer->PFCR, LTDC_LxPFCR_PF_Msk, (uint32_t)Layers[layer].PixelFormat << LTDC_LxPFCR_PF_Pos);

	// – programming the color frame buffer start address in the LTDC_LxCFBAR register
	ModifyReg(pLTDCLayer->CFBAR, LTDC_LxCFBAR_CFBADD, ((uint32_t)pRGBData << LTDC_LxCFBAR_CFBADD_Pos));

	// – programming the line length and pitch of the color frame buffer in the LTDC_LxCFBLR register
	uint32_t BytesPerPixel;
	switch (Layers[layer].PixelFormat)
	{
	case hwLTDC::ePixelFormat::ARGB8888:
		BytesPerPixel = 4;
		break;
	case hwLTDC::ePixelFormat::RGB888:
		BytesPerPixel = 3;
		break;
	case hwLTDC::ePixelFormat::ARGB4444:
	case hwLTDC::ePixelFormat::RGB565:
	case hwLTDC::ePixelFormat::ARGB1555:
	case hwLTDC::ePixelFormat::AL88:
		BytesPerPixel = 2;
		break;
	default:
		BytesPerPixel = 1;
		break;
	}
	ModifyReg(pLTDCLayer->CFBLR,
		LTDC_LxCFBLR_CFBLL_Msk | LTDC_LxCFBLR_CFBP_Msk,
		(Layers[layer].ImageWidth * BytesPerPixel) << LTDC_LxCFBLR_CFBP_Pos | 
		((Layers[layer].WindowX1 - Layers[layer].WindowX0)*BytesPerPixel) << LTDC_LxCFBLR_CFBLL_Pos);

	// – programming the number of lines of the color frame buffer in the LTDC_LxCFBLNR register
	ModifyReg(pLTDCLayer->CFBLNR,LTDC_LxCFBLNR_CFBLNBR_Msk,Layers[layer].ImageHeight << LTDC_LxCFBLNR_CFBLNBR_Pos);
	
	// – if needed, loading the CLUT with the RGB values and its address in the LTDC_LxCLUTWR register
	pLTDCLayer->CR &= ~LTDC_LxCR_CLUTEN_Msk;
	
	// – If needed, configuring the default color and the blending factors respectively in the
	// LTDC_LxDCCR and LTDC_LxBFCR registers
	ModifyReg(pLTDCLayer->DCCR,
		LTDC_LxDCCR_DCBLUE_Msk | LTDC_LxDCCR_DCGREEN_Msk | LTDC_LxDCCR_DCRED_Msk | LTDC_LxDCCR_DCALPHA_Msk,
		0x0f<<LTDC_LxDCCR_DCBLUE_Pos | 0x0f<<LTDC_LxDCCR_DCGREEN_Pos| 0x0f<<LTDC_LxDCCR_DCRED_Pos | 0x0f<<LTDC_LxDCCR_DCALPHA_Pos);

	// Specifies the blending factors 
	ModifyReg(pLTDCLayer->BFCR,
		LTDC_LxBFCR_BF1_Msk | LTDC_LxBFCR_BF2_Msk,
		((uint32_t)Layers[layer].BlendingFactor1 << LTDC_LxBFCR_BF1_Pos) | ((uint32_t)Layers[layer].BlendingFactor2 << LTDC_LxBFCR_BF2_Pos));
	
	ModifyReg(pLTDCLayer->CR, LTDC_LxCR_LEN_Msk, LTDC_LxCR_LEN);

	// Reload the shadow registers to active register through the LTDC_SRCR register.
	LTDC->SRCR |= LTDC_SRCR_IMR;
}

void hwLTDC::Init(void *pRGBData1, void *pRGBData2)
{
	// 1. Enable the LTDC clock in the RCC register.
	RCC->APB3ENR |= RCC_APB3ENR_LTDCEN;

	// 2. Configure the required pixel clock following the panel datasheet.
	// Done in the RCC setup
	
	// 3. Configure the synchronous timings: VSYNC, HSYNC, vertical and horizontal back
		// porch, active data area and the front porch timings following the panel datasheet 
		// as described in the Section 33.4.1.
	SynchronousTimings();

	// 4. Configure the synchronous signals and clock polarity in the LTDC_GCR register.
	// Configure the HS, VS, DE and PC polarity 
	ClockPolarity();

	// 5. If needed, configure the background color in the LTDC_BCCR register.
	BackgroundColor(0, 0, 0);
	
	// 6. Configure the needed interrupts in the LTDC_IER and LTDC_LIPCR register.
	
	// 7. Configure the layer1 / 2 parameters by :
	// 8. Enable layer1 / 2 and if needed the CLUT in the LTDC_LxCR register.
	Layer(1, pRGBData1);
	Layer(2, pRGBData2);
	
	// 9. If needed, enable dithering and color keying respectively in the LTDC_GCR and LTDC_LxCKCR registers.They can be also enabled on the fly.
	ModifyReg(LTDC->GCR, LTDC_GCR_DEN_Msk, 0);
	
	// 10. Reload the shadow registers to active register through the LTDC_SRCR register.
	LTDC->SRCR |= LTDC_SRCR_IMR;
	
	// 11. Enable the LTDC controller in the LTDC_GCR register.
	LTDC->GCR |= LTDC_GCR_LTDCEN;
	// DSI Wrapper Control Register
	DSI->WCR |= DSI_WCR_LTDCEN;
	// DSI Host Video Shadow Control Register
	DSI->VSCR |= DSI_VSCR_EN | DSI_VSCR_UR;
	
	// 12. All layer parameters can be modified on the fly except the CLUT.The new configuration
		// must be either reloaded immediately or during vertical blanking period by configuring
		// the LTDC_SRCR register.
	
}
