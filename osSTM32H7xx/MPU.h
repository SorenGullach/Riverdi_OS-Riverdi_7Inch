#pragma once

#include "stm32h7xx.h"
#include <stdint.h>
#include <assert.h>

// https://www.st.com/resource/en/programming_manual/pm0253-stm32f7-series-and-stm32h7-series-cortexm7-processor-programming-manual-stmicroelectronics.pdf
// section 4.6
/*
	The Memory Protection Unit (MPU) divides the memory map into a number of regions, and
	defines the location, size, access permissions, and memory attributes of each region. It
	supports:
	• Independent attribute settings for each region.
	• Overlapping regions.
	• Export of memory attributes to the system.
	The memory attributes affect the behavior of memory accesses to the region. The Cortex®-
	M7 MPU defines:
	• 8 or 16 separate memory regions, 0-7 or 0-15.
	• A background region.
	When memory regions overlap, a memory access is affected by the attributes of the region
	with the highest number. For example, the attributes for region 7 take precedence over the
	attributes of any region that overlaps region 7.
	The background region has the same memory access attributes as the default memory
	map, but is accessible from privileged software only.
	The Cortex®-M7 MPU memory map is unified. This means instruction accesses and data
	accesses have same region settings.
	If a program accesses a memory location that is prohibited by the MPU, the processor
	generates a MemManage fault. This causes a fault exception, and might cause termination
	of the process in an OS environment. In an OS environment, the kernel can update the MPU
	region setting dynamically based on the process to be executed. Typically, an embedded
	OS uses the MPU for memory protection.
 **/

class hwMPU
{
public:
	enum class eAccessPermission
	{
		/*
		AP[2:0]		Privileged permissions	Unprivileged permissions	Description
		000			No access				No access					All accesses generate a permission fault
		001			RW						No access					Access from a privileged software only
		010			RW						RO							Written by an unprivileged software generates a permission fault
		011			RW						RW							Full access
		100			Unpredictable			Unpredictable				Reserved
		101			RO						No access					Read by a privileged software only
		110			RO						RO							Read only, by privileged or unprivileged software
		111			RO						RO							Read only, by privileged or unprivileged software
		*/	
		NoAccess = 0,
		PRW = 1,
		PRW_URO = 2,
		PRW_URW = 3,
		PRO = 5,
		PRO_URO = 6
	};
		
	enum class eTexLevel
	{
		/*
		TEX		C	B	Memory type			Description							Shareable
		000		0	0	Strongly ordered	Strongly ordered					Yes
		000		0	1	Device				Shared device						Yes
		000		1	0	Normal				Write through, no write allocate	S bit
		000		1	1	Normal				Write-back, no write allocate		S bit
		001		0	0	Normal				Non-cacheable						S bit
		001		0	1	Reserved			Reserved							Reserved
		001		1	0	Undefined			Undefined							Undefined
		001		1	1	Normal				Write-back, write and read allocate S bit
		010		0	0	Device				Non-shareable device				No
		010		0	1	Reserved			Reserved							Reserved
		*/
		Level0 = 0,
		Level1 = 1,
		Level2 = 2
	};
	
	enum class eSize
	{
		S32B   = 0x04,
		S64B   = 0x05,
		S128B  = 0x06,
		S256B  = 0x07,
		S512B  = 0x08,
		S1KB   = 0x09,
		S2KB   = 0x0A,
		S4KB   = 0x0B,
		S8KB   = 0x0C,
		S16KB  = 0x0D,
		S32KB  = 0x0E,
		S64KB  = 0x0F,
		S128KB = 0x10,
		S256KB = 0x11,
		S512KB = 0x12,
		S1MB   = 0x13,
		S2MB   = 0x14,
		S4MB   = 0x15,
		S8MB   = 0x16,
		S16MB  = 0x17,
		S32MB  = 0x18,
		S64MB  = 0x19,
		S128MB = 0x1A,
		S256MB = 0x1B,
		S512MB = 0x1C,
		S1GB   = 0x1D,
		S2GB   = 0x1E,
		S4GB   = 0x1F,
	};
	
