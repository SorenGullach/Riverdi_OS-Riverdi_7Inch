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

/* Entry point */
ENTRY(Reset_Handler)

/* Memory layout */
MEMORY
{
	FLASH (RX)    : ORIGIN = 0x08000000, LENGTH = 1M		/* Internal FLASH */
	DTCMRAM (RWX) : ORIGIN = 0x20000000, LENGTH = 128K		/* Internal fast RAM */
	ITCMRAM (RWX) : ORIGIN = 0x00000000, LENGTH = 64K		/* Internal fast RAM */
	RAM_D1 (RWX)  : ORIGIN = 0x24000000, LENGTH = 512K		/* Internal RAM */
	QSPI (rx)	  : ORIGIN = 0x90000000, LENGTH = 128000K	/* External QSPI FLASH */
    SHARED (xrw)  : ORIGIN = 0x38000000, LENGTH = 64K		/* Shared SRAM4 0x38000000 - 0x3800FFFF */
    BKPSRAM (xrw) : ORIGIN = 0x38800000, LENGTH = 4K		/* Backup SRAM4 0x38800000 - 0x38800FFF */
	SDRAM (RWX)	  : ORIGIN = 0xD0000000, LENGTH = 0x800000	/* External SDRAM */
}

/* Heap and stack sizes */
_Min_Heap_Size = 0xD000; /* required amount of heap  */

/* End of stack */
_estack = ORIGIN(DTCMRAM) + LENGTH(DTCMRAM)-16; /* end of "DTCMRAM" Ram type memory */

/* Define sections */
SECTIONS
{
    /* Interrupt vector table */
	.isr_vector :
	{
		. = ALIGN(4);
		KEEP(*(.isr_vector))
		. = ALIGN(4);
	} > FLASH

    /* Text section */
	.text :
	{
		. = ALIGN(4);
		_stext = .;

		*(.text)
		*(.text*)
		*(.rodata)
		*(.rodata*)
		*(.glue_7)
		*(.glue_7t)
		KEEP(*(.init))
		KEEP(*(.fini))
		. = ALIGN(4);
		_etext = .;

	} > FLASH

	.ARM.extab :
	{
		. = ALIGN(4);
		*(.ARM.extab)
		*(.gnu.linkonce.armextab.*)
		. = ALIGN(4);
	} > FLASH

	.exidx :
	{
		. = ALIGN(4);
		PROVIDE(__exidx_start = .);
		*(.ARM.exidx*)
		. = ALIGN(4);
		PROVIDE(__exidx_end = .);
	} > FLASH

    /* ARM attributes */
	.ARM.attributes :
	{
		*(.ARM.attributes)
	} > FLASH

    /* Pre-initialized data */
	.preinit_array :
	{
		PROVIDE(__preinit_array_start = .);
		KEEP(*(.preinit_array*))
		PROVIDE(__preinit_array_end = .);
	} > FLASH

    /* Initialization data */
	.init_array :
	{
		PROVIDE(__init_array_start = .);
		KEEP(*(SORT(.init_array.*)))
		KEEP(*(.init_array*))
		PROVIDE(__init_array_end = .);
	} > FLASH

    /* Finalization data */
	.fini_array :
	{
		PROVIDE(__fini_array_start = .);
		KEEP(*(.fini_array*))
		KEEP(*(SORT(.fini_array.*)))
		PROVIDE(__fini_array_end = .);
	} > FLASH

    /* Initialized data */
	.data :
	{
		. = ALIGN(4);
		_sdata = .;

		PROVIDE(__data_start__ = _sdata);
		*(.data)
		*(.data*)
		. = ALIGN(4);
		_edata = .;

		PROVIDE(__data_end__ = _edata);
	} > DTCMRAM AT >FLASH

	_sidata = LOADADDR(.data);

    /* RAM data */
	.ramdata :
	{
		. = ALIGN(4);
		_sramdata = .;

		*(.ramdata)
		*(.ramdata*)
		. = ALIGN(4);
		_eramdata = .;

	} >RAM_D1

    /* Fast code */
	.fastcode :
	{
		. = ALIGN(4);
		_sfastcode = .;

		*(.fastcode)
		*(.fastcode*)
		. = ALIGN(4);
		_efastcode = .;

	} >ITCMRAM AT> FLASH

	_sifastcode = LOADADDR(.fastcode);

    /* Uninitialized data */
	.bss :
	{
		. = ALIGN(4);
		_sbss = .;

		PROVIDE(__bss_start__ = _sbss);
		*(.bss)
		*(.bss*)
		*(COMMON)
		. = ALIGN(4);
		_ebss = .;

		PROVIDE(__bss_end__ = _ebss);
	} > DTCMRAM

	PROVIDE(end = .);

    /* SDRAM */
	.sdram  (NOLOAD):
	{
		. = ALIGN(4);
		PROVIDE(_ssdram = .);
		KEEP(*(.sdram))
		. = . + LENGTH(SDRAM);
		*(.sdram)
		*(.sdram*)
		. = ALIGN(4);
		PROVIDE(_esdram = .);

	} >SDRAM

    /* Heap */
	.heap (NOLOAD) :
	{
		. = ALIGN(4);
		PROVIDE(__heap_start__ = .);
		KEEP(*(.heap))
		. = . + _Min_Heap_Size;
		. = ALIGN(4);
		PROVIDE(__heap_end__ = .);
		FILL(0xEAEA);
	} > DTCMRAM

	__StackLimit = __heap_end__;
	_sstack = __heap_end__;
	Stack_Size = _estack-__heap_end__; /* required amount of stack */

    /* Reserved space for stack */
	.reserved_for_stack (NOLOAD) :
	{
		. = ALIGN(4);
		KEEP(*(.reserved_for_stack))
		. = . + Stack_Size;
		. = ALIGN(4);
		FILL(0xACAC);
	} > DTCMRAM

}

