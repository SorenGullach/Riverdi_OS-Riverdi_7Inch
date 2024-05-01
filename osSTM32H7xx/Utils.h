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
#include <stdint.h>
#include <assert.h>

#ifdef DEBUG
#include <stdarg.h>
#include <stdio.h>
void Printf(const char *format, ...);
#else
void Printf(const char *, ...) {}
#endif

#define ModifyReg(reg,	clear,	set) (reg = ((reg & ~(clear)) | (set)))

#define SetReg(		reg, set	) (reg = ((reg & ~(set##_Msk)) | (set)))
#define ClearReg(	reg, clear	) (reg &= ~(clear##_Msk))

struct __attribute__((packed)) ID_TypeDef
{
	uint32_t Chip_Id; // 0x00
	uint8_t Vaf_NumY :4;
	uint8_t Vaf_NumX : 4;
	uint8_t Lot_Num[7];
};
static_assert(sizeof(ID_TypeDef) == 12, "unexpected size");
extern ID_TypeDef *Chip_Id;

extern uint16_t *Chip_FlashSize;
extern char *Chip_LineIdentifier;
void DumpChipInfo();
void DumpMemInfo();

/*
enum STM32Variant {
	UFBGA169_LQFP176_H7x7 = 0b0010,
	LQFP144_H7x5          = 0b0011,
	LQFP176_H7x5          = 0b0110,
	UFBGA176_H7x5         = 0b0111,
	LQFP208_H7x7          = 0b1001,
	LQFP208_H7x5          = 0b1010
};
*/