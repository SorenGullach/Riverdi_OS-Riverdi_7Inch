
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