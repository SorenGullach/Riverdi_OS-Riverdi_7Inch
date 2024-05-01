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

#include <RCC.h>
#include <PWR.h>

// instantiate static vars
hwCC hwSysClock::CC;
hwCC::PLL<1> hwCC::PLL1;
hwCC::PLL<2> hwCC::PLL2;
hwCC::PLL<3> hwCC::PLL3;
hwCC::hwSCGU hwCC::SCGU;

void hwCC::DumpClocks()
{
	SystemCoreClockUpdate();
	if (HSE_clk > 0) Printf("HSE_clk %u\n", HSE_clk);
	if (LSE_clk > 0) Printf("LSE_clk %u\n", LSE_clk);
	if (HSI_clk > 0) Printf("HSI_clk %u\n", HSI_clk);
	if (CSI_clk > 0) Printf("CSI_clk %u\n", CSI_clk);
	if (HSI48_clk > 0) Printf("HSI48_clk %u\n", HSI48_clk);
	if (LSI_clk > 0) Printf("LSI_clk %u\n", LSI_clk);
	if (SYS_clk > 0) Printf("SYS_clk %u\n", SYS_clk);
	if (RTC_clk > 0) Printf("RTC_clk %u\n", RTC_clk);
	if (HSE_1M_clk > 0) Printf("HSE_1M_clk %u\n", HSE_1M_clk);
	if (PER_clk > 0) Printf("PER_clk %u\n", PER_clk);
	
	PLL1.DumpClocks();
	PLL2.DumpClocks();
	PLL3.DumpClocks();
	
	SCGU.DumpClocks();
	
}

void hwCC::SystemCoreClockUpdate()
{
	LSI_clk = 0;
	if ((RCC->CSR & RCC_CSR_LSION) != 0)
		LSI_clk = LSI_VALUE;

	LSE_clk = 0;
	if ((RCC->BDCR & RCC_BDCR_LSEON) != 0)
		LSE_clk = LSE_VALUE;

	HSE_clk = 0;
	if ((RCC->CR & RCC_CR_HSERDY) != 0)
		HSE_clk = HSE_VALUE;

	HSE_1M_clk = 0;
	uint32_t div = (RCC->CFGR & RCC_CFGR_RTCPRE) >> RCC_CFGR_RTCPRE_Pos;
	if (div >= 2)
		HSE_1M_clk = HSE_clk / div;
	assert(HSE_1M_clk <= 1'000'000);
	
	RTC_clk = 0;
	uint32_t sel = (RCC->BDCR & RCC_BDCR_RTCSEL_Msk) >> RCC_BDCR_RTCSEL_Pos;
	switch (sel)
	{
	default:
	case 0:
		RTC_clk = 0;
		break;
	case 1:
		RTC_clk = LSE_clk;
		break;
	case 2:
		RTC_clk = LSI_clk;
		break;
	case 3:
		RTC_clk = HSE_1M_clk;
		break;
	}

	HSI_clk = 0;
	if ((RCC->CR & RCC_CR_HSIRDY) != 0)
	{
		switch (RCC->CR & RCC_CR_HSIDIV_Msk)
		{
		default:
		case RCC_CR_HSIDIV_1:
			HSI_clk = HSI_VALUE / 1;
			break;
		case RCC_CR_HSIDIV_2:
			HSI_clk = HSI_VALUE / 2;
			break;
		case RCC_CR_HSIDIV_4:
			HSI_clk = HSI_VALUE / 4;
			break;
		case RCC_CR_HSIDIV_8:
			HSI_clk = HSI_VALUE / 8;
			break;
		}
	}

	CSI_clk = 0;
	if ((RCC->CR & RCC_CR_CSIRDY) != 0)
		CSI_clk = HSI_VALUE;

	HSI48_clk = 0;
	if ((RCC->CR & RCC_CR_HSI48RDY) != 0)
		HSI48_clk = 48'000'000;
		
	PER_clk = 0;		
	sel = (RCC->D1CCIPR & RCC_D1CCIPR_CKPERSEL) >> RCC_D1CCIPR_CKPERSEL_Pos;
	switch (sel)
	{
	default:
	case 0:
		PER_clk = HSI_clk;		
		break;
	case 1:
		PER_clk = CSI_clk;		
		break;
	case 2:
		PER_clk = HSE_clk;		
		break;
	case 3:
		PER_clk = 0;		
		break;
	}
	
	PLL1.ClockUpdate();
	PLL2.ClockUpdate();
	PLL3.ClockUpdate();

	SYS_clk = 0;
	switch (RCC->CFGR & RCC_CFGR_SWS_Msk)
	{
	case RCC_CFGR_SWS_HSI:
		SYS_clk = HSI_clk;
		break;
	case RCC_CFGR_SWS_CSI:
		SYS_clk = CSI_clk;
		break;
	case RCC_CFGR_SWS_HSE:
		SYS_clk = HSE_clk;
		break;
	case RCC_CFGR_SWS_PLL1:
		SYS_clk = PLL1.P_clk;
		break;
	}

	SCGU.ClockUpdate();
}
void hwCC::HSE(hwCC::eHSE hse, uint32_t clk)
{
	switch (hse)
	{
	case hwCC::eHSE::Off:
		ModifyReg(RCC->CR, RCC_CR_HSEBYP_Msk | RCC_CR_HSEON_Msk, 0);
		HSE_clk = 0;
		return;
	case hwCC::eHSE::Bypass:
		ModifyReg(RCC->CR, RCC_CR_HSEBYP_Msk | RCC_CR_HSEON_Msk, RCC_CR_HSEBYP | RCC_CR_HSEON);
		break;
	case hwCC::eHSE::External:
		ModifyReg(RCC->CR, RCC_CR_HSEBYP_Msk | RCC_CR_HSEON_Msk, RCC_CR_HSEON);
		break;
	}
	while ((RCC->CR & RCC_CR_HSERDY) == 0) __asm(""); // wait for ready
	HSE_clk = clk;
}

void hwCC::LSE(eHSE hse, uint32_t clk)
{
	switch (hse)
	{
	case eHSE::Off:
		ModifyReg(RCC->BDCR, RCC_BDCR_LSEBYP_Msk | RCC_BDCR_LSEON_Msk, 0);
		LSE_clk = 0;
		return;
	case eHSE::Bypass:
		ModifyReg(RCC->BDCR, RCC_BDCR_LSEBYP_Msk | RCC_BDCR_LSEON_Msk, RCC_BDCR_LSEBYP | RCC_BDCR_LSEON);
		break;
	case eHSE::External:
		ModifyReg(RCC->BDCR, RCC_BDCR_LSEBYP_Msk | RCC_BDCR_LSEON_Msk, RCC_BDCR_LSEON);
		break;
	}
	while ((RCC->BDCR & RCC_BDCR_LSERDY) == 0) __asm(""); // wait for ready
	LSE_clk = clk;
}

