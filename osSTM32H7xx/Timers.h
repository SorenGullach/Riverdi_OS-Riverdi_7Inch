#pragma once

#include "stm32h7xx.h"
#include <RCC.h>
#include <GPIO.h>
#include <SysTick.h>
#include <Utils.h>

//https://www.st.com/resource/en/reference_manual/rm0399-stm32h745755-and-stm32h747757-advanced-armbased-32bit-mcus-stmicroelectronics.pdf#page=1911"

class Timer
{
protected:
	Timer(TIM_TypeDef* timerAddress)
	{
		pTIM = timerAddress;
	}
	TIM_TypeDef *pTIM;

public:
	enum class eMode
	{
		PWM1, 
		PWM2
	};
};
template<typename T>
	class TimerCounter : Timer
	{
	public:
		void Prescaler(uint16_t div) 
		{ 
			MODIFY_REG(pTIM->CR1, TIM_CR1_UDIS_Msk, TIM_CR1_UDIS);
			pTIM->PSC = div; 
			MODIFY_REG(pTIM->CR1, TIM_CR1_UDIS_Msk, 0);
		}
		T Counter() { return pTIM->CNT;	}
		T Periode() { return pTIM->ARR; }
		void Periode(T value) 
		{ 
			MODIFY_REG(pTIM->CR1, TIM_CR1_UDIS_Msk, TIM_CR1_UDIS);
			pTIM->ARR = value; 
			MODIFY_REG(pTIM->CR1, TIM_CR1_UDIS_Msk, 0);
		}
		void Repetion(T value) { pTIM->RCR = value; }

	protected:
		TimerCounter(TIM_TypeDef* timerAddress)
			: Timer(timerAddress)
		{
		}
		void Start() 
		{ 
			MODIFY_REG(pTIM->EGR, TIM_EGR_UG_Msk, TIM_EGR_UG);
			pTIM->CR1 |= TIM_CR1_CEN; 
		}
		void Stop() { pTIM->CR1 &= ~TIM_CR1_CEN; }

		void Mode(bool ARPE = false)
		{
			MODIFY_REG(pTIM->CR1, TIM_CR1_DIR_Msk, 0);
			MODIFY_REG(pTIM->CR1, TIM_CR1_CMS_Msk, 0);
			MODIFY_REG(pTIM->CR1, TIM_CR1_ARPE_Msk, ARPE ? TIM_CR1_ARPE : 0); // Auto-reload preload enable
		}

	};

#define STR(x) #x
#define CONCAT(a,b) a##b
template<typename T, int no>
	class TimerCaptureCompare : public Timer
	{
	public:
		T Compare() 
		{ 
			if (no == 1)
				return pTIM->CCR1;				
			if (no == 2)
				return pTIM->CCR2;				
			else
				return 0;
		}
		void Compare(T value) 
		{ 
			if (no == 1)
				pTIM->CCR1 = value; 
			if (no == 2)
				pTIM->CCR2 = value; 
		}
	protected:
		TimerCaptureCompare(TIM_TypeDef* timerAddress)
			: Timer(timerAddress)
		{
		}
	
		void Mode(eMode mode, bool OCxE = true, bool OCxNE = false, bool OCxP = false)
		{
			switch (mode)
			{
			case eMode::PWM1:
			case eMode::PWM2:
				if (no == 1)
				{
					ModifyReg(pTIM->CCMR1, TIM_CCMR1_CC1S_Msk, 0B00 << TIM_CCMR1_CC1S_Pos); //  CC1 channel is configured as output.
					ModifyReg(pTIM->CCMR1, TIM_CCMR1_OC1FE_Msk, 0B0 << TIM_CCMR1_OC1FE_Pos); // Output Compare 1 fast enable
					ModifyReg(pTIM->CCMR1, TIM_CCMR1_OC1PE_Msk, 0B1 << TIM_CCMR1_OC1PE_Pos); // Output Compare 1 preload enable
					if (mode == eMode::PWM1)
						ModifyReg(pTIM->CCMR1, TIM_CCMR1_OC1M_Msk, 0B0110 << TIM_CCMR1_OC1M_Pos); // PWM mode 1
					else
						ModifyReg(pTIM->CCMR1, TIM_CCMR1_OC1M_Msk, 0B0111 << TIM_CCMR1_OC1M_Pos); // PWM mode 2
		
					ModifyReg(pTIM->CCER, TIM_CCER_CC1E_Msk, OCxE ? TIM_CCER_CC1E : 0); //  Capture/Compare OCx output enable
					ModifyReg(pTIM->CCER, TIM_CCER_CC1NE_Msk, OCxNE ? TIM_CCER_CC1NE : 0); //  Capture/Compare OCxN output enable
				
					ModifyReg(pTIM->CCER, TIM_CCER_CC1P_Msk, OCxP ? TIM_CCER_CC1P : 0); //  Capture/Compare output polarity
					Output(true);
				}
				break;			
			}
		}
		void Output(bool enable, bool disableHigh = false)
		{
			ModifyReg(pTIM->BDTR, TIM_BDTR_MOE_Msk, enable ? TIM_BDTR_MOE : 0); //  Main output enable
			while (((pTIM->BDTR & TIM_BDTR_MOE_Msk) != 0) != enable) __asm("");
			ModifyReg(pTIM->BDTR, TIM_BDTR_OSSI_Msk, disableHigh ? TIM_BDTR_OSSI : 0); // Off-state selection for Idle mode
			if (no == 1)
				ModifyReg(pTIM->CR2, TIM_CR2_OIS1_Msk, 0); //  Output Idle state 1 (OC1 output)
			if (no == 2) 
				ModifyReg(pTIM->CR2, TIM_CR2_OIS2_Msk, 0); //  Output Idle state 1 (OC2 output)
		}
	};


