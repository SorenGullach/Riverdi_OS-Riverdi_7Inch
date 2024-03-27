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