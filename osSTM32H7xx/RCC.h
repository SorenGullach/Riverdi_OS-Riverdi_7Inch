#pragma once

#include <stm32h7xx.h>
#include <Utils.h>
#include <assert.h>
#include <stdio.h>
#include <Registers.h>

#if !defined  (HSI_VALUE)
#define HSI_VALUE    64'000'000UL /*!< Value of the Internal oscillator in Hz*/
#endif /* HSI_VALUE */

#if !defined  (HSE_VALUE)
#define HSE_VALUE    16'000'000UL /*!< Value of the External oscillator in Hz */
#endif /* HSE_VALUE */

#if !defined  (LSE_VALUE)
#define LSE_VALUE    32'000UL /*!< Value of the Internal oscillator in Hz*/
#endif /* LSE_VALUE */

#if !defined  (LSI_VALUE)
#define LSI_VALUE    32'000UL /*!< Value of the Internal oscillator in Hz*/
#endif /* LSI_VALUE */

#if !defined  (CSI_VALUE)
#define CSI_VALUE    4'000'000UL /*!< Value of the Internal oscillator in Hz*/
#endif /* CSI_VALUE */

// forward declear
class hwSysClock;

class hwCC
{
	// forward declear
	template <int Idx>
		class PLL;
	class hwSCGU;

	// Friends
	friend hwSysClock;
	friend PLL<1>;
	friend PLL<2>;
	friend PLL<3>;
	friend hwSCGU;
	
	// Dumps the current settings
	void DumpClocks();
	// Find the clocks
	void SystemCoreClockUpdate();

	uint32_t SYS_clk, LSI_clk, LSE_clk, RTC_clk, HSE_1M_clk, HSE_clk, HSI_clk, CSI_clk, HSI48_clk, PER_clk;
	
#pragma region MCO_Outputs
	// As shown in the Figure 51, the RCC offers 2 clock outputs(MCO1 and MCO2), with a great
	// flexibility on the clock selection and frequency adjustment.
	enum class eMCOSEL
	{
		sys_ck,
		pll2_p_ck,
		hse_ck,
		pll1_p_ck,
		csi_ck,
		lsi_ck
	};
	/*
		static CGPIO* MCO(R_t<uint8_t, 1, 2>MCO_Pin, eMCOSEL MCOSEL, R_t<uint8_t, 0, 15> div)
		{
			static CGPIO MCOPin;
			if (MCO_Pin == 1)
			{
				MCOPin.Init(GPIOA, 8, GPIO_AF0_MCO, CGPIO::ePP::PushPull, CGPIO::ePUPD::None);
				ModifyReg(RCC->CFGR,
					RCC_CFGR_MCO1_Msk | RCC_CFGR_MCO1PRE_Msk, // clear
					((uint32_t)MCOSEL << RCC_CFGR_MCO1_Pos) | ((uint32_t)div << RCC_CFGR_MCO1PRE_Pos)); // Set
			}
			else
			{
				MCOPin.Init(GPIOC, 9, GPIO_AF0_MCO, CGPIO::ePP::PushPull, CGPIO::ePUPD::None);
				ModifyReg(RCC->CFGR,
					RCC_CFGR_MCO2_Msk | RCC_CFGR_MCO2PRE_Msk, // clear
					((uint32_t)MCOSEL << RCC_CFGR_MCO2_Pos) | ((uint32_t)div << RCC_CFGR_MCO2PRE_Pos)); // Set
			}
			return &MCOPin;
		}
		*/
#pragma endregion

#pragma region RCC_clock_block
	enum class eHSE
	{
		Off,
		Bypass,
		External
	};
	/*
	* The HSE block can generate a clock from two possible sources:
	• External crystal/ceramic resonator
	• External clock source
	*/
	void HSE(hwCC::eHSE hse, uint32_t clk = HSE_VALUE);

	/*
	* The LSE block can generate a clock from two possible sources:
	• External crystal/ceramic resonator
	• External user clock
	*/
	void LSE(eHSE hse, uint32_t clk = LSE_VALUE);

	enum class eHSI
	{
		Off,
		Mhz8,
		Mhz16,
		Mhz32,
		Mhz64
	};
	/*
	The HSI is a high-speed internal RC oscillator which can be used directly as system clock,
	peripheral clock, or as PLL input. A predivider allows the application to select an HSI output
	frequency of 8, 16, 32 or 64 MHz. This predivider is controlled by the HSIDIV.
	*/
	void HSI(eHSI hsi);

	/*
	The CSI is a low-power RC oscillator which can be used directly as system clock, peripheral
	clock, or PLL input.
	*/
	void CSI(eHSE hse);

	/*
	The HSI48 is an RC oscillator that delivers a 48 MHz clock that can be used directly as
	kernel clock for some peripherals
	*/
	void HSI48(bool on);