class Timer15 : public TimerCounter<uint16_t>, public TimerCaptureCompare<uint16_t, 1>, public TimerCaptureCompare<uint16_t, 2>
{
public:
	enum class eInputClk
	{
		Internal, 
		External, 
		Trigger
	};

	Timer15(eInputClk src)
		: TimerCounter(TIM15)
		, TimerCaptureCompare<uint16_t, 1>(TIM15)
		, TimerCaptureCompare<uint16_t, 2>(TIM15)
	{
		RCC->APB2ENR |= RCC_APB2ENR_TIM15EN;

		switch (src)
		{
			// Slave mode selection
		case eInputClk::Internal: //  Slave mode disabled - if CEN = ‘1’ then the prescaler is clocked directly by the internal clock.
			ModifyReg(TIM15->SMCR, TIM_SMCR_SMS_Msk, 0B0000 << TIM_SMCR_SMS_Pos); 
			break;
		case eInputClk::External: //  External Clock Mode 1 - Rising edges of the selected trigger (TRGI) clock the counter.
			ModifyReg(TIM15->SMCR, TIM_SMCR_SMS_Msk, 0B0111 << TIM_SMCR_SMS_Pos); 
			break;
		case eInputClk::Trigger: // Trigger Mode - The counter starts at a rising edge of the trigger TRGI (but it is not reset). Only the start of the counter is controlled.
			ModifyReg(TIM15->SMCR, TIM_SMCR_SMS_Msk, 0B0110 << TIM_SMCR_SMS_Pos); 
			break;
		}
	}
	// Call when clock is knowen, 
	// freq is the CNT freq
	uint16_t FindPrescalerValue(uint32_t freq, uint32_t clock = 0)
	{
		if (clock == 0)
			clock = hwSysClock::APB2TClk(); // domain 2 timer clock
		uint32_t div = (clock / freq) - 1;
		assert(div <= UINT16_MAX && "Freq to low for this clock");
		return div;
	}

	void Mode(R_t<uint8_t, 1, 2> ccr, eMode mode)
	{
		switch (mode)
		{
		case eMode::PWM1:
		case eMode::PWM2:
			TimerCounter::Mode();
			switch (ccr)
			{
			case 1:	TimerCaptureCompare<uint16_t, 1>::Mode(mode); break;
			case 2:	TimerCaptureCompare<uint16_t, 2>::Mode(mode); break;
			}
			break;			
		}
	}
	
	uint16_t Compare(R_t<uint8_t, 1, 2> ccr) 
	{
		switch (ccr)
		{
		case 1:	return TimerCaptureCompare<uint16_t, 1>::Compare(); 
		case 2:	return TimerCaptureCompare<uint16_t, 2>::Compare(); 
		}
		return 0;
	}
	void Compare(R_t<uint8_t, 1, 2> ccr, uint16_t value) 
	{
		switch (ccr)
		{
		case 1:	TimerCaptureCompare<uint16_t, 1>::Compare(value); break;
		case 2:	TimerCaptureCompare<uint16_t, 2>::Compare(value); break;
		}
	}
	
	void Start() 
	{ 
		TimerCounter::Start();
		TimerCaptureCompare<uint16_t, 1>::Output(true);
		TimerCaptureCompare<uint16_t, 2>::Output(true);
	}
	void Stop() 
	{ 
		TimerCounter::Stop();
		TimerCaptureCompare<uint16_t, 1>::Output(false);
		TimerCaptureCompare<uint16_t, 2>::Output(false);
	}


private:
};

class Timer16 : public TimerCounter<uint16_t>
{
protected:
	Timer16(uint16_t div)
		: TimerCounter(TIM16)
	{
		Prescaler(div);
	}
	
private:
};

class Timer17 : public TimerCounter<uint16_t>
{
public:
	Timer17(uint16_t div)
		: TimerCounter(TIM17)
	{
		Prescaler(div);
	}
};