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
    DbgSerial::GetInstance().Initialize();
    sei();
    uint32_t    l = 0;

    /* Replace with your application code */
    while (1)
    {
        DbgSerial::GetInstance().Send(l++);
        DbgSerial::GetInstance().Send(" Lofaszbingo\r\n");
        DbgSerial::GetInstance().Send(F("Flash Lofaszbingo\r\n"));
        _delay_ms(100);
    }
}

