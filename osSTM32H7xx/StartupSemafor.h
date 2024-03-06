#pragma once

#ifdef CORE_CM4
void WaitForCM7();
#endif
#ifdef CORE_CM7
void WaitForCM4();
void ReleaseCM4();
#endif