void hwCC::HSI(eHSI hsi)
{
	switch (hsi)
	{
	case eHSI::Off:
		ModifyReg(RCC->CR, RCC_CR_HSION_Msk, 0);
		HSI_clk = 0;
		return;
	case eHSI::Mhz8:
		ModifyReg(RCC->CR, RCC_CR_HSIDIV_Msk | RCC_CR_HSION_Msk, RCC_CR_HSION | RCC_CR_HSIDIV_8);
		HSI_clk = 8'000'000;
		break;
	case eHSI::Mhz16:
		ModifyReg(RCC->CR, RCC_CR_HSIDIV_Msk | RCC_CR_HSION_Msk, RCC_CR_HSION | RCC_CR_HSIDIV_4);
		HSI_clk = 16'000'000;
		break;
	case eHSI::Mhz32:
		ModifyReg(RCC->CR, RCC_CR_HSIDIV_Msk | RCC_CR_HSION_Msk, RCC_CR_HSION | RCC_CR_HSIDIV_2);
		HSI_clk = 32'000'000;
		break;
	case eHSI::Mhz64:
		ModifyReg(RCC->CR, RCC_CR_HSIDIV_Msk | RCC_CR_HSION_Msk, RCC_CR_HSION | RCC_CR_HSIDIV_1);
		HSI_clk = 64'000'000;
		break;
	}
	while ((RCC->CR & RCC_CR_HSIRDY) == 0) __asm(""); // wait for ready
}

void hwCC::CSI(eHSE hse)
{
	switch (hse)
	{
	case eHSE::Off:
		ModifyReg(RCC->CR, RCC_CR_CSION_Msk, 0);
		CSI_clk = 0;
		return;
	case eHSE::Bypass:
	case eHSE::External:
		ModifyReg(RCC->CR, RCC_CR_CSION_Msk, RCC_CR_CSION);
		break;
	}
	while ((RCC->CR & RCC_CR_CSIRDY) == 0) __asm(""); // wait for ready
	CSI_clk = CSI_VALUE;
}

void hwCC::HSI48(bool on)
{
	if (!on)
	{
		ModifyReg(RCC->CR, RCC_CR_HSI48ON_Msk, 0);
		while ((RCC->CR & RCC_CR_HSI48ON_Msk) != 0) __asm(""); // wait for ready
		HSI48_clk = 0;
		return;
	}
	ModifyReg(RCC->CR, RCC_CR_HSI48ON_Msk, RCC_CR_HSI48ON);
	while ((RCC->CR & RCC_CR_HSI48RDY_Msk) == 0) __asm(""); // wait for ready
	HSI48_clk = 48'000'000;
}

void hwCC::LSI(bool on)
{
	if (!on)
	{
		ModifyReg(RCC->CSR, RCC_CSR_LSION_Msk, 0);
		LSI_clk = 0;
		return;
	}
	ModifyReg(RCC->CSR, RCC_CSR_LSION_Msk, RCC_CSR_LSION);
	while ((RCC->CSR & RCC_CSR_LSIRDY_Msk) == 0) __asm(""); // wait for ready
	LSI_clk = 32'000;
}

void hwCC::PLLSource(ePLL_SRC src)
{
	ModifyReg(RCC->CR, RCC_CR_PLL1ON | RCC_CR_PLL2ON | RCC_CR_PLL3ON, 0); // Disable PLLs
	uint32_t div = 32; // default div
	ModifyReg(RCC->PLLCKSELR,
		RCC_PLLCKSELR_DIVM1_Msk | RCC_PLLCKSELR_DIVM2_Msk | RCC_PLLCKSELR_DIVM3_Msk,// prescaler disabled
		(div << RCC_PLLCKSELR_DIVM1_Pos) | (div << RCC_PLLCKSELR_DIVM2_Pos) | (div << RCC_PLLCKSELR_DIVM3_Pos)); // default div

	switch (src)
	{
	case ePLL_SRC::None:
		ModifyReg(RCC->PLLCKSELR, RCC_PLLCKSELR_PLLSRC_Msk, RCC_PLLCKSELR_PLLSRC_NONE);
		PLL1.Src_clk = PLL2.Src_clk = PLL3.Src_clk = 0;
		return;
	case ePLL_SRC::HSI:
		ModifyReg(RCC->PLLCKSELR, RCC_PLLCKSELR_PLLSRC_Msk, RCC_PLLCKSELR_PLLSRC_HSI);
		PLL1.Src_clk = PLL2.Src_clk = PLL3.Src_clk = HSI_clk;
		break;
	case ePLL_SRC::CSI:
		ModifyReg(RCC->PLLCKSELR, RCC_PLLCKSELR_PLLSRC_Msk, RCC_PLLCKSELR_PLLSRC_CSI);
		PLL1.Src_clk = PLL2.Src_clk = PLL3.Src_clk = CSI_clk;
		break;
	case ePLL_SRC::HSE:
		ModifyReg(RCC->PLLCKSELR, RCC_PLLCKSELR_PLLSRC_Msk, RCC_PLLCKSELR_PLLSRC_HSE);
		PLL1.Src_clk = PLL2.Src_clk = PLL3.Src_clk = HSE_clk;
		break;
	}
}

