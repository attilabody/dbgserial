/*
* DbgSerial.cpp
*
* Created: 7/26/2016 10:20:44 AM
* Author: compi
*/
#ifdef DEBUG_SERIAL

#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <util/atomic.h>
#include <stdlib.h>
#include <string.h>
#include "DbgSerial.h"

#if defined(DBGSERIAL_USART0)

#define _UCSRA UCSR0A
#define _UCSRB UCSR0B
#define _UCSRC UCSR0C
#define _UBRR UBRR0
#define _UDR UDR0
#define _U2X U2X0
#define _UCSZ0 UCSZ00
#define _UCSZ1 UCSZ01
#define _UCSZ2 UCSZ02
#define _RXEN RXEN0
#define _TXEN TXEN0
#define _RXCIE RXCIE0
#define _UDRIE UDRIE0

#elif defined(DBGSERIAL_USART1)

#define _UCSRA UCSR1A
#define _UCSRB UCSR1B
#define _UCSRC UCSR1C
#define _UBRR UBRR1
#define _UDR UDR1
#define _U2X U2X1
#define _UCSZ0 UCSZ10
#define _UCSZ1 UCSZ11
#define _UCSZ2 UCSZ12
#define _RXEN RXEN1
#define _TXEN TXEN1
#define _RXCIE RXCIE1
#define _UCSRB UCSR1B
#define _UDRIE UDRIE1

#elif defined(DBGSERIAL_USART2)

#define _UCSRA UCSR2A
#define _UCSRB UCSR2B
#define _UCSRC UCSR2C
#define _UBRR UBRR2
#define _UDR UDR2
#define _U2X U2X2
#define _UCSZ0 UCSZ20
#define _UCSZ1 UCSZ21
#define _UCSZ2 UCSZ22
#define _RXEN RXEN2
#define _TXEN TXEN2
#define _RXCIE RXCIE2
#define _UCSRB UCSR2B
#define _UDRIE UDRIE2

#elif defined(DBGSERIAL_USART3)

#define _UCSRA UCSR3A
#define _UCSRB UCSR3B
#define _UCSRC UCSR3C
#define _UBRR UBRR3
#define _UDR UDR3
#define _U2X U2X3
#define _UCSZ0 UCSZ30
#define _UCSZ1 UCSZ31
#define _UCSZ2 UCSZ32
#define _RXEN RXEN3
#define _TXEN TXEN3
#define _RXCIE RXCIE3
#define _UCSRB UCSR3B
#define _UDRIE UDRIE3

#endif

DbgSerial DbgSerial::m_instance;

////////////////////////////////////////////////////////////////////
void DbgSerial::Initialize(bool block)
{
	m_block = block;

	unsigned int prescaler = (F_CPU / 4 / BAUDRATE - 1) / 2;
	_UCSRA = _BV(_U2X);

	if(((F_CPU == 16000000UL) && (BAUDRATE == 57600)) || (prescaler > 4095)) {
		_UCSRA = 0;
		prescaler = (F_CPU / 8 / BAUDRATE - 1) / 2;
	}

	_UBRR = prescaler;

	_UCSRC = _BV(_UCSZ1) | _BV(_UCSZ0);             // N81
	_UCSRB = /*_BV(_RXEN) |*/ _BV(_TXEN) | _BV(_RXCIE); // Enable RX and TX
}

////////////////////////////////////////////////////////////////////
void DbgSerial::TxDataRegisterEmpty()
{
	if(m_txCount) {
		_UDR = m_txBuffer[m_txStart++];

		if(m_txStart == SERIAL_TX_BUFFER_SIZE) {
			m_txStart = 0;
		}

		--m_txCount;
	} else {
		_UCSRB &= ~_BV(_UDRIE);
	}

}

////////////////////////////////////////////////////////////////////
size_t DbgSerial::FillTxBuffer(char *buffer, size_t count)
{
	size_t  copied = 0;

	while(m_txCount < sizeof(m_txBuffer) && count--) {
		uint16_t pos = m_txStart + m_txCount++;

		if(pos >= SERIAL_TX_BUFFER_SIZE) {
			pos -= SERIAL_TX_BUFFER_SIZE;
		}
		m_txBuffer[pos] = *buffer++;
		++copied;
	}

	return copied;
}


