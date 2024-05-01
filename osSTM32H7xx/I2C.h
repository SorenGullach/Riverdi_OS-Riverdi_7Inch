
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

#pragma once

#include "stm32h7xx.h"
#include <GPIO.h>
#include <SysTick.h>
#include <Utils.h>
#include <MemoryDefs.h>

template<int ch>
	class hwI2C
	{
	public:
		void Init()
		{
			switch (ch)
			{
			case 1:
				pI2C = I2C1;
				break;
			case 2:
				pI2C = I2C2;
				break;
			case 3:
				pI2C = I2C3;
				break;
			case 4:
				pI2C = I2C4;
				break;
			}
			
			// select clock source
			if (ch == 4)
				ModifyReg(RCC->D3CCIPR, RCC_D3CCIPR_I2C4SEL_Msk, (uint32_t)I2C4ClockSource::RCC_PCLK4 << RCC_D3CCIPR_I2C4SEL_Pos);
			else
				ModifyReg(RCC->D2CCIP2R, RCC_D2CCIP2R_I2C123SEL_Msk, (uint32_t)I2C123ClockSource::RCC_PCLK1 << RCC_D2CCIP2R_I2C123SEL_Pos);
			// enable clock
			RCC->APB1LENR |= RCC_APB1LENR_I2C1EN;
			
			// disable 
			pI2C->CR1 &= ~I2C_CR1_PE;
			
			// Select analog filter
			//pI2C->CR1 |= I2C_CR1_ANFOFF;
			ModifyReg(pI2C->CR1, I2C_CR1_DNF_Msk, 0);

			// I2C timings
			// 0x0 0 B 0 3FDB
			ModifyReg(pI2C->TIMINGR, I2C_TIMINGR_PRESC_Msk, 0 << I2C_TIMINGR_PRESC_Pos);
			ModifyReg(pI2C->TIMINGR, I2C_TIMINGR_SCLDEL_Msk, 0xB << I2C_TIMINGR_SCLDEL_Pos);
			ModifyReg(pI2C->TIMINGR, I2C_TIMINGR_SDADEL_Msk, 0 << I2C_TIMINGR_SDADEL_Pos);
			ModifyReg(pI2C->TIMINGR, I2C_TIMINGR_SCLH_Msk, 0x3F << I2C_TIMINGR_SCLH_Pos);
			ModifyReg(pI2C->TIMINGR, I2C_TIMINGR_SCLL_Msk, 0xDB << I2C_TIMINGR_SCLL_Pos);
			
			// Own address
			
			// Enable the AUTOEND by default, and enable NACK (should be disable only during Slave process 
			pI2C->CR2 = (I2C_CR2_AUTOEND | I2C_CR2_NACK);

			// Enable
			pI2C->CR1 |= I2C_CR1_PE;
			
		}

		void Reset()
		{
			pI2C->CR1 &= ~I2C_CR1_PE;
			pI2C->CR1 &= ~I2C_CR1_PE;
			pI2C->CR1 &= ~I2C_CR1_PE;
			pI2C->CR1 &= ~I2C_CR1_PE;
			pI2C->CR1 |= I2C_CR1_PE;
		}
		
		bool WaitOnReady()
		{
			int cnt = 100000;
			while (--cnt > 0 && (pI2C->ISR & I2C_ISR_BUSY) != 0) __asm("");
			return cnt == 0;
		}
		bool WaitOnTransmitReady()
		{
			int cnt = 10000;
			while (--cnt > 0 && (pI2C->ISR & I2C_ISR_TXIS) == 0)__asm("");
			return cnt == 0;
		}
		bool WaitOnTransferComplete()
		{
			int cnt = 10000;
			while (--cnt > 0 && (pI2C->ISR & I2C_ISR_TC) == 0) __asm("");
			return cnt == 0;
		}
		bool IsTransferComplete()
		{
			return (pI2C->ISR & I2C_ISR_TC)>0;
		}
		bool WaitOnReceiveDataReady()
		{
			int cnt = 10000;
			while (--cnt > 0 && (pI2C->ISR & I2C_ISR_RXNE) == 0) __asm("");
			return cnt == 0;
		}

		bool SendCmd(uint8_t adr, uint8_t cmd, bool stop=false)
		{
			if (WaitOnReady())
			{
			//	Printf("I2C Not ready \n");
				Reset();
				return false;
			}
			
			pI2C->CR2 = 0;
			pI2C->ISR |= I2C_ISR_TXE;
			
			pI2C->CR2 = (adr << 1) << I2C_CR2_SADD_Pos;
			pI2C->CR2 |= 1 << I2C_CR2_NBYTES_Pos;
			pI2C->CR2 |= I2C_CR2_AUTOEND | I2C_CR2_NACK;
			pI2C->CR2 |= I2C_CR2_START;
			
			if (WaitOnTransmitReady())
			{
//				Printf("I2C No transmit ready\n");
			//	pI2C->CR2 |= I2C_CR2_STOP;
				return false;
			}
			
			pI2C->TXDR = cmd;

			//WaitOnTransferComplete();
			if (stop)
				pI2C->CR2 |= I2C_CR2_STOP;
			return true;
		}
		
		uint8_t Read(int8_t adr, uint8_t *data, uint8_t nbytes)
		{
			if (WaitOnReady())
			{
				Printf("I2C Read not ready \n");
				return 0;
			}

			*data = (uint8_t)pI2C->RXDR;

			pI2C->CR2 = (adr << 1) << I2C_CR2_SADD_Pos;
			pI2C->CR2 |= I2C_CR2_RD_WRN;
			pI2C->CR2 |= nbytes << I2C_CR2_NBYTES_Pos;
			pI2C->CR2 |= I2C_CR2_AUTOEND | I2C_CR2_NACK;
			pI2C->CR2 |= I2C_CR2_START;

			uint8_t n = 0;
			while (nbytes > 0 && !IsTransferComplete())
			{
				if (WaitOnReceiveDataReady())
				{
					Printf("I2C No receive ready %d\n", n);
					break;
				}
				else
				{
					*data = (uint8_t)pI2C->RXDR;
					//				Printf("%d 0x%02X\n",n,*data);
					data++; nbytes--; n++;
				}
			}
			pI2C->CR2 |= I2C_CR2_STOP;
			WaitOnTransferComplete();
			return n;
		}
	
	private:
		enum class I2C123ClockSource {
			RCC_PCLK1 = 0, // rcc_pclk1 clock is selected as kernel clock (default after reset)
			PLL3_R_CK = 1, // pll3_r_ck clock is selected as kernel clock
			HSI_KER_CK = 2, // hsi_ker_ck clock is selected as kernel clock
			CSI_KER_CK = 3      // csi_ker_ck clock is selected as kernel clock
		};
		enum class I2C4ClockSource {
			RCC_PCLK4 = 0, // rcc_pclk4 clock selected as kernel peripheral clock (default after reset)
			PLL3_R_CK = 1, // pll3_r_ck clock selected as kernel peripheral clock
			HSI_KER_CK = 2, // hsi_ker_ck clock selected as kernel peripheral clock
			CSI_KER_CK = 3      // csi_ker_ck clock selected as kernel peripheral clock
		};
		
		I2C_TypeDef *pI2C;
	};

class hwI2C1 : public hwI2C<1>
{
	
private:
	hwGPIO SDA {GPIOB, 7, 4, hwGPIO::ePP::OpenDrian, hwGPIO::ePUPD::PullUp, hwGPIO::eSpeed::VeryHigh  }
	;
	hwGPIO SCL {GPIOB, 6, 4, hwGPIO::ePP::OpenDrian, hwGPIO::ePUPD::PullUp, hwGPIO::eSpeed::VeryHigh  }
	;
};