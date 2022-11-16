/**************************************************************************
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
*********************************************************************/
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include "hardware.h"
#include "eeprom.h"
// #include <avr\eeprom.h>

/* Internal EEPROM of the AVR - http://supp.iar.com/Support/?note=45745 */


int eeprom_bytes_read(
    uint16_t eeaddr,    /* EEPROM starting memory address (offset of zero) */
    uint8_t * buf,      /* data to store */
    int len)
{       /* number of bytes of data to read */
		eeaddr += EEPROM_INTERNAL_ADDR ;
		eeprom_read_block(buf, (const void *) eeaddr, len) ;
    return len ;
}

int eeprom_bytes_write(
    uint16_t eeaddr,    /* EEPROM starting memory address */
    uint8_t * buf,      /* data to send */
    int len)
{       /* number of bytes of data */
		eeaddr += EEPROM_INTERNAL_ADDR ;
		eeprom_update_block(buf, (void *) eeaddr, len) ;
    return len ;
}

uint8_t ee_read_byte(uint16_t addr) {
	uint8_t b ;
	eeprom_bytes_read(addr, &b, 1) ;
	return b ;
}
uint16_t ee_read_word(uint16_t addr) {
	uint16_t w ;
	eeprom_bytes_read(addr, (uint8_t *) &w, 2) ;
	return w ;
}
void ee_write_byte(uint16_t addr, uint8_t val) {
	eeprom_bytes_write(addr, &val, 1) ;
}
void ee_write_word(uint16_t addr, uint16_t val) {
	eeprom_bytes_write(addr, (uint8_t *) &val, 2) ;
}


