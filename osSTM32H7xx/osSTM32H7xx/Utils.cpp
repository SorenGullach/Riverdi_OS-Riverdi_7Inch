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

#include <Utils.h>

#ifdef DEBUG
#include <stdarg.h>
#include <stdio.h>
void Printf(const char *format, ...)
{
	va_list args;
	va_start(args, format);
	vprintf(format, args);
	va_end(args); 
}
#endif

ID_TypeDef *Chip_Id = (ID_TypeDef *)UID_BASE;
uint16_t *Chip_FlashSize = (uint16_t *)FLASHSIZE_BASE;
char *Chip_LineIdentifier = (char *)(0x1FF1E8C0);

#include <inttypes.h>
void DumpChipInfo()
{
	Printf("Chip id \t\t0x%lX\n", Chip_Id->Chip_Id);
	Printf("Chip Waf num \t\t%d,%d\n", Chip_Id->Vaf_NumX, Chip_Id->Vaf_NumY);
	Printf("Chip Lot num \t\t0x%X%X%X%X%X%X%X\n", 
		Chip_Id->Lot_Num[6], 
		Chip_Id->Lot_Num[5],
		Chip_Id->Lot_Num[4],
		Chip_Id->Lot_Num[3],
		Chip_Id->Lot_Num[2],
		Chip_Id->Lot_Num[1],
		Chip_Id->Lot_Num[0]);
	Printf("Chip FlashSize KBytes \t%ld 0x%lX\n", *Chip_FlashSize, *Chip_FlashSize);
	Printf("Chip Identifier \tSTM32%c%c%c%c\n\n", 
		Chip_LineIdentifier[3], 
		Chip_LineIdentifier[2], 
		Chip_LineIdentifier[1], 
		Chip_LineIdentifier[0]);
}

extern void *_estack, *_sstack, *__StackLimit;
extern void *__heap_start__, *__heap_end__;
extern void *_ssdram,*_esdram;

void DumpMemInfo()
{
	Printf("Heap stack dump\n");
	Printf("Heap %lX to %lX %ld bytes\n", &__heap_start__, &__heap_end__, &__heap_end__ - &__heap_start__); 
	Printf("Stack %lX to %lX %ld bytes\n", &_sstack, &_estack, &_estack - &_sstack);
	Printf("SDRAM %lX to %lX %ld bytes\n", &_ssdram, &_esdram, &_esdram - &_ssdram);
	Printf("Heap stack dump end\n\n");
}
