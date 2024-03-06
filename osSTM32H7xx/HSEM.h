#pragma once

#include "stm32h7xx.h"
#include <stdarg.h>
#include <assert.h>

#ifdef CORE_CM4
#define HSEM_RLR_COREID_CURRENT   (HSEM_CPU2_COREID << HSEM_RLR_COREID_Pos)
#define HSEM_R_COREID_CURRENT   (HSEM_CPU2_COREID << HSEM_R_COREID_Pos)
#define HSEM_R_COREID_OTHER   (HSEM_CPU1_COREID << HSEM_R_COREID_Pos)
#endif /* CORE_CM4 */
#ifdef CORE_CM7
#define HSEM_RLR_COREID_CURRENT   (HSEM_CPU1_COREID << HSEM_RLR_COREID_Pos)
#define HSEM_R_COREID_CURRENT   (HSEM_CPU1_COREID << HSEM_R_COREID_Pos)
#define HSEM_R_COREID_OTHER   (HSEM_CPU2_COREID << HSEM_R_COREID_Pos)
#endif /* CORE_CM7 */

#define FREE_PROCESS_ID HSEM_SEMID_MAX+1
#ifdef CORE_CM4
#define PROCESS_ID ProcessIdCM4
#define CHANGED ChangedCM4
#define NESTEDLOCK NestedLockCM4
#endif
#ifdef CORE_CM7
#define PROCESS_ID ProcessIdCM7
#define CHANGED ChangedCM7
#define NESTEDLOCK NestedLockCM7
#endif

class hwHSEMControl;
	
// so we can make a ref to the object (we cant to a template!)
class hwHSEMBase
{
		friend hwHSEMControl;

public:
	hwHSEMBase(uint8_t semId)
	{
		SemId = semId;
	}
private:
	uint8_t SemId = 0;
	
	hwHSEMBase()
	{
		if ((int)this >= 0x38800000 && (int)this <= 0x38800FFF) // backup ram
		{
			// ! we are in uinitialized RAM, so init all class var
			ProcessIdCM7 = ProcessIdCM4 = FREE_PROCESS_ID; 
			ChangedCM7 = ChangedCM4 = false;
			NestedLockCM7 = NestedLockCM4 = 0;
		}
		if ((int)this >= 0x38000000 && (int)this <= 0x3800FFFF) // Shared ram
		{
		}
	}

	uint32_t ProcessIdCM7 = FREE_PROCESS_ID, ProcessIdCM4 = FREE_PROCESS_ID;
	bool ChangedCM7 = false, ChangedCM4 = false;
	uint32_t NestedLockCM7 = 0, NestedLockCM4 = 0;
	
public:
	void Changed()
	{
		assert(PROCESS_ID != FREE_PROCESS_ID && "Must be taken");
		CHANGED = true;
	}
	
	bool Take(uint32_t ProcessID)
	{
		assert(ProcessID <= HSEM_PROCESSID_MAX);
		if (ProcessID > 0)
		{
			uint32_t mask = (HSEM_R_COREID_CURRENT | (ProcessID << HSEM_R_PROCID_Pos) | HSEM_R_LOCK);
			// First step  write R register with MasterID, processID and take bit = 1
			HSEM->R[SemId] = mask; // 2 step take
			uint32_t lock = HSEM->R[SemId];
			return lock == mask; 
		}
		
		uint32_t mask = (HSEM_RLR_COREID_CURRENT | HSEM_RLR_LOCK);
		// Read the RLR register to take the semaphore
		uint32_t lock = HSEM->RLR[SemId]; // 1 step take
		return lock == mask; 
	}

	void Release(uint32_t ProcessID, bool signal)
	{
		assert(ProcessID <= HSEM_PROCESSID_MAX);
		
		if (signal)
		{
			// enable interrupt for the receiver
#ifdef CORE_CM7
			HSEM->C2IER |= (1 << SemId); // enable interrupt on CM4
#endif
#ifdef CORE_CM4
			HSEM->C1IER |= (1 << SemId); // enable interrupt on CM7
#endif
		}
		
		uint32_t mask = (HSEM_R_COREID_CURRENT | (ProcessID << HSEM_R_PROCID_Pos));
		// Clear the semaphore by writing to the R register : the MasterID, the processID and take bit = 0
		HSEM->R[SemId] = mask; // release
//		assert(HSEM->R[SemID] == 0);  
	}
	
	// locked by any CPU
	bool IsTakenAny()
	{
		assert(SemId <= HSEM_SEMID_MAX);
		uint32_t mask = HSEM_R_LOCK;
		return (HSEM->R[SemId] & HSEM_R_LOCK_Msk) == mask;
	}

	// locked by other CPU
	bool IsTakenOther()
	{
		assert(SemId <= HSEM_SEMID_MAX);
		uint32_t mask = (HSEM_R_COREID_OTHER | HSEM_R_LOCK);
		return (HSEM->R[SemId] & (HSEM_R_COREID_Msk | HSEM_R_LOCK_Msk)) == mask;
	}

	// locked by this CPU
	bool IsTakenThis()
	{
		assert(SemId <= HSEM_SEMID_MAX);
		uint32_t mask = (HSEM_R_COREID_CURRENT | HSEM_R_LOCK);
		return (HSEM->R[SemId] & (HSEM_R_COREID_Msk | HSEM_R_LOCK_Msk)) == mask;
	}

};

// inherite this in your shared object
template<int8_t SEM_ID>
	class hwHSEM : public hwHSEMBase
	{
	public:
		static_assert(SEM_ID <= HSEM_SEMID_MAX);
		static_assert(SEM_ID >= 0);
		hwHSEM()
			: hwHSEMBase(SEM_ID) {}
	};

// inherite this in your class that need access to a shared data object notification
class hwHSEMControl
{
public:
	// register shared data to receive Modified Notified events
	// format: num, address of shared objects, ...
	hwHSEMControl(int num = 0, ...)
	{
		va_list args;
		va_start(args, num);
		for (int i = 0; i < num; i++)
		{
			hwHSEMBase *pdmb = va_arg(args, hwHSEMBase *);			

			assert(pdmb != nullptr);
			assert(pdmb->SemId <= HSEM_SEMID_MAX);
			assert(pdmb->SemId > 0);
			// TODO extend to allow serveal objects to register on the same HSEM
			assert(HSEMCallBacks[pdmb->SemId].Data == nullptr);
			assert(HSEMCallBacks[pdmb->SemId].Control == nullptr);
			HSEMCallBacks[pdmb->SemId] = { pdmb, this }; // for the interrupt handler to find object
		}
		va_end(args); 
	}
	
	virtual void ModifiedNotified(hwHSEMBase *) {}
		
	static void Init()
	{
		// Enable HSEM
		RCC->AHB4ENR |= RCC_AHB4ENR_HSEMEN;
		// enable interrupt HSEM 
#ifdef CORE_CM7
		NVIC_EnableIRQ(HSEM1_IRQn); // we use interrupt line HEMS1, from CM4 to CM7
#endif
#ifdef CORE_CM4
		NVIC_EnableIRQ(HSEM2_IRQn); // we use interrupt line HEMS2, from CM7 to CM4
#endif
	}

	struct HSEMCallBack
	{
		hwHSEMBase *Data;
		hwHSEMControl *Control;
	};

	// array to hold callback for interrupts
	static HSEMCallBack HSEMCallBacks[HSEM_SEMID_MAX + 1];

};
