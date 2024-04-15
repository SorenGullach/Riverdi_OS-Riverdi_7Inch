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
