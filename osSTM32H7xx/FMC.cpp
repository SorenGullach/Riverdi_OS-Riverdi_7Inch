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

#include <FMC.h>
#include <assert.h>
#include <Utils.h>

void hwFMCSDRAM::Init()
{
	// The initialization sequence is managed by software. If the two banks are used, the
	// initialization sequence must be generated simultaneously to Bank 1 and Bank 2 by setting
	// the Target Bank bits CTB1 and CTB2 in the FMC_SDCMR register:*/

	// 1. Program the memory device features into the FMC_SDCRx register. The SDRAM
	// clock frequency, RBURST and RPIPE must be programmed in the FMC_SDCR1 register.
	ModifyReg(FMC_Bank5_6_R->SDCR[0],
		FMC_SDCRx_SDCLK_Msk | FMC_SDCRx_RBURST_Msk | FMC_SDCRx_RPIPE_Msk,
		(uint32_t)ClkConfiguration << FMC_SDCRx_SDCLK_Pos |
		(BurstRead ? 1U : 0U)<<FMC_SDCRx_RBURST_Pos |
		(uint32_t)ReadPipe << FMC_SDCRx_RPIPE_Pos);

	ModifyReg(FMC_Bank5_6_R->SDCR[1],
		FMC_SDCRx_NC_Msk| FMC_SDCRx_NR_Msk | FMC_SDCRx_MWID_Msk | FMC_SDCRx_NB_Msk | FMC_SDCRx_CAS_Msk|	FMC_SDCRx_WP_Msk, 
		(WriteProtection ? 1U : 0U) << FMC_SDCRx_WP_Pos |
		(uint32_t)CASLatency << FMC_SDCRx_CAS_Pos |
		(NumberInternalBanks4 ? 1U : 0U) << FMC_SDCRx_NB_Pos |
		(uint32_t)DataBusWidth << FMC_SDCRx_MWID_Pos |
		(uint32_t)NumberRowAddressBits << FMC_SDCRx_NR_Pos |
		(uint32_t)NumberColumnAddressBits << FMC_SDCRx_NC_Pos);
	
	// 2. Program the memory device timing into the FMC_SDTRx register. The TRP and TRC
	// timings must be programmed in the FMC_SDTR1 register.
	ModifyReg(FMC_Bank5_6_R->SDTR[0],
		FMC_SDTRx_TRC_Msk | FMC_SDTRx_TRP_Msk |FMC_SDTRx_TWR_Msk,
		(RowCycleDelay - 1) << FMC_SDTRx_TRC_Pos |
		(RowPrechargeDelay - 1) << FMC_SDTRx_TRP_Pos |
		(16 - 1) << FMC_SDTRx_TWR_Pos); // not used bank

	ModifyReg(FMC_Bank5_6_R->SDTR[1], 
		FMC_SDTRx_TRP_Msk | FMC_SDTRx_TRC_Msk | FMC_SDTRx_TMRD_Msk |
		FMC_SDTRx_TXSR_Msk | FMC_SDTRx_TRAS_Msk | FMC_SDTRx_TWR_Msk |
		FMC_SDTRx_TRCD_Msk,
		0 << FMC_SDTRx_TRP_Pos | //,
		0 << FMC_SDTRx_TRC_Pos |  //,
		(LoadModeRegisterActive - 1) << FMC_SDTRx_TMRD_Pos |
		(ExitSelfRefreshDelay - 1) << FMC_SDTRx_TXSR_Pos |
		(SelfRefresTime - 1) << FMC_SDTRx_TRAS_Pos |
		(RecoveryDelay - 1) << FMC_SDTRx_TWR_Pos |
		(RowColumnDelay - 1) << FMC_SDTRx_TRCD_Pos);
	
	// FMC controller Enable 
	FMC_Bank1_R->BTCR[0] |= FMC_BCR1_FMCEN;

	// 3. Set MODE bits to ‘001’ and configure the Target Bank bits (CTB1 and/or CTB2) in the
	// FMC_SDCMR register to start delivering the clock to the memory (SDCKE is driven high).
	// 4. Wait during the prescribed delay period. Typical delay is around 100 μs (refer to the
	// SDRAM datasheet for the required delay after power-up).
	// 5. Set MODE bits to ‘010’ and configure the Target Bank bits (CTB1 and/or CTB2) in the
	// FMC_SDCMR register to issue a “Precharge All” command.
	// 6. Set MODE bits to ‘011’, and configure the Target Bank bits (CTB1 and/or CTB2) as well
	// as the number of consecutive Auto-refresh commands (NRFS) in the FMC_SDCMR
	// register. Refer to the SDRAM datasheet for the number of Auto-refresh commands that
	// should be issued. Typical number is 8.
	// 7. Configure the MRD field, set the MODE bits to ‘100’, and configure the Target Bank bits
	// (CTB1 and/or CTB2) in the FMC_SDCMR register to issue a “Load Mode Register”
	// command and program the SDRAM device. In particular the Burst Length (BL) has to
	// be set to ‘1’) and the CAS latency has to be selected. If the Mode Register is not the
	// same for both SDRAM banks, this step has to be repeated twice, once for each bank
	// and the Target Bank bits set accordingly. For mobile SDRAM devices, the MRD field is
	// also used to configure the extended mode register while issuing the Load Mode Register”
	Init_SDRAMDevice();
	
	// 8. Program the refresh rate in the FMC_SDRTR register
	// The refresh rate corresponds to the delay between refresh cycles. Its value must be
	// adapted to SDRAM devices. */
	ModifyReg(FMC_Bank5_6_R->SDRTR, FMC_SDRTR_COUNT_Msk, REFRESH_COUNT << FMC_SDRTR_COUNT_Pos);

}

void hwFMCSDRAM::Init_SDRAMDevice()
{
	SendCommand(eMemoryModes::Clock_Configuration_Enable, SDBank1, SDBank2, 1, 0); // 0b001
	SendCommand(eMemoryModes::PALL_Command, SDBank1, SDBank2, 1, 0); // 0b010
	SendCommand(eMemoryModes::Auto_Refresh_Command, SDBank1, SDBank2, 1, 0); // 0b011
	uint16_t mrd = 
		(uint32_t)eBurstLength::LENGTH_1 << 0;
		mrd |= (uint32_t)eBurstType::SEQUENTIAL << 3;
		mrd |= (uint32_t)eCAS_Latency::LATENCY_3 << 4;
		mrd |= (uint32_t)eOperatingMode::STANDARD << 7;
		mrd |= (uint32_t)eWriteBurstMode::SINGLE << 9;
	
	SendCommand(eMemoryModes::Load_Mode_Register, SDBank1, SDBank2, 1, mrd); // 0b100
}
void hwFMCSDRAM::SendCommand(eMemoryModes mode, bool bank1, bool bank2, uint8_t autoRefreshCycles, uint16_t registerDefinition)
{
	uint32_t cmd = 
		(uint32_t)mode << FMC_SDCMR_MODE_Pos; 
		cmd |= bank1 ? FMC_SDCMR_CTB1 : 0U;
		cmd |= bank2 ? FMC_SDCMR_CTB2 : 0U;
		cmd |= (autoRefreshCycles - 1) << FMC_SDCMR_NRFS_Pos;
		cmd |= registerDefinition << FMC_SDCMR_MRD_Pos;
	
	FMC_Bank5_6_R->SDCMR = cmd;
	osDelay(100);
}