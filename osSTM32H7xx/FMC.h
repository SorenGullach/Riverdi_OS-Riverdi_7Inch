#pragma once

#include "stm32h7xx.h"
#include <GPIO.h>
#include <SysTick.h>
#include <Utils.h>

class hwFMCSDRAM
{
public:
	void *GetSDRAMAddr()
	{
		return (void *)SDRAM_DEVICE_ADDR;
	}
	uint32_t GetSDRAMSize()
	{
		return SDRAM_DEVICE_SIZE;
	}
protected:
	void Init();
	
	void UnitTest()
	{
		Printf("SDRAM test start\n");

		uint32_t test = 0x10101010;
		uint32_t *pData = (uint32_t *)GetSDRAMAddr();
		uint32_t SizeW = GetSDRAMSize() / sizeof(uint32_t);
		
		Printf("SDRAM test start write\n");
		uint32_t t = osTicks_ms;
		for (uint32_t i = 0; i < SizeW; i++)
		{
			pData[i] = test;
		}
		t = osTicks_ms - t;
		Printf("SDRAM test write time %d %d W/ms\n", t, SizeW / t);

		Printf("SDRAM test start read\n");
		t = osTicks_ms;
		for (uint32_t i = 0; i < SizeW; i++)
		{
			if (test != pData[i])
			{
				Printf("SDRAM test read error %d\n", i);
				return;
			}
		}	
		Printf("SDRAM test read time %d\n", osTicks_ms - t);
		Printf("SDRAM test end\n");
	}
	
private:
	const uint32_t REFRESH_COUNT = ((uint32_t)0x0603); /* SDRAM refresh counter (100Mhz SD clock) */
	const uint32_t SDRAM_DEVICE_ADDR = 0xD0000000U;
	const uint32_t SDRAM_DEVICE_SIZE = 0x800000U;

	enum class eBurstLength {
		LENGTH_1    = 0U,
		LENGTH_2    = 1U,
		LENGTH_4    = 2U,
		LENGTH_8    = 3U,
		LENGTH_FULL = 7U
	};
	enum class eBurstType {
		SEQUENTIAL  = 0U,
		INTERLEAVED = 1U
	};
	enum class eCAS_Latency {
		LATENCY_2 = 2U,
		LATENCY_3 = 3U
	};
	enum class eOperatingMode {
		STANDARD = 0U
	};
	enum class eWriteBurstMode {
		PROGRAMMED = 0U,
		SINGLE     = 1U
	};
	
	enum class eReadPipe
	{
		No  = 0B00 /* fmc_ker_ck clock cycle delay */,
		One = 0B01,
		Two = 0B10,
	};
	enum class eClkConfiguration
	{
		Disabled = 0B00 /* Disabled */,
		Periods2 = 0B10	/* SDCLK period = 2 x fmc_ker_ck periods */,
		Periods3 = 0B11 /* SDCLK period = 3 x fmc_ker_ck periods */
	};
	enum class eDataBusWidth
	{
		Bits8  = 0B00,
		Bits16 = 0B01,
		Bits32 = 0B10,
	};
	enum class eNumberRowAddressBits
	{
		Bits11 = 0b00,
		Bits12 = 0b01,
		Bits13 = 0b10
	};
	enum class eNumberColumnAddressBits
	{
		Bits8  = 0b00,
		Bits9  = 0b01,
		Bits10 = 0b10,
		Bits11 = 0b11
	};

	const bool SDBank1 = false;
	const bool SDBank2 = true;
	const eClkConfiguration ClkConfiguration = eClkConfiguration::Periods2;
	const bool BurstRead = true;
	const eReadPipe ReadPipe = eReadPipe::No;
	const bool WriteProtection = false;
	const eCAS_Latency CASLatency = eCAS_Latency::LATENCY_3;
	const bool NumberInternalBanks4 = true;
	const eDataBusWidth DataBusWidth = eDataBusWidth::Bits32;
	const eNumberRowAddressBits NumberRowAddressBits  = eNumberRowAddressBits::Bits11;
	const eNumberColumnAddressBits NumberColumnAddressBits = eNumberColumnAddressBits::Bits8;
	
