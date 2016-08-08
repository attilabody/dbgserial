/*
 * FriendISR.cpp
 *
 * Created: 7/26/2016 8:17:15 PM
 * Author : compi
 */
#define F_CPU 16000000UL
#include <util/delay.h>
#include <avr/interrupt.h>
#include "DbgSerial.h"
////////////////////////////////////////////////////////////////////
int main(void)
{
	#ifdef DEBUG_SERIAL
	DbgSerial::GetInstance().Initialize();
	#endif
	sei();
	uint32_t    l = 0;

	/* Replace with your application code */
	while (1)
	{
		DPS(l++);
		DPS(" Lofaszbingo ");
		DPS(F("Flash "));
		DPS((uint16_t)0, true);
		_delay_ms(100);
	}
}

