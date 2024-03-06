
#include <StartupSemafor.h>
#include <MemoryDefs.h>

///////////////////////////////////////////////////////////////////////////////
// Syncronize the start up both CPUs
enum class eStartSemafors { Wait = 0, CM4Waiting = 1, CM4Release = 2, CM4Running = 3 };
__SHARED eStartSemafors StartupSemafor = eStartSemafors::Wait;

#pragma GCC push_options
#pragma GCC optimize ("O0")
#ifdef CORE_CM4
void WaitForCM7()
{
	StartupSemafor = eStartSemafors::CM4Waiting;
	while (StartupSemafor != eStartSemafors::CM4Release) {}
	StartupSemafor = eStartSemafors::CM4Running;
}
#endif

#ifdef CORE_CM7
void WaitForCM4()
{
	while (StartupSemafor != eStartSemafors::CM4Waiting) {}
}
void ReleaseCM4()
{
	StartupSemafor = eStartSemafors::CM4Release;
	while (StartupSemafor != eStartSemafors::CM4Running) {}
}
#endif
#pragma GCC pop_options
///////////////////////////////////////////////////////////////////////////////