	/* SdramTiming */
	const uint8_t RowColumnDelay = 3;
	const uint8_t RowPrechargeDelay = 3;
	const uint8_t RecoveryDelay = 4;
	const uint8_t RowCycleDelay = 6;
	const uint8_t SelfRefresTime = 4;
	const uint8_t ExitSelfRefreshDelay = 7;
	const uint8_t LoadModeRegisterActive = 3;
	/*
	SdramTiming.LoadToActiveDelay = 3;
	SdramTiming.ExitSelfRefreshDelay = 7;
	SdramTiming.SelfRefreshTime = 4;
	SdramTiming.RowCycleDelay = 7;
	SdramTiming.WriteRecoveryTime = 4;
	SdramTiming.RPDelay = 3;
	SdramTiming.RCDDelay = 3;
	*/
	void Init_SDRAMDevice();
	
	enum class eMemoryModes {
		Normal_Mode                = 0b000,
		Clock_Configuration_Enable = 0b001,
		PALL_Command               = 0b010,
		Auto_Refresh_Command       = 0b011,
		Load_Mode_Register         = 0b100,
		Self_Refresh_Command       = 0b101,
		Power_Down_Command         = 0b110
	};
	void SendCommand(eMemoryModes mode, bool bank1, bool bank2, uint8_t autoRefreshCycles, uint16_t registerDefinition);
	
	hwGPIO SDCLK {GPIOG, 8, 12, hwGPIO::ePP::PushPull, hwGPIO::ePUPD::PullUp, hwGPIO::eSpeed::VeryHigh  }
	;
	hwGPIO SDCKE1 {GPIOH, 7, 12, hwGPIO::ePP::PushPull, hwGPIO::ePUPD::PullUp, hwGPIO::eSpeed::VeryHigh  }
	;
	hwGPIO SDNE1 {GPIOH, 6, 12, hwGPIO::ePP::PushPull, hwGPIO::ePUPD::PullUp, hwGPIO::eSpeed::VeryHigh  }
	;
	hwGPIO SDNRAS {GPIOF, 11, 12, hwGPIO::ePP::PushPull, hwGPIO::ePUPD::PullUp, hwGPIO::eSpeed::VeryHigh  }
	;
	hwGPIO SDNCAS {GPIOG, 15, 12, hwGPIO::ePP::PushPull, hwGPIO::ePUPD::PullUp, hwGPIO::eSpeed::VeryHigh  }
	;
	hwGPIO SDNWE {GPIOH, 5, 12, hwGPIO::ePP::PushPull, hwGPIO::ePUPD::PullUp, hwGPIO::eSpeed::VeryHigh  }
	;
	hwGPIO SDNBL0 {GPIOE, 0, 12, hwGPIO::ePP::PushPull, hwGPIO::ePUPD::PullUp, hwGPIO::eSpeed::VeryHigh  }
	;						 
	hwGPIO SDNBL1 {GPIOE, 1, 12, hwGPIO::ePP::PushPull, hwGPIO::ePUPD::PullUp, hwGPIO::eSpeed::VeryHigh  }
	;						 
	hwGPIO SDNBL2 {GPIOI, 4, 12, hwGPIO::ePP::PushPull, hwGPIO::ePUPD::PullUp, hwGPIO::eSpeed::VeryHigh  }
	;						 
	hwGPIO SDNBL3 {GPIOI, 5, 12, hwGPIO::ePP::PushPull, hwGPIO::ePUPD::PullUp, hwGPIO::eSpeed::VeryHigh  }
	;

