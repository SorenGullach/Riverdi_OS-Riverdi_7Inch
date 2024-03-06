
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
