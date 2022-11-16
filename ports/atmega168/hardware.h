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

#if defined(__IAR_SYSTEMS_ICC__) || defined(__IAR_SYSTEMS_ASM__)
#include <iom168.h>
#else
#if !defined(__AVR_ATmega168__)
#error Firmware is configured for ATmega168 only (-mmcu=atmega168)
#endif
#endif
#include "iar2gcc.h"
#include "avr035.h"

#define PT_LED_NPDU			PORTC
#define PN_LED_NPDU			PC2
#define DD_LED_NPDU			DDRC

#define PT_LED_GREEN		PORTC
#define PN_LED_GREEN		PC3
#define DD_LED_GREEN		DDRC

#define PT_LED_TX				PORTC
#define PN_LED_TX				PC0
#define DD_LED_TX				DDRC

#define PT_LED_RX				PORTC
#define PN_LED_RX				PC1
#define DD_LED_RX				DDRC

#define PT_485_DE				PORTD
#define PN_485_DE				PD4
#define DD_485_DE				DDRD

#define LED_NPDU_INIT() 	BIT_SET(DD_LED_NPDU, PN_LED_NPDU)
#define LED_NPDU_ON() 		BIT_CLEAR(PT_LED_NPDU, PN_LED_NPDU)
#define LED_NPDU_OFF() 		BIT_SET(PT_LED_NPDU, PN_LED_NPDU)

#define LED_GREEN_INIT() 	BIT_SET(DD_LED_GREEN, PN_LED_GREEN)
#define LED_GREEN_ON() 		BIT_CLEAR(PT_LED_GREEN, PN_LED_GREEN)
#define LED_GREEN_OFF() 	BIT_SET(PT_LED_GREEN, PN_LED_GREEN)

#endif