	hwGPIO SDA[13] = 
	{ 
		{ GPIOF, 0, 12, hwGPIO::ePP::PushPull, hwGPIO::ePUPD::PullUp, hwGPIO::eSpeed::VeryHigh }	/* A0 */,
		{ GPIOF, 1, 12, hwGPIO::ePP::PushPull, hwGPIO::ePUPD::PullUp, hwGPIO::eSpeed::VeryHigh }	/* A1 */, 
		{ GPIOF, 2, 12, hwGPIO::ePP::PushPull, hwGPIO::ePUPD::PullUp, hwGPIO::eSpeed::VeryHigh }	/* A2 */, 
		{ GPIOF, 3, 12, hwGPIO::ePP::PushPull, hwGPIO::ePUPD::PullUp, hwGPIO::eSpeed::VeryHigh }	/* A3 */,
		{ GPIOF, 4, 12, hwGPIO::ePP::PushPull, hwGPIO::ePUPD::PullUp, hwGPIO::eSpeed::VeryHigh }	/* A4 */,
		{ GPIOF, 5, 12, hwGPIO::ePP::PushPull, hwGPIO::ePUPD::PullUp, hwGPIO::eSpeed::VeryHigh }	/* A5 */,
		{ GPIOF, 12, 12, hwGPIO::ePP::PushPull, hwGPIO::ePUPD::PullUp, hwGPIO::eSpeed::VeryHigh }	/* A6 */,
		{ GPIOF, 13, 12, hwGPIO::ePP::PushPull, hwGPIO::ePUPD::PullUp, hwGPIO::eSpeed::VeryHigh }	/* A7 */,
		{ GPIOF, 14, 12, hwGPIO::ePP::PushPull, hwGPIO::ePUPD::PullUp, hwGPIO::eSpeed::VeryHigh }	/* A8 */,
		{ GPIOF, 15, 12, hwGPIO::ePP::PushPull, hwGPIO::ePUPD::PullUp, hwGPIO::eSpeed::VeryHigh }	/* A9 */,
		{ GPIOG, 0, 12, hwGPIO::ePP::PushPull, hwGPIO::ePUPD::PullUp, hwGPIO::eSpeed::VeryHigh }	/* A10*/,
		{ GPIOG, 1, 12, hwGPIO::ePP::PushPull, hwGPIO::ePUPD::PullUp, hwGPIO::eSpeed::VeryHigh }	/* A11*/,
		{ GPIOG, 2, 12, hwGPIO::ePP::PushPull, hwGPIO::ePUPD::PullUp, hwGPIO::eSpeed::VeryHigh }	/* A12*/
	}
;
	hwGPIO SDBA[2] = 
	{ 
		{ GPIOG, 4, 12, hwGPIO::ePP::PushPull, hwGPIO::ePUPD::PullUp, hwGPIO::eSpeed::VeryHigh }	/* A14 */,
		{ GPIOG, 5, 12, hwGPIO::ePP::PushPull, hwGPIO::ePUPD::PullUp, hwGPIO::eSpeed::VeryHigh }	/* A15 */,
	};

