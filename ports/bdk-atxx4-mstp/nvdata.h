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
#ifndef NVDATA_H
#define NVDATA_H

#include "seeprom.h"
#include "eeprom.h"

#define L2B_SENSORS		8
#define L2B_OUTPUTS		8
#define L2B_DEVS			(L2B_SENSORS + L2B_OUTPUTS)

typedef enum {
	ST_UNUSED = 0, ST_UNUSEDHI,		// app fudges procid when asked for word @ 0

	/*=============== NV_ locations used by BACnet client/server ================*/
	NV_EEPROM_MAC,				// this is the MAC (MSTP) address on the BACnet side 
	NV_EEPROM_BAUD_K, 		// BACnet baud 9=9.6k, 19=19.2k, 38=38.4k, 57=57.6k, 76=76.8k, 115=115.2k 
	NV_EEPROM_MAX_MASTER,	// BACnet max-master
	
	NV_EEPROM_DEVICE_0, 	// device instance is only 22 bits - easier if we use 32 bits 
	NV_EEPROM_DEVICE_1,
	NV_EEPROM_DEVICE_2,
	NV_EEPROM_DEVICE_3,

	/* ============== ST_ locations used for LAN comms ========================== */
  // Program ID
  ST_APPID, ST_APPIDHI,			// @ 9/10
	
	// Resets
	ST_RESETS, ST_RESETSHI,

  // System config
	ST_LANADDR,
	ST_LANTXCYCLE,
	ST_TIMEOUT,									// I/O timeout
	ST_ICACTIVE,								// ICActive mask - applies to ADC chans only
	
	/* =============== BACnet server config ==================================== */
	ST_SERVERMAC,
	
	// LAN-2-BAC device mapping table - maps sensor and output channels to BACnet object/instances
	ST_L2B_OBJ0, ST_L2B_INST0,
#define L2B_BYTES		2
	ST_L2B_END = ST_L2B_OBJ0 + L2B_DEVS * L2B_BYTES - 1,
	
  ST_END
} TStatus ;


/*=================== BACnet strings - allow 1K data starting at 1K offset ==============================*/
/* BACnet Names - 32 bytes of data each */
#define NV_EEPROM_NAME_LENGTH(n) ((n)+0)
#define NV_EEPROM_NAME_ENCODING(n) ((n)+1)
#define NV_EEPROM_NAME_STRING(n) ((n)+2)
#define NV_EEPROM_NAME_SIZE 30
#define NV_EEPROM_NAME_OFFSET (1+1+NV_EEPROM_NAME_SIZE)
/* Device Name - starting offset */
#define NV_EEPROM_DEVICE_NAME 1024
/* Device Description - starting offset  */
#define NV_EEPROM_DEVICE_DESCRIPTION \
    (NV_EEPROM_DEVICE_NAME+NV_EEPROM_NAME_OFFSET)
/* Device Location - starting offset  */
#define NV_EEPROM_DEVICE_LOCATION \
    (NV_EEPROM_DEVICE_DESCRIPTION+NV_EEPROM_NAME_OFFSET)

/*=============== SEEPROM maps to internal EEPROM from 2K ================*/
/* data version - use to check valid version */
#define SEEPROM_ID 0xBAC0
#define SEEPROM_VERSION 0x0001

#define SEEPROM_BYTES_MAX (2*1024)

/* list of SEEPROM addresses */
/* note to developers: define each byte,
   even if they are not used explicitly */
#define NV_SEEPROM_TYPE_0 0
#define NV_SEEPROM_TYPE_1 1
#define NV_SEEPROM_VERSION_0 2
#define NV_SEEPROM_VERSION_1 3

/* SEEPROM free space - 4..31 */

#define NV_SEEPROM_BINARY_OUTPUT_0 32
/* BO properties */
#define NV_SEEPROM_BO_POLARITY 0
#define NV_SEEPROM_BO_OUT_OF_SERVICE 1
#define NV_SEEPROM_BO_PRIORITY_ARRAY_1 2
#define NV_SEEPROM_BO_PRIORITY_ARRAY_2 3
#define NV_SEEPROM_BO_PRIORITY_ARRAY_3 4
#define NV_SEEPROM_BO_PRIORITY_ARRAY_4 5
#define NV_SEEPROM_BO_PRIORITY_ARRAY_5 6
#define NV_SEEPROM_BO_PRIORITY_ARRAY_6 7
#define NV_SEEPROM_BO_PRIORITY_ARRAY_7 8
#define NV_SEEPROM_BO_PRIORITY_ARRAY_8 9
#define NV_SEEPROM_BO_PRIORITY_ARRAY_9 10
#define NV_SEEPROM_BO_PRIORITY_ARRAY_10 11
#define NV_SEEPROM_BO_PRIORITY_ARRAY_11 12
#define NV_SEEPROM_BO_PRIORITY_ARRAY_12 13
#define NV_SEEPROM_BO_PRIORITY_ARRAY_13 14
#define NV_SEEPROM_BO_PRIORITY_ARRAY_14 15
#define NV_SEEPROM_BO_PRIORITY_ARRAY_15 16
#define NV_SEEPROM_BO_PRIORITY_ARRAY_16 17
/* formula for paramters */
#define NV_SEEPROM_BINARY_OUTPUT_SIZE 18
#define NV_SEEPROM_BINARY_OUTPUT(n,p) \
    (NV_SEEPROM_BINARY_OUTPUT_0 + \
    (NV_SEEPROM_BINARY_OUTPUT_SIZE * (n)) + (p))

/* SEEPROM free space - depends on number of BO */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */




#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif
