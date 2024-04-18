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

#include "LTDC.h"
#include "Utils.h"

// 3. Configure the synchronous timings: VSYNC, HSYNC, vertical and horizontal back
	// porch, active data area and the front porch timings following the panel datasheet 
	// as described in the Section 33.4.1.
void hwLTDC::TimingParameters()
{
	uint32_t TotalW = HSW, TotalH = VSH;
	// Set Synchronization size W,H
	assert(TotalW == 70);
	assert(TotalH == 10);
	ModifyReg(LTDC->SSCR,
		LTDC_SSCR_HSW_Msk | LTDC_SSCR_VSH_Msk,
		((HSW - 1) << LTDC_SSCR_HSW_Pos) | /*!< Horizontal Synchronization Width */
		((VSH - 1) << LTDC_SSCR_VSH_Pos)); /*!< Vertical Synchronization Height  */
	
	TotalW += HBP; TotalH += VBP;
	// Set Accumulated Back porch H,V 
	assert(TotalW == 230);
	assert(TotalH == 33);
	ModifyReg(LTDC->BPCR,
		LTDC_BPCR_AHBP_Msk | LTDC_BPCR_AVBP_Msk,
		((TotalW - 1) << LTDC_BPCR_AHBP_Pos) | /*!< Accumulated Horizontal Back Porch */
		((TotalH - 1) << LTDC_BPCR_AVBP_Pos)); /*!< Accumulated Vertical Back Porch   */

	TotalW += HACT; TotalH += VACT;
	// Set Accumulated Active Width W,H
	assert(TotalW == 1254);
	assert(TotalH == 633);
	ModifyReg(LTDC->AWCR,
		LTDC_AWCR_AAW_Msk | LTDC_AWCR_AAH_Msk,
		((TotalW - 1) << LTDC_AWCR_AAW_Pos) | /*!< Accumulated Active Width */
		((TotalH - 1) << LTDC_AWCR_AAH_Pos)); /*!< Accumulated Active height */

	TotalW += HFP; TotalH += VFP;
	// Set Total Width W,H
	assert(TotalW == 1414);
	assert(TotalH == 645);
	ModifyReg(LTDC->TWCR,
		LTDC_TWCR_TOTALW_Msk | LTDC_TWCR_TOTALH_Msk,
		((TotalW - 1) << LTDC_TWCR_TOTALW_Pos) | /*!< Total Width */
		((TotalH - 1) << LTDC_TWCR_TOTALH_Pos)); /*!< Total height */
}

// A constant background color (RGB888) can programmed.
// It is used for blending with the bottom layer.
void hwLTDC::BackgroundColor(uint8_t red, uint8_t green, uint8_t blue)
{
	LTDC->BCCR = (blue << LTDC_BCCR_BCBLUE_Pos) | (green << LTDC_BCCR_BCGREEN_Pos) | (red << LTDC_BCCR_BCRED_Pos);
}

// Every layer can have a default color in the format ARGB which is used outside the defined
// layer window or when a layer is disabled.
void hwLTDC::DefaultColor(R_t<uint8_t, 1, 2> layer, uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha)
{
	ModifyReg((layer == 1 ? LTDC_Layer1 : LTDC_Layer2)->DCCR,
		LTDC_LxDCCR_DCBLUE_Msk | LTDC_LxDCCR_DCGREEN_Msk | LTDC_LxDCCR_DCRED_Msk | LTDC_LxDCCR_DCALPHA_Msk,
		blue<<LTDC_LxDCCR_DCBLUE_Pos | green<<LTDC_LxDCCR_DCGREEN_Pos| red<<LTDC_LxDCCR_DCRED_Pos | alpha<<LTDC_LxDCCR_DCALPHA_Pos);
	LTDC->SRCR |= LTDC_SRCR_VBR;
	while ((LTDC->SRCR & LTDC_SRCR_VBR) != 0) __asm("");
}
// A color key (RGB) can be configured to be representative for a transparent pixel.
void hwLTDC::ColorKey(R_t<uint8_t, 1, 2> layer, uint8_t red, uint8_t green, uint8_t blue)
{
	ModifyReg((layer == 1 ? LTDC_Layer1 : LTDC_Layer2)->CKCR,
		LTDC_LxCKCR_CKBLUE_Msk | LTDC_LxCKCR_CKGREEN_Msk | LTDC_LxCKCR_CKRED_Msk,
		blue<<LTDC_LxCKCR_CKBLUE_Pos | green<<LTDC_LxCKCR_CKGREEN_Pos| red<<LTDC_LxCKCR_CKRED_Pos);
	LTDC->SRCR |= LTDC_SRCR_VBR;
	while ((LTDC->SRCR & LTDC_SRCR_VBR) != 0) __asm("");
}

// The blending is always active and the two layers can be blended following the blending factors
void hwLTDC::BlendingFactors(R_t<uint8_t, 1, 2> layer, eBlendingFactor1 BF1, eBlendingFactor2 BF2, uint8_t constantAlpha)
{
	(layer == 1 ? LTDC_Layer1 : LTDC_Layer2)->CACR = constantAlpha;
	(layer == 1 ? LTDC_Layer1 : LTDC_Layer2)->BFCR = ((uint32_t)BF1 << LTDC_LxBFCR_BF1_Pos) | ((uint32_t)BF2 << LTDC_LxBFCR_BF2_Pos);
	LTDC->SRCR |= LTDC_SRCR_VBR;
	while ((LTDC->SRCR & LTDC_SRCR_VBR) != 0) __asm("");
}

// 7. Configure the layer1 / 2 parameters by :
// 8. Enable layer1 / 2 and if needed the CLUT in the LTDC_LxCR register.
void hwLTDC::Layer(R_t<uint8_t, 1, 2> layer, void *pRGBData, uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{
	ePixelFormat PixelFormat = ePixelFormat::ARGB8888;
	uint8_t ConstantAlpha = 255; // used when BFx == Fx_CA
	eBlendingFactor1 BlendingFactor1 = eBlendingFactor1::F1_CA;
	eBlendingFactor2 BlendingFactor2 = eBlendingFactor2::F2_CA;
	uint8_t BackColorAlpha = 255; // used when BFx == Fx_PAXCA

	LTDC_Layer_TypeDef *pLTDCLayer = LTDC_Layer1;
	if (layer == 2)
		pLTDCLayer = LTDC_Layer2;

	if (pRGBData == NULL) // disable
	{
		pLTDCLayer->BFCR = 0x0000'0607;
		pLTDCLayer->CACR = 0; 
	
		ModifyReg(pLTDCLayer->CR, LTDC_LxCR_LEN_Msk, 0);
		return;
	}

	uint32_t W = x1 - x0;
	uint32_t H = y1 - y0;

	// – programming the layer window horizontal and vertical position in the
	// LTDC_LxWHPCR and LTDC_WVPCR registers.The layer window must be in the active data area.
	uint32_t XOff = ((LTDC->BPCR & LTDC_BPCR_AHBP_Msk) >> LTDC_BPCR_AHBP_Pos);
	uint32_t X0 = XOff + x0 + 1U;
	uint32_t X1 = XOff + x1;
	ModifyReg(pLTDCLayer->WHPCR,
		LTDC_LxWHPCR_WHSTPOS_Msk | LTDC_LxWHPCR_WHSPPOS_Msk, 
		(X0 << LTDC_LxWHPCR_WHSTPOS_Pos) | (X1 << LTDC_LxWHPCR_WHSPPOS_Pos));

	uint32_t YOff = ((LTDC->BPCR & LTDC_BPCR_AVBP_Msk) >> LTDC_BPCR_AVBP_Pos);
	uint32_t Y0  = YOff + y0 + 1U;
	uint32_t Y1 = YOff + y1;
	ModifyReg(pLTDCLayer->WVPCR,
		LTDC_LxWVPCR_WVSTPOS_Msk | LTDC_LxWVPCR_WVSPPOS_Msk, 
		(Y0 << LTDC_LxWVPCR_WVSTPOS_Pos) | (Y1 << LTDC_LxWVPCR_WVSPPOS_Pos));

	// – programming the pixel input format in the LTDC_LxPFCR register
	ModifyReg(pLTDCLayer->PFCR, LTDC_LxPFCR_PF_Msk, (uint32_t)PixelFormat << LTDC_LxPFCR_PF_Pos);

	// – programming the color frame buffer start address in the LTDC_LxCFBAR register
	pLTDCLayer->CFBAR = (uint32_t)pRGBData;

	// – programming the line length and pitch of the color frame buffer in the LTDC_LxCFBLR register
	uint32_t BytesPerPixel;
	switch (PixelFormat)
	{
	case ePixelFormat::ARGB8888:
		BytesPerPixel = 4;
		break;
	case ePixelFormat::RGB888:
		BytesPerPixel = 3;
		break;
	case ePixelFormat::ARGB4444:
	case ePixelFormat::RGB565:
	case ePixelFormat::ARGB1555:
	case ePixelFormat::AL88:
		BytesPerPixel = 2;
		break;
	default:
		BytesPerPixel = 1;
		break;
	}
	// LTDC Layerx Color Frame Buffer Length Register
	uint32_t LP = W * BytesPerPixel;
	assert((LP % 64) == 0 && "Wrong line pitch");
	pLTDCLayer->CFBLR = 
		(LP) << LTDC_LxCFBLR_CFBP_Pos | 
		(LP + 0U) << LTDC_LxCFBLR_CFBLL_Pos;

	// – programming the number of lines of the color frame buffer in the LTDC_LxCFBLNR register
	pLTDCLayer->CFBLNR = H;
	
	// – if needed, loading the CLUT with the RGB values and its address in the LTDC_LxCLUTWR register
	pLTDCLayer->CR &= ~LTDC_LxCR_CLUTEN_Msk;
	
	// – If needed, configuring the default color and the blending factors respectively in the
	// LTDC_LxDCCR and LTDC_LxBFCR registers
	DefaultColor(layer, 0, 0, 0, BackColorAlpha);

	// Specifies the blending factors 
	BlendingFactors(layer, BlendingFactor1, BlendingFactor2, ConstantAlpha);

	ModifyReg(pLTDCLayer->CR, LTDC_LxCR_LEN_Msk, LTDC_LxCR_LEN);

	// Reload the shadow registers to active register through the LTDC_SRCR register.
	LTDC->SRCR |= LTDC_SRCR_VBR;
	while ((LTDC->SRCR & LTDC_SRCR_VBR) != 0) __asm("");
	
	// clear RGB buffer 
	uint32_t *p = (uint32_t *)pRGBData;
	uint32_t *e = p + (H*W*BytesPerPixel) / sizeof(uint32_t);
	while (p < e) *p++ = 0;
}

void hwLTDC::Init(/*void *pRGBData1, void *pRGBData2*/)
{
	hwDSI::Init();
	
	// 1. Enable the LTDC clock in the RCC register.
	RCC->APB3ENR |= RCC_APB3ENR_LTDCEN;

	// 2. Configure the required pixel clock following the panel datasheet.
	// Done in the RCC setup
	
	// 3. Configure the synchronous timings: VSYNC, HSYNC, vertical and horizontal back
		// porch, active data area and the front porch timings following the panel datasheet 
		// as described in the Section 33.4.1.
	TimingParameters();

	// 4. Configure the synchronous signals and clock polarity in the LTDC_GCR register.
	// Configure the HS, VS, DE and PC polarity 
	LTDC->GCR &= ~(LTDC_GCR_HSPOL_Msk | LTDC_GCR_VSPOL_Msk | LTDC_GCR_DEPOL_Msk | LTDC_GCR_PCPOL_Msk);
	LTDC->GCR |= HSPolarityHigh ? LTDC_GCR_HSPOL : 0 | 
		VSPolarityHigh ? LTDC_GCR_VSPOL : 0 | 
	    !DEPolarityHigh ? LTDC_GCR_DEPOL : 0 | 
		PCPolarityInverted ? LTDC_GCR_PCPOL : 0;

	// 5. If needed, configure the background color in the LTDC_BCCR register.
	BackgroundColor(0, 0, 0);
	
	// 6. Configure the needed interrupts in the LTDC_IER and LTDC_LIPCR register.
	
	// 7. Configure the layer1 / 2 parameters by :
	// 8. Enable layer1 / 2 and if needed the CLUT in the LTDC_LxCR register.
//	Layer(1, pRGBData1);
//	Layer(2, pRGBData2);
	
	// 9. If needed, enable dithering and color keying respectively in the LTDC_GCR and LTDC_LxCKCR registers.They can be also enabled on the fly.
	ModifyReg(LTDC->GCR, LTDC_GCR_DEN_Msk, 0);
	
	// 10. Reload the shadow registers to active register through the LTDC_SRCR register.
	LTDC->SRCR |= LTDC_SRCR_IMR;
	
	// 11. Enable the LTDC controller in the LTDC_GCR register.
	LTDC->GCR |= LTDC_GCR_LTDCEN;

	// DSI Host Video Shadow Control Register
	//DSI->VSCR |= DSI_VSCR_EN | DSI_VSCR_UR;
	
	// 12. All layer parameters can be modified on the fly except the CLUT.The new configuration
		// must be either reloaded immediately or during vertical blanking period by configuring
		// the LTDC_SRCR register.
	LTDC->SRCR |= LTDC_SRCR_VBR;
	
	LTDC->LIPCR = LTDC->AWCR & LTDC_AWCR_AAH_Msk;
	LTDC->IER |= LTDC_IER_LIE; // | LTDC_IER_FUIE;
	//	NVIC_SetPriority(LTDC_IRQn, (1UL << __NVIC_PRIO_BITS) - 1UL);
	NVIC_EnableIRQ(LTDC_IRQn);
}

hwLTDC_IRQs *phwLTDC_IRQs;

extern "C" void LTDC_IRQHandler()
{
	if ((LTDC->ISR & LTDC_ISR_LIF) != 0)
	{
		LTDC->ICR |= LTDC_ICR_CLIF;
		phwLTDC_IRQs->cbBlanking();
	}
	if ((LTDC->ISR & LTDC_ISR_FUIF) != 0) 
	{
		Printf("LTDC FIFO underrun \n");
	}
}