	hwGPIO SDD[32] = 
	{ 
		{ GPIOD, 14, 12, hwGPIO::ePP::PushPull, hwGPIO::ePUPD::PullUp, hwGPIO::eSpeed::VeryHigh }	/* D0 */,
		{ GPIOD, 15, 12, hwGPIO::ePP::PushPull, hwGPIO::ePUPD::PullUp, hwGPIO::eSpeed::VeryHigh }	/* D1 */,
		{ GPIOD, 0, 12, hwGPIO::ePP::PushPull, hwGPIO::ePUPD::PullUp, hwGPIO::eSpeed::VeryHigh }	/* D2 */,
		{ GPIOD, 1, 12, hwGPIO::ePP::PushPull, hwGPIO::ePUPD::PullUp, hwGPIO::eSpeed::VeryHigh }	/* D3 */,
		{ GPIOE, 7, 12, hwGPIO::ePP::PushPull, hwGPIO::ePUPD::PullUp, hwGPIO::eSpeed::VeryHigh }	/* D4 */,
		{ GPIOE, 8, 12, hwGPIO::ePP::PushPull, hwGPIO::ePUPD::PullUp, hwGPIO::eSpeed::VeryHigh }	/* D5 */,
		{ GPIOE, 9, 12, hwGPIO::ePP::PushPull, hwGPIO::ePUPD::PullUp, hwGPIO::eSpeed::VeryHigh }	/* D6 */,
		{ GPIOE, 10, 12, hwGPIO::ePP::PushPull, hwGPIO::ePUPD::PullUp, hwGPIO::eSpeed::VeryHigh }	/* D7 */,
		{ GPIOE, 11, 12, hwGPIO::ePP::PushPull, hwGPIO::ePUPD::PullUp, hwGPIO::eSpeed::VeryHigh }	/* D8 */,
		{ GPIOE, 12, 12, hwGPIO::ePP::PushPull, hwGPIO::ePUPD::PullUp, hwGPIO::eSpeed::VeryHigh }	/* D9 */,
		{ GPIOE, 13, 12, hwGPIO::ePP::PushPull, hwGPIO::ePUPD::PullUp, hwGPIO::eSpeed::VeryHigh }	/* D10 */,
		{ GPIOE, 14, 12, hwGPIO::ePP::PushPull, hwGPIO::ePUPD::PullUp, hwGPIO::eSpeed::VeryHigh }	/* D11 */,
		{ GPIOE, 15, 12, hwGPIO::ePP::PushPull, hwGPIO::ePUPD::PullUp, hwGPIO::eSpeed::VeryHigh }	/* D12 */,
		{ GPIOD, 8, 12, hwGPIO::ePP::PushPull, hwGPIO::ePUPD::PullUp, hwGPIO::eSpeed::VeryHigh }	/* D13 */,
		{ GPIOD, 9, 12, hwGPIO::ePP::PushPull, hwGPIO::ePUPD::PullUp, hwGPIO::eSpeed::VeryHigh }	/* D14 */,
		{ GPIOD, 10, 12, hwGPIO::ePP::PushPull, hwGPIO::ePUPD::PullUp, hwGPIO::eSpeed::VeryHigh }	/* D15 */,
		{ GPIOH, 8, 12, hwGPIO::ePP::PushPull, hwGPIO::ePUPD::PullUp, hwGPIO::eSpeed::VeryHigh }	/* D16 */,
		{ GPIOH, 9, 12, hwGPIO::ePP::PushPull, hwGPIO::ePUPD::PullUp, hwGPIO::eSpeed::VeryHigh }	/* D17 */,
		{ GPIOH, 10, 12, hwGPIO::ePP::PushPull, hwGPIO::ePUPD::PullUp, hwGPIO::eSpeed::VeryHigh }	/* D18 */,
		{ GPIOH, 11, 12, hwGPIO::ePP::PushPull, hwGPIO::ePUPD::PullUp, hwGPIO::eSpeed::VeryHigh }	/* D19 */,
		{ GPIOH, 12, 12, hwGPIO::ePP::PushPull, hwGPIO::ePUPD::PullUp, hwGPIO::eSpeed::VeryHigh }	/* D20 */,
		{ GPIOH, 13, 12, hwGPIO::ePP::PushPull, hwGPIO::ePUPD::PullUp, hwGPIO::eSpeed::VeryHigh }	/* D21 */,
		{ GPIOH, 14, 12, hwGPIO::ePP::PushPull, hwGPIO::ePUPD::PullUp, hwGPIO::eSpeed::VeryHigh }	/* D22 */,
		{ GPIOH, 15, 12, hwGPIO::ePP::PushPull, hwGPIO::ePUPD::PullUp, hwGPIO::eSpeed::VeryHigh }	/* D23 */,
		{ GPIOI, 0, 12, hwGPIO::ePP::PushPull, hwGPIO::ePUPD::PullUp, hwGPIO::eSpeed::VeryHigh }	/* D24 */,
		{ GPIOI, 1, 12, hwGPIO::ePP::PushPull, hwGPIO::ePUPD::PullUp, hwGPIO::eSpeed::VeryHigh }	/* D25 */,
		{ GPIOI, 2, 12, hwGPIO::ePP::PushPull, hwGPIO::ePUPD::PullUp, hwGPIO::eSpeed::VeryHigh }	/* D26 */,
		{ GPIOI, 3, 12, hwGPIO::ePP::PushPull, hwGPIO::ePUPD::PullUp, hwGPIO::eSpeed::VeryHigh }	/* D27 */,
		{ GPIOI, 6, 12, hwGPIO::ePP::PushPull, hwGPIO::ePUPD::PullUp, hwGPIO::eSpeed::VeryHigh }	/* D28 */,
		{ GPIOI, 7, 12, hwGPIO::ePP::PushPull, hwGPIO::ePUPD::PullUp, hwGPIO::eSpeed::VeryHigh }	/* D29 */,
		{ GPIOI, 9, 12, hwGPIO::ePP::PushPull, hwGPIO::ePUPD::PullUp, hwGPIO::eSpeed::VeryHigh }	/* D30 */,
		{ GPIOI, 10, 12, hwGPIO::ePP::PushPull, hwGPIO::ePUPD::PullUp, hwGPIO::eSpeed::VeryHigh }	/* D31 */,
	};

};

class hwFMC : public hwFMCSDRAM
{
public:
	hwFMC()
	{
	}
	void Init()
	{
		// Enable the fmc clock
		RCC->AHB3ENR |= RCC_AHB3ENR_FMCEN;
		// Reset
		RCC->AHB3RSTR |= RCC_AHB3RSTR_FMCRST;
		RCC->AHB3RSTR &= ~RCC_AHB3RSTR_FMCRST;
		// select PLL r2, see also RCC Setup
		ModifyReg(RCC->D1CCIPR, RCC_D1CCIPR_FMCSEL_Msk, 0B10 << RCC_D1CCIPR_FMCSEL_Pos);
		
		// disable FMC
		ModifyReg(FMC_Bank1_R->BTCR[0], 
			FMC_BCR1_FMCEN_Msk |
			FMC_BCR1_BMAP_Msk,
			0);

		hwFMCSDRAM::Init();
	}

	void UnitTest()
	{
		hwFMCSDRAM::UnitTest();
	}
private:
	

};