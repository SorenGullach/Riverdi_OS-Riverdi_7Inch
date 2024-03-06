#pragma once

#include "stm32h7xx.h"
#include <assert.h>
#include <stdio.h>

// https://www.st.com/content/ccc/resource/training/technical/product_training/group0/97/f1/9d/3a/c4/c2/4c/ff/STM32H7-System-Power_control_PWR/files/STM32H7-System-Power_control_PWR.pdf/_jcr_content/translations/en.STM32H7-System-Power_control_PWR.pdf

// The Riverdi PCB uses mode 4 (or 3 with modifications)
class hwPWR
{
public:
	enum class eSupplyConfigurations
	{
		/*
		* | ID | Supply Configuration											| SDLEVEL | SDEXTHP | SDEN | LDOEN | BYPASS | Description                                                                                                          |
		* |----|---------------------------------								|---------|---------|------|-------|--------|----------------------------------------------------------------------------------------------------------------------|
		* | 0  | Default configuration											| 00      | 0       | 1    | 1     | 0      | VCORE Power Domains are supplied from the LDO according to VOS. SMPS step-down converter enabled at 1.2V, may be used to supply the LDO.                                      |
		* | 1  | LDO supply														| x       | x       | 0    | 1     | 0      | VCORE Power Domains are supplied from the LDO according to VOS. LDO power mode (Main, LP, Off) will follow system low-power modes. SMPS step-down converter disabled.             |
		* | 2  | Direct SMPS stepdown converter									| x       | 0		| 1    | 0     | 0      | VCORE Power Domains are supplied from SMPS step-down converter according to VOS. LDO bypassed. SMPS step-down converter power mode (MR, LP, Off) will follow system low-power modes. |
		* | 3  | SMPS step-down converter supplies LDO							| 01 or 10| 0		| 1    | 1     | 0      | VCORE Power Domains are supplied from the LDO according to VOS. LDO power mode (Main, LP, Off) will follow system low-power modes. SMPS step-down converter enabled according to SDLEVEL, and supplies the LDO. SMPS step-down converter power mode (MR, LP, Off) will follow system low-power modes. |
		* | 4  | SMPS step-down converter supplies External and LDO				| 01 or 10| 1		| 1    | 1     | 0      | VCORE Power Domains are supplied from voltage regulator according to VOS. LDO power mode (Main, LP, Off) will follow system low-power modes. SMPS step-down converter enabled according to SDLEVEL used to supply external circuits and may supply the LDO. SMPS step-down converter forced ON in MR mode. |
		* | 5  | SMPS step-down converter supplies external and LDO Bypass		| 01 or 10| 1		| 1    | 0     | 1      | VCORE supplied from external source. SMPS step-down converter enabled according to SDLEVEL used to supply external circuits and may supply the external source for VCORE. SMPS step-down converter forced ON in MR mode.        |
		* | 6  | SMPS step-down converter disabled and LDO Bypass				| x       | x       | 0    | 0     | 1      | VCORE supplied from external source. SMPS step-down converter disabled and LDO bypassed,
		 */
		C0,
		C1,
		C2,
		C3,
		C4, 
		C5,
		C6
	};
	static void SupplyConfiguration(
#ifdef CORE_CM7
		eSupplyConfigurations config
#endif
		)
	{
#ifdef CORE_CM7
		uint32_t CR3 = PWR->CR3 & ~(PWR_CR3_SMPSLEVEL_Msk | PWR_CR3_SMPSEXTHP_Msk | PWR_CR3_SMPSEN_Msk | PWR_CR3_LDOEN_Msk | PWR_CR3_BYPASS_Msk);
		switch (config)
		{
		case hwPWR::eSupplyConfigurations::C0:
			PWR->CR3 = CR3 | PWR_CR3_SMPSEN | PWR_CR3_LDOEN;
			break;
		case hwPWR::eSupplyConfigurations::C1:
			PWR->CR3 = CR3 | PWR_CR3_LDOEN;
			break;
		case hwPWR::eSupplyConfigurations::C2:
			PWR->CR3 = CR3 | PWR_CR3_SMPSEN;
			break;
		case hwPWR::eSupplyConfigurations::C3:
			PWR->CR3 = CR3 | PWR_CR3_SMPSEN | PWR_CR3_LDOEN;
			break;
		case hwPWR::eSupplyConfigurations::C4:
			PWR->CR3 = CR3 | PWR_CR3_SMPSEXTHP | PWR_CR3_SMPSEN | PWR_CR3_LDOEN;
			break;
		case hwPWR::eSupplyConfigurations::C5:
			PWR->CR3 = CR3 | PWR_CR3_SMPSEXTHP | PWR_CR3_SMPSEN | PWR_CR3_BYPASS;
			break;
		case hwPWR::eSupplyConfigurations::C6:
			PWR->CR3 = CR3 | PWR_CR3_BYPASS;
			break;
		}
		//		assert(PWR->CSR1 & PWR_CSR1_ACTVOSRDY);
#endif

#ifdef CORE_CM7
		DBGMCU->CR |= DBGMCU_CR_DBG_STANDBYD1 | DBGMCU_CR_DBG_STOPD1 | DBGMCU_CR_DBG_SLEEPD1;
#endif
#ifdef CORE_CM4
		DBGMCU->CR |= DBGMCU_CR_DBG_STANDBYD2 | DBGMCU_CR_DBG_STOPD2 | DBGMCU_CR_DBG_SLEEPD2;
#endif
	}

