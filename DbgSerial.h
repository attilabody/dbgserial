/*
* DbgSerial.h
*
* Created: 7/26/2016 10:20:45 AM
* Author: compi
*/
#pragma once

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

#include <stdint.h>
#include <stddef.h>

#ifdef USART_UDRE_vect
extern "C" void USART_UDRE_vect(void) __attribute__((signal));
#else
extern "C" void USART0_UDRE_vect(void) __attribute__((signal));
#endif
extern "C" void USART1_UDRE_vect(void) __attribute__((signal));
extern "C" void USART2_UDRE_vect(void) __attribute__((signal));
extern "C" void USART3_UDRE_vect(void) __attribute__((signal));

#ifdef DEBUG_SERIAL
class __FlashStringHelper;
#define F(string_literal) (reinterpret_cast<const __FlashStringHelper *>(PSTR(string_literal)))
#define DBGPRINTLN(str) DbgSerial::GetInstance().Send(str "\n")
#define DBGPRINT(str) DbgSerial::GetInstance().Send(str)
#define DPL(str) DbgSerial::GetInstance().Send(F(str "\n"))
#define DP(str) DbgSerial::GetInstance().Send(F(str))
#define DPS(...) DbgSerial::GetInstance().Send(__VA_ARGS__)
#else
#define F(string_literal)
#define DBGPRINTLN(str)
#define DBGPRINT(str)
#define DPL(str)
#define DP(str)
#define DPS(...)
#endif

#ifdef DEBUG_SERIAL
class DbgSerial final
{
		//variables
	public:
	protected:
	private:
		static const uint16_t SERIAL_TX_BUFFER_SIZE = 128;
		static const uint32_t BAUDRATE = 230400;//115200;

		volatile char m_txBuffer[SERIAL_TX_BUFFER_SIZE];
		volatile uint16_t m_txStart = 0;
		volatile uint16_t m_txCount = 0;

		bool m_block = true;

		static DbgSerial m_instance;

		//functions
	public:
		DbgSerial(const DbgSerial &) = delete;
		DbgSerial(DbgSerial &&) = delete;
		DbgSerial &operator=(const DbgSerial &) = delete;

		void Initialize(bool block = true);
		static DbgSerial &GetInstance()
		{
			return m_instance;
		}
		size_t Send(void *buffer, size_t count);
		size_t Send(char c);
		size_t Send(bool b);
		size_t Send(unsigned long l, bool hex = false, bool prefix = true);
		size_t Send(unsigned int i, bool hex = false, bool prefix = true);
		size_t Send(const char *str);
		size_t Send(const __FlashStringHelper *ifsh);

		void SetBlocking(bool block)
		{
			m_block = block;
		}

		void TxDataRegisterEmpty();	//module should be able to work with interrupts disabled

	protected:
	private:
		DbgSerial() = default;
		size_t FillTxBuffer(char *buffer, size_t count);

#ifdef USART_UDRE_vect
		friend void USART_UDRE_vect(void);
#else
		friend void USART0_UDRE_vect(void);
#endif
		friend void USART1_UDRE_vect(void);
		friend void USART2_UDRE_vect(void);
		friend void USART3_UDRE_vect(void);
}; //DbgSerial

#endif	//	DEBUG_SERIAL