	static void DisableRegion(uint8_t no, uint32_t address)
	{
		assert(no >= 0 && no < ((MPU->TYPE && MPU_TYPE_DREGION_Msk) << MPU_TYPE_DREGION_Pos));
		MPU->RNR = no; // Select region 0
		MPU->RBAR = address;
		MPU->RASR = 0;
	}
	
	static void EnableRegion( uint8_t no,
		uint32_t address,
		eSize size,
		bool executeNever,
		bool shareable,
		bool cacheable,
		bool bufferable,
		eAccessPermission accessPermission,
		eTexLevel texLevel)
	{

		assert(no >= 0 && no < ((MPU->TYPE && MPU_TYPE_DREGION_Msk) << MPU_TYPE_DREGION_Pos));
		
		Disable();
		
		// Configure region 
		MPU->RNR = no; // Select region 0
		MPU->RBAR = address;
		MPU->RASR = 
			(uint32_t)0 << MPU_RASR_ATTRS_Pos |
			(uint32_t)executeNever << MPU_RASR_XN_Pos |
			(uint32_t)accessPermission << MPU_RASR_AP_Pos | 
			(uint32_t)texLevel << MPU_RASR_TEX_Pos |
			(uint32_t)shareable << MPU_RASR_S_Pos |
			(uint32_t)cacheable << MPU_RASR_C_Pos |
			(uint32_t)bufferable << MPU_RASR_B_Pos |
			(uint32_t)size << MPU_RASR_SIZE_Pos |
		    MPU_RASR_ENABLE_Msk
			;

		// Enable MPU
		Enable(eControlMode_Hartfault::None, eControlMode_Privileged::Privileged_default);
	}

	enum class eControlMode_Hartfault
	{
		/*
		Enables the operation of MPU during hard fault, NMI, and FAULTMASK
		handlers.
		When the MPU is enabled:
		0: MPU is disabled during hard fault, NMI, and FAULTMASK handlers,
		regardless of the value of the ENABLE bit.
		1: The MPU is enabled during hard fault, NMI, and FAULTMASK handlers.
		When the MPU is disabled, if this bit is set to 1 the behavior is
		Unpredictable.
		*/
			None = 0,
		HartFault_NMI = 1
	};
	enum class eControlMode_Privileged
	{
		/*
		Enables privileged software access to the default memory map:
		0: If the MPU is enabled, disables use of the default memory map. Any
		memory access to a location not covered by any enabled region causes a
		fault.
		1: If the MPU is enabled, enables use of the default memory map as a
		background region for privileged software accesses.
		When enabled, the background region acts as if it is region number -1. Any
		region that is defined and enabled has priority over this default map.
		If the MPU is disabled, the processor ignores this bit.
		*/
			None = 0, 
		Privileged_default = 1
	};
	// Enable the MPU 
	static void Enable(eControlMode_Hartfault hartfault, eControlMode_Privileged privileged)
	{
		MPU->CTRL = 
			(uint32_t)hartfault << MPU_CTRL_HFNMIENA_Pos | 
			(uint32_t)privileged << MPU_CTRL_PRIVDEFENA_Pos |
			MPU_CTRL_ENABLE_Msk;
		// Enable fault exceptions 
		SCB->SHCSR |= SCB_SHCSR_MEMFAULTENA_Msk;

		// Ensure MPU setting take effects 
		__DSB();
		__ISB();
	}
	// Disable MPU
	static void Disable()
	{
		// Make sure outstanding transfers are done 
		__DMB();
		// Disable fault exceptions 
		SCB->SHCSR &= ~SCB_SHCSR_MEMFAULTENA_Msk;
		// Disable the MPU and clear the control register
		MPU->CTRL = 0;
	}
};