////////////////////////////////////////////////////////////////////
size_t  DbgSerial::Send(void *buffer, size_t count)
{
	size_t  sent = 0, copied;

	while(count) {
		while(m_txCount == sizeof(m_txBuffer))
			if(!m_block)
				return sent;

		ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
			copied = FillTxBuffer((char *)buffer, count);

			if(!(_UCSRB & _BV(_UDRIE)))
				_UCSRB |= _BV(_UDRIE);
		}

		buffer = (unsigned char *)buffer + copied;
		count -= copied;
		sent += copied;
	}

	return sent;
}


////////////////////////////////////////////////////////////////////
size_t  DbgSerial::Send(char c)
{
	while(m_txCount == sizeof(m_txBuffer))
		if(!m_block)
			return 0;

	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		FillTxBuffer(&c, 1);
		if(!(_UCSRB & _BV(_UDRIE)))
			_UCSRB |= _BV(_UDRIE);
	}

	return 1;
}


////////////////////////////////////////////////////////////////////
size_t DbgSerial::Send(bool b)
{
	return Send(b ? '1' : '0');
}


////////////////////////////////////////////////////////////////////
size_t DbgSerial::Send(unsigned long l, bool hex, bool prefix)
{
	char    buffer[13];

	if(hex) {
		if(prefix) {
			buffer[0] = '0';
			buffer[1] = 'x';
		}
		ultoa(l, buffer + (prefix ? 2 : 0), 16);
	} else {
		ultoa(l, buffer, 10);
	}
	return Send(buffer, strlen(buffer));
}


////////////////////////////////////////////////////////////////////
size_t DbgSerial::Send(unsigned int u, bool hex, bool prefix)
{
	char    buffer[8];

	if(hex) {
		if(prefix) {
			buffer[0] = '0';
			buffer[1] = 'x';
		}
		utoa(u, buffer + (prefix ? 2 : 0), 16);
	} else {
		utoa(u, buffer, 10);
	}
	return Send(buffer, strlen(buffer));
}


////////////////////////////////////////////////////////////////////
size_t DbgSerial::Send(const char *str)
{
	return Send((void *)str, strlen(str));
}

////////////////////////////////////////////////////////////////////
size_t DbgSerial::Send(const __FlashStringHelper *ifsh)
{
	size_t sent = 0;
	PGM_P p = reinterpret_cast<PGM_P>(ifsh);
	char c;

	while(true) {
		c = pgm_read_byte(p++);
		if(c == 0 || Send(c) == 0)
			break;
		++sent;
	}
	return sent;
}

////////////////////////////////////////////////////////////////////
#if defined(DBGSERIAL_USART0)
////////////////////////////////////////////////////////////////////
#if defined(USART_UDRE_vect)
ISR(USART_UDRE_vect)
#else
ISR(USART0_UDRE_vect)
#endif
{
	DbgSerial::GetInstance().TxDataRegisterEmpty();
}

////////////////////////////////////////////////////////////////////
#if defined(USART_TX_vect)
ISR(USART_TX_vect) {}
#else
ISR(USART0_TX_vect) {}
#endif
////////////////////////////////////////////////////////////////////
#elif defined(DBGSERIAL_USART1)
////////////////////////////////////////////////////////////////////
ISR(USART1_UDRE_vect)
{
	DbgSerial::GetInstance().TxDataRegisterEmpty();
}

////////////////////////////////////////////////////////////////////
ISR(USART1_TX_vect)
{
}

////////////////////////////////////////////////////////////////////
#elif defined(DBGSERIAL_USART2)
////////////////////////////////////////////////////////////////////
ISR(USART2_UDRE_vect)
{
	DbgSerial::GetInstance().TxDataRegisterEmpty();
}

////////////////////////////////////////////////////////////////////
ISR(USART2_TX_vect)
{
}
////////////////////////////////////////////////////////////////////
#elif defined(DBGSERIAL_USART3)
////////////////////////////////////////////////////////////////////
ISR(USART3_UDRE_vect)
{
	DbgSerial::GetInstance().TxDataRegisterEmpty();
}

////////////////////////////////////////////////////////////////////
ISR(USART2_TX_vect)
{
}
#endif

#endif//	DEBUG_SERIAL