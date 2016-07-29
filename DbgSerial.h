/*
* DbgSerial.h
*
* Created: 7/26/2016 10:20:45 AM
* Author: compi
*/
#pragma once

#include <stdint.h>
#include <avr/io.h>
#include <stddef.h>
#include <avr/pgmspace.h>

#define USE_USART0

#if defined(USE_USART0)
extern "C" void USART0_UDRE_vect(void) __attribute__((signal));
#elif defined(USE_USART1)
extern "C" void USART1_UDRE_vect(void) __attribute__((signal));
#elif defined(USE_USART2)
extern "C" void USART2_UDRE_vect(void) __attribute__((signal));
#endif

class __FlashStringHelper;
#define F(string_literal) (reinterpret_cast<const __FlashStringHelper *>(PSTR(string_literal)))
#define DBGPRINTLN(str) DbgSerial::GetInstance().Send(str "\n")
#define DBGPRINT(str) DbgSerial::GetInstance().Send(str)

class DbgSerial final
{
        //variables
    public:
    protected:
    private:
        static const uint8_t SERIAL_TX_BUFFER_SIZE = 32;
        static const uint32_t BAUDRATE = 115200;

        volatile char m_txBuffer[SERIAL_TX_BUFFER_SIZE];
        volatile uint8_t m_txStart = 0;
        volatile uint8_t m_txCount = 0;

        bool m_block = true;

        static DbgSerial m_instance;

        //functions
    public:
        DbgSerial(const DbgSerial &) = delete;
        DbgSerial(DbgSerial &&) = delete;
        DbgSerial &operator=(const DbgSerial &) = delete;

        void Initialize();
        static DbgSerial &GetInstance()
        {
            return m_instance;
        }
        size_t Send(void* buffer, size_t count);
        size_t Send(char c);
        size_t Send(unsigned long l, bool hex = false);
        size_t Send(unsigned int i, bool hex = false);
        size_t Send(const char* str);
        size_t Send(const __FlashStringHelper* ifsh);

        void SetBlocking(bool block)
        {
            m_block = block;
        }

    protected:
    private:
        DbgSerial() = default;
        void TxDataRegisterEmpty();
        size_t FillTxBuffer(char* buffer, size_t count);

#if defined(USE_USART0)
        friend void USART0_UDRE_vect(void);
#elif defined(USE_USART1)
        friend void USART1_UDRE_vect(void);
#elif defined(USE_USART2)
        friend void USART2_UDRE_vect(void);
#endif
}; //DbgSerial
