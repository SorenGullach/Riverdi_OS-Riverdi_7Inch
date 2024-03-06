#pragma once

#ifdef DEBUG
#include <stdarg.h>
#include <stdio.h>
void Printf(const char *format, ...);
#else
void Printf(const char *, ...) {}
#endif

#define ModifyReg(reg,	clear,	set) (reg = ((reg & ~(clear)) | (set)))