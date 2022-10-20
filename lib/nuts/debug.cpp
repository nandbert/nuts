/** 
 *
 *
 *  \file      debug.cpp
 *  \author    Norbert Stoeffler
 *  \date      2020-02-02
 */
#ifdef ARDUINO
#include <Arduino.h>
#include <stdarg.h>

extern "C" {

int nuts_printf(const char *format, ...)
{
	int			r=0;
	va_list		args;
	static char buffer[128];

	va_start(args, format);
	vsnprintf(buffer,sizeof(buffer),format, args);
#ifdef ARDUINO_ARCH_STM32F1
	Serial1.print(buffer);
#else
	Serial.print(buffer);
#endif
	va_end(args);

	return r;
}

}

#endif