void hwCC::SYSClkSource(eSysClkSource src)
{
//	Printf("1\n");
	switch (src)
	{
	case eSysClkSource::HSI:
		assert((RCC->CR & RCC_CR_HSIRDY) != 0 && HSI_clk != 0);

		ModifyReg(RCC->CFGR, RCC_CFGR_SW_Msk, RCC_CFGR_SW_HSI);
		while ((RCC->CFGR & RCC_CFGR_SWS_HSI) != 0) __asm(""); // Wait for selected - note value 0
		SYS_clk = HSI_clk;
		break;
	case eSysClkSource::CSI:
		assert((RCC->CR & RCC_CR_CSIRDY) != 0 && CSI_clk != 0);

		ModifyReg(RCC->CFGR, RCC_CFGR_SW_Msk, RCC_CFGR_SW_CSI);
		while ((RCC->CFGR & RCC_CFGR_SWS_CSI) == 0) __asm(""); // Wait for selected
		SYS_clk = CSI_clk;
		break;
	case eSysClkSource::HSE:
		assert((RCC->CR & RCC_CR_HSERDY) != 0 && HSE_clk != 0);

		ModifyReg(RCC->CFGR, RCC_CFGR_SW_Msk, RCC_CFGR_SW_HSE);
		while ((RCC->CFGR & RCC_CFGR_SWS_HSE) == 0) __asm(""); // Wait for selected
		SYS_clk = HSE_clk;
		break;
	case eSysClkSource::PLL1:
		assert(PLL1.P_clk != 0);
		assert(PLL1.IsEnabled());

		ModifyReg(RCC->CFGR, RCC_CFGR_SW_Msk, RCC_CFGR_SW_PLL1);
		while ((RCC->CFGR & RCC_CFGR_SWS_PLL1) == 0) __asm(""); // Wait for selected
		SYS_clk = PLL1.P_clk;
		break;
	}
	Printf("2\n");
	// adjust C1_clk
	SCGU.C1Clk();
	Printf("3\n");
	// adjust C2_clk
	SCGU.C2Clk();
//	Printf("4\n");
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
template <int Idx>
	void hwCC::PLL<Idx>::DumpClocks()
	{
		if (IsEnabled())
		{
			Printf("   PLL%d\n", Idx);
			Printf("   Src_clk %d\n", Src_clk);
			Printf("   Ref_clk %d\n", Ref_clk);
			Printf("   P_clk %d\n", P_clk);
			Printf("   Q_clk %d\n", Q_clk);
			Printf("   R_clk %d\n", R_clk);
			if(Idx == 3)
				Printf("    LTDC clk %d\n", R_clk);
		}
	}

template <int Idx>
	void hwCC::PLL<Idx>::ClockUpdate()
	{
		switch (RCC->PLLCKSELR & RCC_PLLCKSELR_PLLSRC_Msk)
		{
		case RCC_PLLCKSELR_PLLSRC_HSI:
			Src_clk = hwSysClock::CC.HSI_clk;
			break;
		case RCC_PLLCKSELR_PLLSRC_CSI:
			Src_clk = hwSysClock::CC.CSI_clk;
			break;
		case RCC_PLLCKSELR_PLLSRC_HSE:
			Src_clk = hwSysClock::CC.HSE_clk;
			break;
		case RCC_PLLCKSELR_PLLSRC_NONE:
			Src_clk = 0;
			Ref_clk = 0;
			break;
		}

		uint8_t divM, divN, divP, divQ, divR;
		switch (Idx)
		{
		case 1:
			divM = (RCC->PLLCKSELR & RCC_PLLCKSELR_DIVM1_Msk) >> RCC_PLLCKSELR_DIVM1_Pos;
			divN = (RCC->PLL1DIVR & RCC_PLL1DIVR_N1_Msk) >> RCC_PLL1DIVR_N1_Pos;
			divP = (RCC->PLL1DIVR & RCC_PLL1DIVR_P1_Msk) >> RCC_PLL1DIVR_P1_Pos;
			divQ = (RCC->PLL1DIVR & RCC_PLL1DIVR_Q1_Msk) >> RCC_PLL1DIVR_Q1_Pos;
			divR = (RCC->PLL1DIVR & RCC_PLL1DIVR_R1_Msk) >> RCC_PLL1DIVR_R1_Pos;
			break;
		case 2:
			divM = (RCC->PLLCKSELR & RCC_PLLCKSELR_DIVM2_Msk) >> RCC_PLLCKSELR_DIVM2_Pos;
			divN = (RCC->PLL2DIVR & RCC_PLL2DIVR_N2_Msk) >> RCC_PLL2DIVR_N2_Pos;
			divP = (RCC->PLL2DIVR & RCC_PLL2DIVR_P2_Msk) >> RCC_PLL2DIVR_P2_Pos;
			divQ = (RCC->PLL2DIVR & RCC_PLL2DIVR_Q2_Msk) >> RCC_PLL2DIVR_Q2_Pos;
			divR = (RCC->PLL2DIVR & RCC_PLL2DIVR_R2_Msk) >> RCC_PLL2DIVR_R2_Pos;
			break;
		case 3:
			divM = (RCC->PLLCKSELR & RCC_PLLCKSELR_DIVM3_Msk) >> RCC_PLLCKSELR_DIVM3_Pos;
			divN = (RCC->PLL3DIVR & RCC_PLL3DIVR_N3_Msk) >> RCC_PLL3DIVR_N3_Pos;
			divP = (RCC->PLL3DIVR & RCC_PLL3DIVR_P3_Msk) >> RCC_PLL3DIVR_P3_Pos;
			divQ = (RCC->PLL3DIVR & RCC_PLL3DIVR_Q3_Msk) >> RCC_PLL3DIVR_Q3_Pos;
			divR = (RCC->PLL3DIVR & RCC_PLL3DIVR_R3_Msk) >> RCC_PLL3DIVR_R3_Pos;
			break;
		}
		Ref_clk = Src_clk / divM;
		VCO_clk = Ref_clk * (divN + 1);
		P_clk = VCO_clk / (divP + 1);
		Q_clk = VCO_clk / (divQ + 1);
		R_clk = VCO_clk / (divR + 1);
	}

template <int Idx>
	uint32_t  hwCC::PLL<Idx>::PllInClk(R_t<uint32_t, 1'000'000, 16'000'000> clk)
	{
		assert(!IsEnabled());
		uint32_t divMx = Src_clk / clk;
		assert(divMx >= 1 && divMx <= 32);
		DivM(divMx);
		return divMx;
	}

template <int Idx>
	void hwCC::PLL<Idx>::DivM(R_t<uint8_t, 0, 63> divMx)
	{
		assert(!IsEnabled());
		Ref_clk = Src_clk / divMx;
		switch (Idx)
		{
		case 1:
			ModifyReg(RCC->PLLCKSELR, RCC_PLLCKSELR_DIVM1_Msk, (uint32_t)divMx << RCC_PLLCKSELR_DIVM1_Pos);
			break;
		case 2:
			ModifyReg(RCC->PLLCKSELR, RCC_PLLCKSELR_DIVM2_Msk, (uint32_t)divMx << RCC_PLLCKSELR_DIVM2_Pos);
			break;
		case 3:
			ModifyReg(RCC->PLLCKSELR, RCC_PLLCKSELR_DIVM3_Msk, (uint32_t)divMx << RCC_PLLCKSELR_DIVM3_Pos);
			break;
		}
		assert(Ref_clk >= 1'000'000 && Ref_clk <= 16'000'000);
	}

template <int Idx>
	void hwCC::PLL<Idx>::SetupInteger(R_t<uint16_t, 4, 512> mulNx, R_t<uint8_t, 1, 128> divPx, R_t<uint8_t, 1, 128> divQx, R_t<uint8_t, 1, 128> divRx)
	{
		assert(!IsEnabled());
		VCO_clk = Ref_clk * mulNx;
		P_clk = VCO_clk / divPx;
		Q_clk = VCO_clk / divQx;
		R_clk = VCO_clk / divRx;

#ifdef DEBUG
		switch ((PWR->CSR1 & PWR_CSR1_ACTVOS_Msk) >> PWR_CSR1_ACTVOS_Pos)
		{
		case 0: // VOS3
		case 1: // VOS3
			assert(P_clk <= 200'000'000);
			break;
		case 2: // VOS2
			assert(P_clk <= 300'000'000);
			break;
		case 3: // VOS1
			if (SYSCFG->PWRCR & SYSCFG_PWRCR_ODEN)
				assert(P_clk <= 480'000'000);
			else
				assert(P_clk <= 400'000'000);
			break;
		}
		if (Ref_clk > 2'000'000)
			assert(VCO_clk >= 192'000'000 && VCO_clk <= 960'000'000);
		else
			assert(VCO_clk >= 150'000'000 && VCO_clk <= 420'000'000);
#endif					

		assert(mulNx >= 4 && mulNx <= 512);
		assert(divPx >= 1 && divPx <= 128);
		assert(divQx >= 1 && divQx <= 128);
		assert(divRx >= 2 && divRx <= 128);

		switch (Idx)
		{
		case 1:
			if (Ref_clk > 2'000'000)
				ModifyReg(RCC->PLLCFGR, RCC_PLLCFGR_PLL1VCOSEL, 0); // VCOH
			else
				ModifyReg(RCC->PLLCFGR, 0, RCC_PLLCFGR_PLL1VCOSEL); // VCOL

			// PLL input frequency range
			if (Ref_clk < 2'000'000)
				ModifyReg(RCC->PLLCFGR, RCC_PLLCFGR_PLL1RGE_Msk, RCC_PLLCFGR_PLL1RGE_0);
			else if (Ref_clk < 4'000'000)
				ModifyReg(RCC->PLLCFGR, RCC_PLLCFGR_PLL1RGE_Msk, RCC_PLLCFGR_PLL1RGE_1);
			else if (Ref_clk < 8'000'000)
				ModifyReg(RCC->PLLCFGR, RCC_PLLCFGR_PLL1RGE_Msk, RCC_PLLCFGR_PLL1RGE_2);
			else
				ModifyReg(RCC->PLLCFGR, RCC_PLLCFGR_PLL1RGE_Msk, RCC_PLLCFGR_PLL1RGE_3);
						
			ModifyReg(RCC->PLLCFGR, RCC_PLLCFGR_PLL1FRACEN_Msk, 0); // integer mode
			ModifyReg(RCC->PLL1DIVR, RCC_PLL1DIVR_N1_Msk, (mulNx - 1) << RCC_PLL1DIVR_N1_Pos);

			assert(divPx != 3); // ??
			ModifyReg(RCC->PLLCFGR, RCC_PLLCFGR_DIVP1EN, 0); // disable
			ModifyReg(RCC->PLL1DIVR, 
				RCC_PLL1DIVR_P1_Msk | 
				RCC_PLL1DIVR_Q1_Msk |
				RCC_PLL1DIVR_R1_Msk,
				(divPx - 1) << RCC_PLL1DIVR_P1_Pos |
				(divQx - 1) << RCC_PLL1DIVR_Q1_Pos |
				(divRx - 1) << RCC_PLL1DIVR_R1_Pos);
			ModifyReg(RCC->PLLCFGR, 0, RCC_PLLCFGR_DIVP1EN); // enable
			break;
		case 2:
			if (Ref_clk > 2'000'000)
				ModifyReg(RCC->PLLCFGR, RCC_PLLCFGR_PLL2VCOSEL, 0); // VCOH
			else
				ModifyReg(RCC->PLLCFGR, 0, RCC_PLLCFGR_PLL2VCOSEL); // VCOL

			// PLL input frequency range
			if (Ref_clk < 2'000'000)
				ModifyReg(RCC->PLLCFGR, RCC_PLLCFGR_PLL2RGE_Msk, RCC_PLLCFGR_PLL2RGE_0);
			else if (Ref_clk < 4'000'000)
				ModifyReg(RCC->PLLCFGR, RCC_PLLCFGR_PLL2RGE_Msk, RCC_PLLCFGR_PLL2RGE_1);
			else if (Ref_clk < 8'000'000)
				ModifyReg(RCC->PLLCFGR, RCC_PLLCFGR_PLL2RGE_Msk, RCC_PLLCFGR_PLL2RGE_2);
			else
				ModifyReg(RCC->PLLCFGR, RCC_PLLCFGR_PLL2RGE_Msk, RCC_PLLCFGR_PLL2RGE_3);

			ModifyReg(RCC->PLLCFGR, RCC_PLLCFGR_PLL2FRACEN_Msk, 0); // integer mode
			ModifyReg(RCC->PLL2DIVR, RCC_PLL2DIVR_N2_Msk, (mulNx - 1) << RCC_PLL2DIVR_N2_Pos);

			ModifyReg(RCC->PLLCFGR, RCC_PLLCFGR_DIVP2EN, 0); // disable
			ModifyReg(RCC->PLL2DIVR, 
				RCC_PLL2DIVR_P2_Msk | 
				RCC_PLL2DIVR_Q2_Msk |
				RCC_PLL2DIVR_R2_Msk,
				(divPx - 1) << RCC_PLL2DIVR_P2_Pos |
				(divQx - 1) << RCC_PLL2DIVR_Q2_Pos |
				(divRx - 1) << RCC_PLL2DIVR_R2_Pos);
			ModifyReg(RCC->PLLCFGR, 0, RCC_PLLCFGR_DIVR2EN); // enable

			break;
		case 3:
			if (Ref_clk > 2'000'000)
				ModifyReg(RCC->PLLCFGR, RCC_PLLCFGR_PLL3VCOSEL, 0); // VCOH
			else
				ModifyReg(RCC->PLLCFGR, 0, RCC_PLLCFGR_PLL3VCOSEL); // VCOL

			RCC->PLLCFGR &= ~RCC_PLLCFGR_PLL3RGE_Msk;
			// PLL input frequency range
			if (Ref_clk < 2'000'000)
				ModifyReg(RCC->PLLCFGR, RCC_PLLCFGR_PLL3RGE_Msk, RCC_PLLCFGR_PLL3RGE_0);
			else if (Ref_clk < 4'000'000)
				ModifyReg(RCC->PLLCFGR, RCC_PLLCFGR_PLL3RGE_Msk, RCC_PLLCFGR_PLL3RGE_1);
			else if (Ref_clk < 8'000'000)
				ModifyReg(RCC->PLLCFGR, RCC_PLLCFGR_PLL3RGE_Msk, RCC_PLLCFGR_PLL3RGE_2);
			else
				ModifyReg(RCC->PLLCFGR, RCC_PLLCFGR_PLL3RGE_Msk, RCC_PLLCFGR_PLL3RGE_3);

			ModifyReg(RCC->PLLCFGR, RCC_PLLCFGR_PLL3FRACEN_Msk, 0); // integer mode
			ModifyReg(RCC->PLL3DIVR, RCC_PLL3DIVR_N3_Msk, (mulNx - 1) << RCC_PLL3DIVR_N3_Pos);

			ModifyReg(RCC->PLLCFGR, RCC_PLLCFGR_DIVP3EN, 0); // disable
			ModifyReg(RCC->PLL3DIVR, 
				RCC_PLL3DIVR_P3_Msk | 
				RCC_PLL3DIVR_Q3_Msk |
				RCC_PLL3DIVR_R3_Msk,
				(divPx - 1) << RCC_PLL3DIVR_P3_Pos |
				(divQx - 1) << RCC_PLL3DIVR_Q3_Pos |
				(divRx - 1) << RCC_PLL3DIVR_R3_Pos);
			ModifyReg(RCC->PLLCFGR, 0, RCC_PLLCFGR_DIVP3EN); // enable
			break;
		}
		Enable();
	}

//////////////////////////////////////////////////////////////////////////////////////////////////////
void hwCC::hwSCGU::DumpClocks()
{
	Printf("  SCGU\n");
	Printf("  C1_clk %d\n", C1_clk);
	Printf("  C2_clk %d\n", C2_clk);
	Printf("  P1_clk %d\n", P1_clk);
	Printf("     P1T_clk %d\n", P1T_clk);
	Printf("  P2_clk %d\n", P2_clk);
	Printf("     P2T_clk %d\n", P2T_clk);
	Printf("  P3_clk %d\n", P3_clk);
	Printf("  P4_clk %d\n", P4_clk);
}

void hwCC::hwSCGU::ClockUpdate()
{
	switch (RCC->D1CFGR & RCC_D1CFGR_D1CPRE_Msk)
	{
	default:
		C1_clk = hwSysClock::CC.SYS_clk;
		break;
	case RCC_D1CFGR_D1CPRE_DIV2:
		C1_clk = hwSysClock::CC.SYS_clk / 2;
		break;
	case RCC_D1CFGR_D1CPRE_DIV4:
		C1_clk = hwSysClock::CC.SYS_clk / 4;
		break;
	case RCC_D1CFGR_D1CPRE_DIV8:
		C1_clk = hwSysClock::CC.SYS_clk / 8;
		break;
	case RCC_D1CFGR_D1CPRE_DIV16:
		C1_clk = hwSysClock::CC.SYS_clk / 16;
		break;
	case RCC_D1CFGR_D1CPRE_DIV64:
		C1_clk = hwSysClock::CC.SYS_clk / 64;
		break;
	case RCC_D1CFGR_D1CPRE_DIV128:
		C1_clk = hwSysClock::CC.SYS_clk / 128;
		break;
	case RCC_D1CFGR_D1CPRE_DIV256:
		C1_clk = hwSysClock::CC.SYS_clk / 256;
		break;
	case RCC_D1CFGR_D1CPRE_DIV512:
		C1_clk = hwSysClock::CC.SYS_clk / 512;
		break;
	}

	switch (RCC->D1CFGR & RCC_D1CFGR_HPRE_Msk)
	{
	default:
		C2_clk = C1_clk;
		break;
	case RCC_D1CFGR_HPRE_DIV2:
		C2_clk = C1_clk / 2;
		break;
	case RCC_D1CFGR_HPRE_DIV4:
		C2_clk = C1_clk / 4;
		break;
	case RCC_D1CFGR_HPRE_DIV8:
		C2_clk = C1_clk / 8;
		break;
	case RCC_D1CFGR_HPRE_DIV16:
		C2_clk = C1_clk / 16;
		break;
	case RCC_D1CFGR_HPRE_DIV64:
		C2_clk = C1_clk / 64;
		break;
	case RCC_D1CFGR_HPRE_DIV128:
		C2_clk = C1_clk / 128;
		break;
	case RCC_D1CFGR_HPRE_DIV256:
		C2_clk = C1_clk / 256;
		break;
	case RCC_D1CFGR_HPRE_DIV512:
		C2_clk = C1_clk / 512;
		break;
	}
	
	switch (RCC->D1CFGR & RCC_D1CFGR_D1PPRE_Msk)
	{
	default:
		P3_clk = C2_clk;
		break;
	case RCC_D1CFGR_D1PPRE_DIV2:
		P3_clk = C2_clk / 2;
		break;
	case RCC_D1CFGR_D1PPRE_DIV4:
		P3_clk = C2_clk / 4;
		break;
	case RCC_D1CFGR_D1PPRE_DIV8:
		P3_clk = C2_clk / 8;
		break;
	case RCC_D1CFGR_D1PPRE_DIV16:
		P3_clk = C2_clk / 16;
		break;
	}

	switch (RCC->D2CFGR & RCC_D2CFGR_D2PPRE2_Msk)
	{
	default:
		P2T_clk = P2_clk = C2_clk;
		break;
	case RCC_D2CFGR_D2PPRE2_DIV2:
		P2_clk = C2_clk / 2;
		P2T_clk = C2_clk;
		break;
	case RCC_D2CFGR_D2PPRE2_DIV4:
		P2_clk = C2_clk / 4;
		P2T_clk = (RCC->CFGR & RCC_CFGR_TIMPRE) != 0 ? C2_clk : C2_clk / 2;
		break;
	case RCC_D2CFGR_D2PPRE2_DIV8:
		P2_clk = C2_clk / 8;
		P2T_clk = (RCC->CFGR & RCC_CFGR_TIMPRE) != 0 ? C2_clk / 2 : C2_clk / 4;
		break;
	case RCC_D2CFGR_D2PPRE2_DIV16:
		P2_clk = C2_clk / 16;
		P2T_clk = (RCC->CFGR & RCC_CFGR_TIMPRE) != 0 ? C2_clk / 4 : C2_clk / 8;
		break;
	}
	
	switch (RCC->D2CFGR & RCC_D2CFGR_D2PPRE1_Msk)
	{
	default:
		P1T_clk = P1_clk = C2_clk;
		break;
	case RCC_D2CFGR_D2PPRE1_DIV2:
		P1_clk = C2_clk / 2;
		P1T_clk = C2_clk;
		break;
	case RCC_D2CFGR_D2PPRE1_DIV4:
		P1_clk = C2_clk / 4;
		P1T_clk = (RCC->CFGR & RCC_CFGR_TIMPRE) != 0 ? C2_clk : C2_clk / 2;
		break;
	case RCC_D2CFGR_D2PPRE1_DIV8:
		P1_clk = C2_clk / 8;
		P1T_clk = (RCC->CFGR & RCC_CFGR_TIMPRE) != 0 ? C2_clk / 2 : C2_clk / 4;
		break;
	case RCC_D2CFGR_D2PPRE1_DIV16:
		P1_clk = C2_clk / 16;
		P1T_clk = (RCC->CFGR & RCC_CFGR_TIMPRE) != 0 ? C2_clk / 4 : C2_clk / 8;
		break;
	}

	switch (RCC->D3CFGR & RCC_D3CFGR_D3PPRE_Msk)
	{
	default:
		P4_clk = C2_clk;
		break;
	case RCC_D3CFGR_D3PPRE_DIV2:
		P4_clk = C2_clk / 2;
		break;
	case RCC_D3CFGR_D3PPRE_DIV4:
		P4_clk = C2_clk / 4;
		break;
	case RCC_D3CFGR_D3PPRE_DIV8:
		P4_clk = C2_clk / 8;
		break;
	case RCC_D3CFGR_D3PPRE_DIV16:
		P4_clk = C2_clk / 16;
		break;
	}

}

void hwCC::hwSCGU::C1Clk(uint32_t freq)
{
	if (freq > 480'000'000)
		freq = 480'000'000;

	// calc D1CPRE
	uint16_t d = hwSysClock::CC.SYS_clk / 480'000'000;
	if ((hwSysClock::CC.SYS_clk % 480'000'000) > 0) d++;
	if (d <= 1)
	{
		ModifyReg(RCC->D1CFGR, RCC_D1CFGR_D1CPRE_Msk, RCC_D1CFGR_D1CPRE_DIV1);
		C1_clk = hwSysClock::CC.SYS_clk / 1;
	}
	else if (d <= 2)
	{
		ModifyReg(RCC->D1CFGR, RCC_D1CFGR_D1CPRE_Msk, RCC_D1CFGR_D1CPRE_DIV2);
		C1_clk = hwSysClock::CC.SYS_clk / 2;
	}
	else if (d <= 4)
	{
		ModifyReg(RCC->D1CFGR, RCC_D1CFGR_D1CPRE_Msk, RCC_D1CFGR_D1CPRE_DIV4);
		C1_clk = hwSysClock::CC.SYS_clk / 4;
	}
	else if (d <= 8)
	{
		ModifyReg(RCC->D1CFGR, RCC_D1CFGR_D1CPRE_Msk, RCC_D1CFGR_D1CPRE_DIV8);
		C1_clk = hwSysClock::CC.SYS_clk / 8;
	}
	else if (d <= 16)
	{
		ModifyReg(RCC->D1CFGR, RCC_D1CFGR_D1CPRE_Msk, RCC_D1CFGR_D1CPRE_DIV16);
		C1_clk = hwSysClock::CC.SYS_clk / 16;
	}
	else if (d <= 64)
	{
		ModifyReg(RCC->D1CFGR, RCC_D1CFGR_D1CPRE_Msk, RCC_D1CFGR_D1CPRE_DIV64);
		C1_clk = hwSysClock::CC.SYS_clk / 64;
	}
	else if (d <= 128)
	{
		ModifyReg(RCC->D1CFGR, RCC_D1CFGR_D1CPRE_Msk, RCC_D1CFGR_D1CPRE_DIV128);
		C1_clk = hwSysClock::CC.SYS_clk / 128;
	}
	else if (d <= 256)
	{
		ModifyReg(RCC->D1CFGR, RCC_D1CFGR_D1CPRE_Msk, RCC_D1CFGR_D1CPRE_DIV256);
		C1_clk = hwSysClock::CC.SYS_clk / 256;
	}
	else if (d <= 512)
	{
		ModifyReg(RCC->D1CFGR, RCC_D1CFGR_D1CPRE_Msk, RCC_D1CFGR_D1CPRE_DIV512);
		C1_clk = hwSysClock::CC.SYS_clk / 512;
	}
	else
		assert(false);
	assert(C1_clk <= 480'000'000 && "C1 too big");
}

void hwCC::hwSCGU::C2Clk(uint32_t freq)
{
	if (freq > 240'000'000)
		freq = 240'000'000;

	// calc HPRE
	uint16_t d = C1_clk / 240'000'000; 
	if ((C1_clk % 240'000'000) > 0) d++;
/*
//	Printf("3 1\n");
	// adjust wait states
	uint32_t ACR = FLASH->ACR;
//	Printf("3 1 1\n");
	ModifyReg(ACR, FLASH_ACR_LATENCY_Msk, FLASH_ACR_LATENCY_7WS);
	ModifyReg(ACR, FLASH_ACR_WRHIGHFREQ_Msk, 0B11 << FLASH_ACR_WRHIGHFREQ_Pos);
//	Printf("3 1 2\n");
	FLASH->ACR = ACR;
//	Printf("3 1 3\n");
	while (FLASH->ACR != ACR) __asm(""); 
//	Printf("3 2\n");
*/	
	if (d <= 1)
	{
		ModifyReg(RCC->D1CFGR, RCC_D1CFGR_HPRE_Msk, RCC_D1CFGR_HPRE_DIV1);
		C2_clk = C1_clk / 1;
	}
	else if (d <= 2)
	{
		ModifyReg(RCC->D1CFGR, RCC_D1CFGR_HPRE_Msk, RCC_D1CFGR_HPRE_DIV2);
		C2_clk = C1_clk / 2;
	}
	else if (d <= 4)
	{
		ModifyReg(RCC->D1CFGR, RCC_D1CFGR_HPRE_Msk, RCC_D1CFGR_HPRE_DIV4);
		C2_clk = C1_clk / 4;
	}
	else if (d <= 8)
	{
		ModifyReg(RCC->D1CFGR, RCC_D1CFGR_HPRE_Msk, RCC_D1CFGR_HPRE_DIV8);
		C2_clk = C1_clk / 8;
	}
	else if (d <= 16)
	{
		ModifyReg(RCC->D1CFGR, RCC_D1CFGR_HPRE_Msk, RCC_D1CFGR_HPRE_DIV16);
		C2_clk = C1_clk / 16;
	}
	else if (d <= 64)
	{
		ModifyReg(RCC->D1CFGR, RCC_D1CFGR_HPRE_Msk, RCC_D1CFGR_HPRE_DIV64);
		C2_clk = C1_clk / 64;
	}
	else if (d <= 128)
	{
		ModifyReg(RCC->D1CFGR, RCC_D1CFGR_HPRE_Msk, RCC_D1CFGR_HPRE_DIV128);
		C2_clk = C1_clk / 128;
	}
	else if (d <= 256)
	{
		ModifyReg(RCC->D1CFGR, RCC_D1CFGR_HPRE_Msk, RCC_D1CFGR_HPRE_DIV256);
		C2_clk = C1_clk / 256;
	}
	else if (d <= 512)
	{
		ModifyReg(RCC->D1CFGR, RCC_D1CFGR_HPRE_Msk, RCC_D1CFGR_HPRE_DIV512);
		C2_clk = C1_clk / 512;
	}
	else
		assert(false);
	assert(C2_clk <= 240'000'000 && "C2 too big");
/*
//	Printf("3 3\n");
	// adjust wait states
	ACR = FLASH->ACR;
	ModifyReg(ACR, FLASH_ACR_LATENCY_Msk, FLASH_ACR_LATENCY_7WS);
	ModifyReg(ACR, FLASH_ACR_WRHIGHFREQ_Msk, 0B11 << FLASH_ACR_WRHIGHFREQ_Pos);
	FLASH->ACR = ACR;
	while (FLASH->ACR != ACR) __asm(""); 
//	Printf("3 4\n");
*/
}

void hwCC::hwSCGU::P1Clk(ePDivisor p3Divisor, bool setTimerBit)
{
	ModifyReg(RCC->CFGR, RCC_CFGR_TIMPRE_Msk, setTimerBit ? RCC_CFGR_TIMPRE : 0);
	switch (p3Divisor)
	{
	case ePDivisor::P_1:
		ModifyReg(RCC->D2CFGR, RCC_D2CFGR_D2PPRE1_Msk, RCC_D2CFGR_D2PPRE1_DIV1);
		P1T_clk = P1_clk = C2_clk;
		break;
	case ePDivisor::P_2:
		ModifyReg(RCC->D2CFGR, RCC_D2CFGR_D2PPRE1_Msk, RCC_D2CFGR_D2PPRE1_DIV2);
		P1_clk = C2_clk / 2;
		P1T_clk = C2_clk;
		break;
	case ePDivisor::P_4:
		ModifyReg(RCC->D2CFGR, RCC_D2CFGR_D2PPRE1_Msk, RCC_D2CFGR_D2PPRE1_DIV4);
		P1_clk = C2_clk / 4;
		P1T_clk = (RCC->CFGR & RCC_CFGR_TIMPRE) != 0 ? C2_clk : C2_clk / 2;
		break;
	case ePDivisor::P_8:
		ModifyReg(RCC->D2CFGR, RCC_D2CFGR_D2PPRE1_Msk, RCC_D2CFGR_D2PPRE1_DIV8);
		P1_clk = C2_clk / 8;
		P1T_clk = (RCC->CFGR & RCC_CFGR_TIMPRE) != 0 ? C2_clk / 2 : C2_clk / 4;
		break;
	case ePDivisor::P_16:
		ModifyReg(RCC->D2CFGR, RCC_D2CFGR_D2PPRE1_Msk, RCC_D2CFGR_D2PPRE1_DIV16);
		P1_clk = C2_clk / 16;
		P1T_clk = (RCC->CFGR & RCC_CFGR_TIMPRE) != 0 ? C2_clk / 4 : C2_clk / 8;
		break;
	}
}

void hwCC::hwSCGU::P2Clk(ePDivisor p3Divisor, bool HRTIMSEL)
{
	ModifyReg(RCC->CFGR, RCC_CFGR_HRTIMSEL_Msk, HRTIMSEL ? RCC_CFGR_HRTIMSEL : 0);
	switch (p3Divisor)
	{
	case ePDivisor::P_1:
		ModifyReg(RCC->D2CFGR, RCC_D2CFGR_D2PPRE2_Msk, RCC_D2CFGR_D2PPRE2_DIV1);
		P2T_clk = P2_clk = C2_clk;
		break;
	case ePDivisor::P_2:
		ModifyReg(RCC->D2CFGR, RCC_D2CFGR_D2PPRE2_Msk, RCC_D2CFGR_D2PPRE2_DIV2);
		P2_clk = C2_clk / 2;
		P2T_clk = C2_clk;
		break;
	case ePDivisor::P_4:
		ModifyReg(RCC->D2CFGR, RCC_D2CFGR_D2PPRE2_Msk, RCC_D2CFGR_D2PPRE2_DIV4);
		P2_clk = C2_clk / 4;
		P2T_clk = (RCC->CFGR & RCC_CFGR_TIMPRE) != 0 ? C2_clk : C2_clk / 2;
		break;
	case ePDivisor::P_8:
		ModifyReg(RCC->D2CFGR, RCC_D2CFGR_D2PPRE2_Msk, RCC_D2CFGR_D2PPRE2_DIV8);
		P2_clk = C2_clk / 8;
		P2T_clk = (RCC->CFGR & RCC_CFGR_TIMPRE) != 0 ? C2_clk / 2 : C2_clk / 4;
		break;
	case ePDivisor::P_16:
		ModifyReg(RCC->D2CFGR, RCC_D2CFGR_D2PPRE2_Msk, RCC_D2CFGR_D2PPRE2_DIV16);
		P2_clk = C2_clk / 16;
		P2T_clk = (RCC->CFGR & RCC_CFGR_TIMPRE) != 0 ? C2_clk / 4 : C2_clk / 8;
		break;
	}
	P2HRT_clk = (RCC->CFGR & RCC_CFGR_HRTIMSEL_Msk) != 0 ? C1_clk : P2T_clk;
}


void hwCC::hwSCGU::P3Clk(ePDivisor p3Divisor)
{

	switch (p3Divisor)
	{
	case ePDivisor::P_1:
		ModifyReg(RCC->D1CFGR, RCC_D1CFGR_D1PPRE_Msk, RCC_D1CFGR_D1PPRE_DIV1);
		P3_clk = C2_clk;
		break;
	case ePDivisor::P_2:
		ModifyReg(RCC->D1CFGR, RCC_D1CFGR_D1PPRE_Msk, RCC_D1CFGR_D1PPRE_DIV2);
		P3_clk = C2_clk / 2;
		break;
	case ePDivisor::P_4:
		ModifyReg(RCC->D1CFGR, RCC_D1CFGR_D1PPRE_Msk, RCC_D1CFGR_D1PPRE_DIV4);
		P3_clk = C2_clk / 4;
		break;
	case ePDivisor::P_8:
		ModifyReg(RCC->D1CFGR, RCC_D1CFGR_D1PPRE_Msk, RCC_D1CFGR_D1PPRE_DIV8);
		P3_clk = C2_clk / 8;
		break;
	case ePDivisor::P_16:
		ModifyReg(RCC->D1CFGR, RCC_D1CFGR_D1PPRE_Msk, RCC_D1CFGR_D1PPRE_DIV16);
		P3_clk = C2_clk / 16;
		break;
	}
}
void hwCC::hwSCGU::P4Clk(ePDivisor p3Divisor)
{
	switch (p3Divisor)
	{
	case ePDivisor::P_1:
		ModifyReg(RCC->D3CFGR, RCC_D3CFGR_D3PPRE_Msk, RCC_D3CFGR_D3PPRE_DIV1);
		P4_clk = C2_clk;
		break;
	case ePDivisor::P_2:
		ModifyReg(RCC->D3CFGR, RCC_D3CFGR_D3PPRE_Msk, RCC_D3CFGR_D3PPRE_DIV2);
		P4_clk = C2_clk / 2;
		break;
	case ePDivisor::P_4:
		ModifyReg(RCC->D3CFGR, RCC_D3CFGR_D3PPRE_Msk, RCC_D3CFGR_D3PPRE_DIV4);
		P4_clk = C2_clk / 4;
		break;
	case ePDivisor::P_8:
		ModifyReg(RCC->D3CFGR, RCC_D3CFGR_D3PPRE_Msk, RCC_D3CFGR_D3PPRE_DIV8);
		P4_clk = C2_clk / 8;
		break;
	case ePDivisor::P_16:
		ModifyReg(RCC->D3CFGR, RCC_D3CFGR_D3PPRE_Msk, RCC_D3CFGR_D3PPRE_DIV16);
		P4_clk = C2_clk / 16;
		break;
	}
}

////////////////////////////////////////////////////////////////////////////////////
void hwSysClock::Reset()
{
	/* Set HSION bit */
	RCC->CR |= RCC_CR_HSION;

	/* Reset CFGR register */
	RCC->CFGR = 0x00000000;

	/* Reset HSEON, HSECSSON, CSION, HSI48ON, CSIKERON, PLL1ON, PLL2ON and PLL3ON bits */
	RCC->CR &= 0xEAF6ED7FU;

	/* Decreasing the number of wait states because of lower CPU frequency */
	if (FLASH_LATENCY_DEFAULT  < (READ_BIT((FLASH->ACR), FLASH_ACR_LATENCY)))
	{
		/* Program the new number of wait states to the LATENCY bits in the FLASH_ACR register */
		MODIFY_REG(FLASH->ACR, FLASH_ACR_LATENCY, (uint32_t)(FLASH_LATENCY_DEFAULT));
	}

	/* Reset D1CFGR register */
	RCC->D1CFGR = 0x00000000;

	/* Reset D2CFGR register */
	RCC->D2CFGR = 0x00000000;

	/* Reset D3CFGR register */
	RCC->D3CFGR = 0x00000000;

	/* Reset PLLCKSELR register */
	RCC->PLLCKSELR = 0x02020200;

	/* Reset PLLCFGR register */
	RCC->PLLCFGR = 0x01FF0000;
	/* Reset PLL1DIVR register */
	RCC->PLL1DIVR = 0x01010280;
	/* Reset PLL1FRACR register */
	RCC->PLL1FRACR = 0x00000000;

	/* Reset PLL2DIVR register */
	RCC->PLL2DIVR = 0x01010280;

	/* Reset PLL2FRACR register */

	RCC->PLL2FRACR = 0x00000000;
	/* Reset PLL3DIVR register */
	RCC->PLL3DIVR = 0x01010280;

	/* Reset PLL3FRACR register */
	RCC->PLL3FRACR = 0x00000000;

	/* Reset HSEBYP bit */
	RCC->CR &= 0xFFFBFFFFU;

	/* Disable all interrupts */
	RCC->CIER = 0x00000000;
	}
////////////////////////////////////////////////////////////////////////////////////
#define C1_CLK 480
//#define C1_CLK 400
//#define C1_CLK 300
//#define C1_CLK 200
void hwSysClock::Setup()
{
	// make sure MPUs run
	CC.HSI(hwCC::eHSI::Mhz64); // on 
	CC.SYSClkSource(hwCC::eSysClkSource::HSI);
	
	Printf("Selecting HSE\n");
	
	CC.HSE(hwCC::eHSE::External, HSE_VALUE); // 16MHz
	CC.PLLSource(hwCC::ePLL_SRC::HSE); // 16MHz
	CC.PLL1.PllInClk(4'000'000); // 16/4 = 4MHz source for PLL1
#if C1_CLK == 480
	Printf("Selecting 480 MHz\n");
	hwPWR::VOS(hwPWR::eVOSs::VOS0);
	CC.PLL1.SetupInteger(240, 2, 24, 2); // (4*240)/2 = 480 MHz
#endif
#if C1_CLK == 400
	Printf("Selecting 400 MHz\n");
	hwPWR::VOS(hwPWR::eVOSs::VOS1);
	CC.PLL1.SetupInteger(200, 2, 24, 2); // (4*200)/2 = 400 MHz
#endif
#if C1_CLK == 300
	Printf("Selecting 300 MHz\n");
	hwPWR::VOS(hwPWR::eVOSs::VOS2);
	CC.PLL1.SetupInteger(150, 2, 24, 2); // (4*150)/2 = 300 MHz
#endif
#if C1_CLK == 200
	Printf("Selecting 200 MHz\n");
	hwPWR::VOS(hwPWR::eVOSs::VOS3);
	CC.PLL1.SetupInteger(100, 2, 24, 2); // (4*100)/2 = 200 MHz
#endif
	CC.PLL1.Enable();
	
	// LTDC clk
	CC.PLL3.DivM(12); // 16/12 = 1.333MHz source for PLL3
	CC.PLL3.SetupInteger(246, 6, 6, 6); // (1.333*231)/6 = 51.333 MHz
	// (1.333*246)/6 = 54,6653 MHz
	CC.PLL3.Enable();

	// FMC clk
	CC.PLL2.DivM(4); // 16/4 = 4MHz source for PLL2
	CC.PLL2.SetupInteger(180, 4, 11, 3); // (4*180)/3 = 240 MHz
	CC.PLL2.Enable();

	CC.SYSClkSource(hwCC::eSysClkSource::PLL1); // pll P_clk;
	CC.SCGU.P1Clk(hwCC::hwSCGU::ePDivisor::P_2, false);
	CC.SCGU.P2Clk(hwCC::hwSCGU::ePDivisor::P_2, false);
	CC.SCGU.P3Clk(hwCC::hwSCGU::ePDivisor::P_2);
	CC.SCGU.P4Clk(hwCC::hwSCGU::ePDivisor::P_2);
	
	CC.HSI(hwCC::eHSI::Off); // off 
	CC.CSI(hwCC::eHSE::Off); // off 
}

uint32_t hwSysClock::SysTickClk()
{
	if (CC.SYS_clk == 0)
		SystemCoreClockUpdate(); // to find core clock
#ifdef CORE_CM7
	return CC.SCGU.C1_clk / 8;
#endif
#ifdef CORE_CM4
	return CC.SCGU.C2_clk / 8;
#endif
}

uint32_t hwSysClock::C1Clk()
{
	if (CC.SYS_clk == 0)
		SystemCoreClockUpdate(); // to find core clock
	return CC.SCGU.C1_clk;
}
uint32_t hwSysClock::C2Clk()
{
	if (CC.SYS_clk == 0)
		SystemCoreClockUpdate(); // to find core clock
	return CC.SCGU.C2_clk;
}
uint32_t hwSysClock::CPUClk()
{
	if (CC.SYS_clk == 0)
		SystemCoreClockUpdate(); // to find core clock
#ifdef CORE_CM7
	return CC.SCGU.C1_clk;
#endif
#ifdef CORE_CM4
	return CC.SCGU.C2_clk;
#endif
}

uint32_t hwSysClock::AXIClk()
{
	if (CC.SYS_clk == 0)
		SystemCoreClockUpdate(); // to find core clock
	return CC.SCGU.C2_clk;
}

uint32_t hwSysClock::AHB3Clk()
{
	if (CC.SYS_clk == 0)
		SystemCoreClockUpdate(); // to find core clock
	return CC.SCGU.C2_clk;
}

uint32_t hwSysClock::APB1Clk()
{
	if (CC.SYS_clk == 0)
		SystemCoreClockUpdate(); // to find core clock
	return CC.SCGU.P1_clk;
}

uint32_t hwSysClock::APB1TClk()
{
	if (CC.SYS_clk == 0)
		SystemCoreClockUpdate(); // to find core clock
	return CC.SCGU.P1T_clk;
}

uint32_t hwSysClock::APB2Clk()
{
	if (CC.SYS_clk == 0)
		SystemCoreClockUpdate(); // to find core clock
	return CC.SCGU.P2_clk;
}

uint32_t hwSysClock::APB2TClk()
{
	if (CC.SYS_clk == 0)
		SystemCoreClockUpdate(); // to find core clock
	return CC.SCGU.P2T_clk;
}

uint32_t hwSysClock::APB3Clk()
{
	if (CC.SYS_clk == 0)
		SystemCoreClockUpdate(); // to find core clock
	return CC.SCGU.P3_clk;
}

uint32_t hwSysClock::APB4Clk()
{
	if (CC.SYS_clk == 0)
		SystemCoreClockUpdate(); // to find core clock
	return CC.SCGU.P4_clk;
}