	/*
	The LSI acts as a low-power clock source that can be kept running when the system is in
	Stop or Standby mode for the independent watchdog (IWDG) and Auto-Wakeup Unit
	(AWU). The clock frequency is around 32 kHz.
	*/
	void LSI(bool on);

#pragma endregion

	enum class ePLL_SRC
	{
		None,
		HSI,
		CSI,
		HSE
	};

	void PLLSource(ePLL_SRC src);

	/*
	The RCC features three PLLs :
	• A main PLL, PLL1, which is generally used to provide clocks to the CPUs and to some peripherals.
	• Two dedicated PLLs, PLL2 and PLL3, which are used to generate the kernel clock for peripherals.
	*/
	template <int Idx>
		class PLL
		{
			friend hwSysClock;
			friend hwCC;
			
			// Dumps the current settings
			void DumpClocks();
			// Find the clocks
			void ClockUpdate();

			bool IsEnabled()
			{
				uint32_t on = (((Idx == 1) * RCC_CR_PLL1ON) | ((Idx == 2) * RCC_CR_PLL2ON) | ((Idx == 3) * RCC_CR_PLL3ON));
				return (RCC->CR & on) != 0;
			}
			void Enable()
			{
				uint32_t on = (((Idx == 1) * RCC_CR_PLL1ON) | ((Idx == 2) * RCC_CR_PLL2ON) | ((Idx == 3) * RCC_CR_PLL3ON));
				RCC->CR |= on; // Enable PLL
				uint32_t ready = (((Idx == 1) * RCC_CR_PLL1RDY) | ((Idx == 2) * RCC_CR_PLL2RDY) | ((Idx == 3) * RCC_CR_PLL3RDY));
				while ((RCC->CR & ready) == 0) ;
			}

			/*
			The frequency of the reference clock provided to the PLLs (refx_ck) must range from 1 to
			16 MHz. The user application has to program properly the DIVMx dividers of the RCC PLLs
			*/
			uint32_t  PllInClk(R_t<uint32_t, 1'000'000, 16'000'000> clk);

			void DivM(R_t<uint8_t, 0, 63> divMx);

			void SetupInteger(R_t<uint16_t, 4, 512> mulNx, R_t<uint8_t, 1, 128> divPx, R_t<uint8_t, 1, 128> divQx, R_t<uint8_t, 1, 128> divRx);
			
			uint32_t P_clk, Q_clk, R_clk;
			uint32_t Src_clk;
			uint32_t Ref_clk, VCO_clk;
		};
	
	static PLL <1>PLL1;
	static PLL <2>PLL2;
	static PLL <3>PLL3;

	enum class eSysClkSource
	{
		HSI,
		CSI,
		HSE,
		PLL1
	};
	void SYSClkSource(eSysClkSource src);

	// The SCGU block(System Clock Generation Unit) contains several prescalers used to
		// configure the CPU and bus matrix clock frequencies.
	class hwSCGU
	{
		friend hwSysClock;
		friend hwCC;
		
		// Dumps the current settings
		void DumpClocks();
		
		// Find the clocks
		void ClockUpdate();

		void C1Clk(uint32_t freq = 480'000'000);
		void C2Clk(uint32_t freq = 240'000'000);
		enum class ePDivisor
		{
			P_1,
			P_2,
			P_4,
			P_8,
			P_16,
		};
		void P1Clk(ePDivisor pDivisor = ePDivisor::P_1);
		void P2Clk(ePDivisor pDivisor = ePDivisor::P_1);
		void P3Clk(ePDivisor pDivisor = ePDivisor::P_1);
		void P4Clk(ePDivisor pDivisor = ePDivisor::P_1);
		
		uint32_t C1_clk, C2_clk, P3_clk, P1_clk, P2_clk, P4_clk;
	};
	
	static hwSCGU SCGU;

};

class hwSysClock
{
public:
	// Dumps the current settings
	static void DumpClocks()
	{
		Printf("UniTest of SysClock start\n");
		CC.DumpClocks();
		Printf("UniTest of SysClock end\n");
	}
	// Find the clocks
	static void SystemCoreClockUpdate()
	{
		CC.SystemCoreClockUpdate();
	}

	// Setup clocks
	static void Setup();
	
	// Returns the SYSTickClk
	static uint32_t SysTickClk();
	// Returns the C1Clk
	static uint32_t C1Clk();
	// Returns the C2Clk
	static uint32_t C2Clk();
	// Returns the CPUClk
	static uint32_t CPUClk();
	
	// Returns the AXIClk
	static uint32_t AXIClk();
	// Returns the AHB3Clk
	static uint32_t AHB3Clk();
	// Returns the APB1Clk
	static uint32_t APB1Clk();
	// Returns the APB2Clk
	static uint32_t APB2Clk();
	// Returns the APB3Clk
	static uint32_t APB3Clk();
	// Returns the APB4Clk
	static uint32_t APB4Clk();
	
	static hwCC CC; 
};