	enum class eAnalogVoltageDetectorLevel	{ V1_7 = 0x00, V2_1 = 0x01, V2_5 = 0x10, V2_8 = 0x11 };
	bool  hwAnalogVoltageDetectorLevelSelection(eAnalogVoltageDetectorLevel level)
	{ 
		PWR->CR1 &= PWR_CR1_ALS_Msk;
		PWR->CR1 |= (int)level << PWR_CR1_ALS_Pos;
		return true;
	}
		
	enum class eVOSs
	{
		/*
		 * Voltage regulator output can be adjusted according to application needs through 6 power
		 * supply levels:
		 *
		 * Run mode (VOS0 to VOS3)
		 * - Scale 0: boosted performance (available only with LDO regulator)
		 * - Scale 1: high performance
		 * - Scale 2: medium performance and consumption
		 * - Scale 3: optimized performance and low-power consumption
		 *
		 * Stop mode (SVOS3 to SVOS5)
		 * - Scale 3: peripheral with wakeup from Stop mode capabilities (UART, SPI, I2C, LPTIM) are operational
		 * - Scale 4 and 5 where the peripheral with wakeup from Stop mode is disabled
		 *   The peripheral functionality is disabled but wakeup from Stop mode is possible
		 *   through GPIO or asynchronous interrupt.
		 */
		VOS0,
		VOS1,
		VOS2,
		VOS3,
		LP_SVOS3,
		LP_SVOS4,
		LP_SVOS5,
	};
	static bool VOS(eVOSs vos)
	{
		if (vos != eVOSs::VOS0 && (SYSCFG->PWRCR & SYSCFG_PWRCR_ODEN_Msk)) // from VOS0
		{
			// freq must be reduced by now
			RCC->APB4ENR |= RCC_APB4ENR_SYSCFGEN; // enable SYSCFG
			SYSCFG->PWRCR &= ~SYSCFG_PWRCR_ODEN;
		}
		switch (vos) 
		{
		case eVOSs::VOS0:
			if ((PWR->D3CR & PWR_D3CR_VOS_Msk) != (0x3 << PWR_D3CR_VOS_Pos)) // if not in VOS1
				VOS(eVOSs::VOS1);
			if ((PWR->D3CR & PWR_D3CR_VOS_Msk) == (0x3 << PWR_D3CR_VOS_Pos)) // if in VOS1
			{
				RCC->APB4ENR |= RCC_APB4ENR_SYSCFGEN; // enable SYSCFG
				SYSCFG->PWRCR |= SYSCFG_PWRCR_ODEN;
				while (!(PWR->D3CR & PWR_D3CR_VOSRDY)) __asm("");
			}
			break;
		case eVOSs::VOS1:
			ModifyReg(PWR->D3CR, PWR_D3CR_VOS_Msk, 0x3 << PWR_D3CR_VOS_Pos);
			while (!(PWR->D3CR & PWR_D3CR_VOSRDY)) __asm("");
			break;
		case eVOSs::VOS2:
			ModifyReg(PWR->D3CR, PWR_D3CR_VOS_Msk, 0x2 << PWR_D3CR_VOS_Pos);
			while (!(PWR->D3CR & PWR_D3CR_VOSRDY)) __asm("");
			break;
		case eVOSs::VOS3:
			ModifyReg(PWR->D3CR, PWR_D3CR_VOS_Msk, 0x1 << PWR_D3CR_VOS_Pos);
			while (!(PWR->D3CR & PWR_D3CR_VOSRDY)) __asm("");
			ModifyReg(PWR->CR1,PWR_CR1_SVOS_Msk, 0x3 << PWR_CR1_SVOS_Pos);
			PWR->CR1 &= ~PWR_CR1_LPDS;
			break;
		case eVOSs::LP_SVOS3:
			ModifyReg(PWR->D3CR, PWR_D3CR_VOS_Msk, 0x1 << PWR_D3CR_VOS_Pos);
			while (!(PWR->D3CR & PWR_D3CR_VOSRDY)) __asm("");
			ModifyReg(PWR->CR1, PWR_CR1_SVOS_Msk, 0x3 << PWR_CR1_SVOS_Pos);
			PWR->CR1 |= PWR_CR1_LPDS;
			break;
		case eVOSs::LP_SVOS4:
			ModifyReg(PWR->D3CR, PWR_D3CR_VOS_Msk, 0x1 << PWR_D3CR_VOS_Pos);
			while (!(PWR->D3CR & PWR_D3CR_VOSRDY)) __asm("");
			PWR->CR1 = (PWR->CR1 & ~PWR_CR1_SVOS_Msk) | 0x2 << PWR_CR1_SVOS_Pos;
		case eVOSs::LP_SVOS5:
			ModifyReg(PWR->D3CR, PWR_D3CR_VOS_Msk, 0x1 << PWR_D3CR_VOS_Pos);
			while (!(PWR->D3CR & PWR_D3CR_VOSRDY)) __asm("");
			ModifyReg(PWR->CR1, PWR_CR1_SVOS_Msk, 0x1 << PWR_CR1_SVOS_Pos);
			break;
		}
		return true;
	}

