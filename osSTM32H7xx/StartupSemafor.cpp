
#include <StartupSemafor.h>
#include <MemoryDefs.h>

///////////////////////////////////////////////////////////////////////////////
// Syncronize the start up both CPUs
enum class eStartSemafors { Wait = 0, CM4Waiting = 1, CM4Release = 2, CM4Running = 3 };
__SHARED eStartSemafors StartupSemafor = eStartSemafors::Wait;

#ifdef CORE_CM4
void WaitForCM7()
{
	StartupSemafor = eStartSemafors::CM4Waiting;
	while (StartupSemafor != eStartSemafors::CM4Release) { __asm(""); }
	StartupSemafor = eStartSemafors::CM4Running;
}
#endif

#ifdef CORE_CM7
void WaitForCM4()
{
	while (StartupSemafor != eStartSemafors::CM4Waiting) {__asm(""); }
}
void ReleaseCM4()
{
	StartupSemafor = eStartSemafors::CM4Release;
	while (StartupSemafor != eStartSemafors::CM4Running) {__asm(""); }
}
#endif
///////////////////////////////////////////////////////////////////////////////
