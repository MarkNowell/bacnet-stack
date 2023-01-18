/************************************************************************
*
* Copyright (C) 2009 Steve Karg <skarg@users.sourceforge.net>
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
*************************************************************************/

#include <stdbool.h>
#include <stdint.h>
#include "hardware.h"
#include "init.h"
#include "stack.h"
#include "timer.h"
#include "input.h"
#include "led.h"
#include "adc.h"
#include "nvdata.h"
#include "timer.h"
#include "rs485.h"
#include "serial.h"
#include "bacnet.h"
#include "test.h"
#include "watchdog.h"
#include "version.h"

#include "dtl\SlanLAN1.h"

/* global - currently the version of the stack */
char *BACnet_Version = BACNET_VERSION_TEXT;

/* For porting to IAR, see:
   http://www.avrfreaks.net/wiki/index.php/Documentation:AVR_GCC/IarToAvrgcc*/

// Following fuses add WDTON and 2.7V Brown-out to defaults
FUSES = { .low = 0xFF, .high = 0xC8, .extended = 0xFD };

static void InitIO( void ) {
	// PORTA unused
	DDRA = 0 ; PORTA = 0xff ;
	// PORTB: Arduino yellow LED on PB7, Diag LEDs on PB5/6 otherwise unused
	DDRB = 0xe0 ; PORTB = 0x1f ;	// LEDs on
	// PORTC: unused
	DDRC = 0 ; PORTC = 0xff ;
	// PORTD				0=NC(I^),		1=NC(I^),			2=RXD1(I^),	3=TXD1(O^),	4=NC(I^) 		5=NC(I^),		6=NC(I^),		7=NC(I^),
	PORTD = (byte) (0x01	 			| 0x02	 			| 0x04    	| 0x08 			| 0x10 			| 0x20  		| 0x40 			| 0x80 ) ;
	DDRD = (byte)  (0     			| 0		 				| 0 				| 0x08 			| 0		 			| 0		  		| 0		 			| 0    ) ;
	// PORTE				0=RXD0(I^)	1=TXD0(O^)		2=NC(I^)		3=DE3(I^), 	4=SlRx2(I^)	5=BARx2(I^) 6=NC(I^),		7=NC(I^),
	PORTE = (byte) (0x01	 			| 0x02	 			| 0x04    	| 0x08 			| 0x10 			| 0x20  		| 0x40 			| 0x80 ) ;
	DDRE = (byte)  (0     			| 0x02 				| 0 				| 0x0 			| 0		 			| 0		  		| 0		 			| 0    ) ;
	// PORTF: unused/Jtag
	DDRF = 0 ; PORTF = 0xff ;
	// PORTG: unused (missing LEDs on PG0/1)
	DDRG |= 0 ; PORTF |= 0xff ;
	// PORTH				0=RXD2(I^)	1=TXD2(O^)		2=NC(I^) 		3=BaDE(O)		4=BaRE(O^),	5=SlDE(O)		6=SlRE(O^) 	7=NC(I^)
	PORTH = (byte) (0x01	 			| 0x02	 			| 0x04    	| 0		 			| 0x10 			| 0		  		| 0x40 			| 0x80 ) ;
	DDRH = (byte)  (0   				| 0x02 				| 0					| 0x08 			| 0x10 			| 0x20  		| 0x40 			| 0    ) ;
	// PORTJ				0=RXD3(I^)	1=TXD3(O^)		2=NC(I^) 		3=NC(I^)		4=NC(I^),		5=NC(I^)		6=NC(I^) 		7=NC(I^)
	PORTJ = (byte) (0x01	 			| 0x02	 			| 0x04    	| 0x08 			| 0x10 			| 0x20  		| 0x40 			| 0x80 ) ;
	DDRJ = (byte)  (0   				| 0x02 				| 0					| 0 				| 0		 			| 0		  		| 0		 			| 0    ) ;
	// PORTK: unused
	DDRK = 0 ; PORTK = 0xff ;
	// PORTL: unused
	DDRL = 0 ; PORTL = 0xff ;
}

int main(
    void)
{
    init();
    /* Configure the watchdog timer - Disabled for debugging */
#ifdef NDEBUG
    watchdog_init(2000);
#else
    watchdog_init(0);
#endif
		InitIO() ;
    timer_init();
#ifndef MN_CLIENT
    adc_init();
#endif
    input_init();
    seeprom_init();
    rs485_init();
    serial_init();
    led_init();
    bacnet_init();
    test_init();
#ifdef MN_CLIENT
		SlanInit() ;
		ClientInit() ;
#endif
    /* Enable global interrupts */
    __enable_interrupt();
    for (;;) {
        watchdog_reset();
        input_task();
        bacnet_task();
			#ifdef MN_CLIENT
				SlanUpdate() ;
				ClientTask() ;
			#endif
        led_task();
        // test_task();
    }
}