	enum class eDomainPowerModes	{ DStop, DStandby };
	enum class eCPUs		{ CM7, CM4 };
	enum class eDomains		{ D1, D2, D3 };

	void PowerMode(eDomainPowerModes dm, eCPUs CPU, eDomains Domain)
	{
		uint32_t CPUCR = (CPU == eCPUs::CM7 ? PWR->CPUCR : PWR->CPU2CR);
		
		switch (Domain)
		{
		case eDomains::D1:
			switch (dm)
			{
			case eDomainPowerModes::DStop:
				CPUCR &= ~PWR_CPUCR_PDDS_D1_Msk;
				break;
			case eDomainPowerModes::DStandby:
				CPUCR |= PWR_CPUCR_PDDS_D1;
				break;
			}
			break;
		case eDomains::D2:
			switch (dm)
			{
			case eDomainPowerModes::DStop:
				CPUCR &= ~PWR_CPUCR_PDDS_D2_Msk;
				break;
			case eDomainPowerModes::DStandby:
				CPUCR |= PWR_CPUCR_PDDS_D2;
				break;
			}
			break;
		case eDomains::D3:
			assert(false && "illegal domain");
			break;
		}
		(CPU == eCPUs::CM7 ? PWR->CPUCR : PWR->CPU2CR) = CPUCR;
	}
	
	enum class eStopModeEntry { WFI, WFE };
	void EnterLowPower(eStopModeEntry sme)
	{

#ifdef CORE_CM7
		__WFE();
#else	
		__SEV();
		__WFE();
#endif
		
		// Set SLEEPDEEP bit of Cortex System Control Register 
		SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;

		// Ensure that all instructions are done before entering STOP mode 
//		__DSB();
	//	__ISB();

		// Select Stop mode entry 
		if (sme == eStopModeEntry::WFI)
		{
			// Request Wait For Interrupt
			__WFI();
		}
		else
		{
			// Request Wait For Event
			__WFE();
		}
		
		// Clear SLEEPDEEP bit of Cortex-Mx in the System Control Register 
		SCB->SCR &= ~SCB_SCR_SLEEPDEEP_Msk;

	}
	
#ifdef CORE_CM4
	void SleepCM4()
	{
		VOS(eVOSs::VOS3);
		PowerMode(eDomainPowerModes::DStop, eCPUs::CM4, eDomains::D2);
		/*
		SysTick->CTRL &= ~SysTick_CTRL_TICKINT_Msk;
		__set_PRIMASK(1);
		__set_FAULTMASK(1);
		__disable_fault_irq();
		__disable_irq();
		
		EXTI->IMR1 = 0;
		EXTI->EMR1 = 0;
		EXTI->IMR2 = 0;
		EXTI->EMR2 = 0;
		EXTI->IMR3 = 0;
		EXTI->EMR3 = 0;

		EXTI->C2IMR1 = 0;
		EXTI->C2EMR1 = 0;
		EXTI->C2IMR2 = 0;
		EXTI->C2EMR2 = 0;
		EXTI->C2IMR3 = 0;
		EXTI->C2EMR3 = 0;
*/
		EnterLowPower(eStopModeEntry::WFI);
	}
#endif
#ifdef CORE_CM7
	bool WaitForCM4ToSleep(int timeout = INT32_MAX)
	{
		while ((RCC->CR & RCC_CR_D2CKRDY) && timeout > 0)
		{
			if (timeout != INT32_MAX) --timeout;
		}
		return !(RCC->CR & RCC_CR_D2CKRDY);
	}
	bool WaitForCM4ToRun(int timeout = INT32_MAX) 
	{
		
		while (!(RCC->CR & RCC_CR_D2CKRDY) && timeout > 0)
		{
			if (timeout != INT32_MAX) --timeout;
		}
		return (RCC->CR & RCC_CR_D2CKRDY);
	}

#endif

};
