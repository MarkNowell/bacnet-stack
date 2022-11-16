/**************************************************************************
*
* Copyright (C) 2007 Steve Karg <skarg@users.sourceforge.net>
*
* Permission is hereby granted, free of charge, to any person obtaining
* a copy of this software and associated documentation files (the
* "Software"), to deal in the Software without restriction, including
* without limitation the rights to use, copy, modify, merge, publish,
* distribute, sublicense, and/or sell copies of the Software, and to
* permit persons to whom the Software is furnished to do so, subject to
* the following conditions:
*
* The above copyright notice and this permission notice shall be included
* in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*
*********************************************************************/
#ifndef HARDWARE_H
#define HARDWARE_H

#if !defined(F_CPU)
    /* The processor clock frequency */
#define F_CPU 16000000UL
#endif


/* AVR-GCC compiler specific configuration */
#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/power.h>

#include "iar2gcc.h"
#include "bits.h"
#include "mn_defs.h"

#define EEPROM_INTERNAL_ADDR		0				// offset for general LAN and BACnet settings 
#define SEEPROM_INTERNAL_ADDR		2048		// map external seeprom to internal eeprom starting at this offset

#define BAUD_485_DEFAULT				38400		// this gets written to EE if not already set (i.e. erased EE)
#define BAUD_SERIAL_DEFAULT			115200

#define BIT_LED0			BDF(PORTB,6)
#define BIT_LED1			BDF(PORTB,5)
#define BIT_LED2			BDF(PORTG,1)
#define BIT_LED3			BDF(PORTG,0)
#define IN_LED0				BDF(PING,5)
#define IN_LED1				BDF(PINE,3)
#define IN_LED2				BDF(PINJ,1)
#define IN_LED3				BDF(PINJ,0)

#define BIT_BAC_DE		BDF(PORTH,3)
#define BIT_BAC_RE		BDF(PORTH,4)
#define BIT_SLAN_DE		BDF(PORTH,5)
#define BIT_SLAN_RE		BDF(PORTH,6)

#define bUCSR1A(pn)		(((volatile UCSRnA*)_SFR_MEM_ADDR(UCSR1A))->_ ## pn)
#define bUCSR2A(pn)		(((volatile UCSRnA*)_SFR_MEM_ADDR(UCSR2A))->_ ## pn)

#define LED_BRX				2		// BACnet rx packet
#define LED_BTX				3		// BACnet tx packet
#define LED_BO_0			0		// Binary Output 0
#define LED_BO_1			1		// Binary Output 1
#define MAX_LEDS 			4

#define ADC_CHANNELS_MAX 				8
#define MAX_ANALOG_INPUTS 	ADC_CHANNELS_MAX

#endif
