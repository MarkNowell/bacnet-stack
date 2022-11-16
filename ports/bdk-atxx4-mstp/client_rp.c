/**************************************************************************
*
* Copyright (C) 2005 Steve Karg <skarg@users.sourceforge.net>
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
#include <avr/pgmspace.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// #include <signal.h>
// #include <time.h>
#include <stdbool.h>
#include "config.h"
#include "address.h"
#include "bacdef.h"
#include "handlers.h"
#include "client.h"
#include "bacdcode.h"
#include "npdu.h"
#include "apdu.h"
#include "iam.h"
#include "tsm.h"
#include "device.h"
#include "bacfile.h"
#include "datalink.h"
// #include "net.h"
#include "txbuf.h"
#include "dlenv.h"
#include "bacnet.h"
#include "timer.h"
#include "serial.h"


static struct etimer timer ;
static char serBuf[80] ;

static void Read_Properties2( void ) {
    uint32_t device_id = 12 ;
    BACNET_ADDRESS dest ;
    dlmstp_fill_bacnet_address(&dest, device_id) ;
    bool status = false;
    static unsigned property = 0;
    BACNET_APPLICATION_DATA_VALUE oval ;
		uint32_t millisecs = timer_elapsed_time(&timer) ;		// get current millisecond time
		uint32_t secs = millisecs / 1000 ;
		uint8_t chan, level ;

		address_add(device_id, MAX_APDU, &dest) ;

	/** Sends a Read Property request.
	 * @ingroup DSRP
	 *
	 * @param device_id [in] ID of the destination device
	 * @param object_type [in]  Type of the object whose property is to be read.
	 * @param object_instance [in] Instance # of the object to be read.
	 * @param object_property [in] Property to be read, but not ALL, REQUIRED, or OPTIONAL.
	 * @param array_index [in] Optional: if the Property is an array,
	 *   - 0 for the array size
	 *   - 1 to n for individual array members
	 *   - BACNET_ARRAY_ALL (~0) for the full array to be read.
	 * @return invoke id of outgoing message, or 0 if device is not bound or no tsm available
	 */	
	switch (property) {
	case 0: 
	case 1:
		chan = property ;
		status = Send_Read_Property_Request(device_id, OBJECT_ANALOG_INPUT, chan, PROP_PRESENT_VALUE, -1) ; 
		break ;
	/** Sends a Write Property request.
	 * @ingroup DSWP
	 *
	 * @param device_id [in] ID of the destination device
	 * @param object_type [in]  Type of the object whose property is to be written.
	 * @param object_instance [in] Instance # of the object to be written.
	 * @param object_property [in] Property to be written.
	 * @param object_value [in] The value to be written to the property.
	 * @param priority [in] Write priority of 1 (highest) to 16 (lowest)
	 * @param array_index [in] Optional: if the Property is an array,
	 *   - 0 for the array size
	 *   - 1 to n for individual array members
	 *   - BACNET_ARRAY_ALL (~0) for the array value to be ignored (not sent)
	 * @return invoke id of outgoing message, or 0 on failure.
	 */
	case 2: 
	case 3:
		chan = property - 2 ;
		level = secs % 10 < 5 ? 1 : 0 ;
		if (chan & 1) level = !level ;
		oval.context_specific = false ;
		oval.context_tag = 0 ;
		oval.tag = BACNET_APPLICATION_TAG_ENUMERATED ;
		oval.type.Enumerated = level ;		// 0 or 1
		oval.next = NULL ;
		status = Send_Write_Property_Request(device_id, OBJECT_BINARY_OUTPUT, chan, PROP_PRESENT_VALUE, &oval, MESSAGE_PRIORITY_NORMAL, -1) ; 
		break ;
	case 4:
	case 5:
		chan = property - 4 ;
		status = Send_Read_Property_Request(device_id, OBJECT_BINARY_OUTPUT, chan, PROP_PRESENT_VALUE, -1) ; 
		break ;
	default:
		property = 0 ;
	} ;
  if (status)
	  property++;
}

/* MN: this is my modified RP ack handler to decode a simple single-element value. The original code decoded an array but uses calloc() */
static void mn_handler_read_property_ack(
    uint8_t * service_request,
    uint16_t service_len,
    BACNET_ADDRESS * src,
    BACNET_CONFIRMED_SERVICE_ACK_DATA * service_data)
{
	int len = 0;
  BACNET_READ_PROPERTY_DATA rp1data;
  // BACNET_PROPERTY_REFERENCE rp1property;    /* single property */
  BACNET_APPLICATION_DATA_VALUE value ;
  uint8_t *vdata;
  int vlen, tmp ;

  (void) src;
  (void) service_data;        /* we could use these... */
  len = rp_ack_decode_service_request(service_request, service_len, &rp1data);
  if (len <= 0) return ;		// no value properties
	// Transfer to the BACNET_READ_ACCESS_DATA structure and decode the value(s) portion
  // rp1property.propertyIdentifier = rp1data.object_property;
  // rp1property.propertyArrayIndex = rp1data.array_index;
  /* rp_ack_decode_service_request() processing already removed the Opening and Closing '3' Tags. 
		Note: if this is an array, there will be more than one element to decode */
  vdata = rp1data.application_data;
  vlen = rp1data.application_data_len;
  // rp1property.value = &value;
  if (IS_CONTEXT_SPECIFIC(*vdata)) {
  	len = bacapp_decode_context_data(vdata, vlen, &value, rp1data.object_property);
  } else {
    len = bacapp_decode_application_data(vdata, vlen, &value);	// should get the decoded value in value
  }
  if (len < 0) return ;	// len is number octets consumed by decoding above

	// if decode was successful, should have something sensible in value here
	vlen = 0 ;
	vlen += sprintf(serBuf + vlen, "Src:%d/%d Inv:%d", src->mac_len, src->mac[0], service_data->invoke_id) ;
	vlen += sprintf(serBuf + vlen, " Ty:%d Ins:%ld Pr:%ld Idx:%ld Tag:%d V:", 
		rp1data.object_type, rp1data.object_instance, rp1data.object_property, rp1data.array_index, value.tag) ;
	switch (value.tag) {
  case BACNET_APPLICATION_TAG_NULL: 
		vlen += sprintf(serBuf + vlen, "null") ;
    break;
  case BACNET_APPLICATION_TAG_BOOLEAN:
		vlen += sprintf(serBuf + vlen, "Bool:%d", value.type.Boolean) ;
      break;
  case BACNET_APPLICATION_TAG_UNSIGNED_INT:
		vlen += sprintf(serBuf + vlen, "U32:%lu", value.type.Unsigned_Int) ;
      break;
  case BACNET_APPLICATION_TAG_SIGNED_INT:		// support can be added
		vlen += sprintf(serBuf + vlen, "I32:???") ;
      break;
  case BACNET_APPLICATION_TAG_REAL:
		tmp = (int32_t) (value.type.Real * 10.0) ;
		vlen += sprintf(serBuf + vlen, "Real:%d", tmp) ;
		// vlen += sprintf(serBuf + vlen, "Real:%f", (double) value.type.Real) ;
      break;
  case BACNET_APPLICATION_TAG_DOUBLE:				// support can be added
		vlen += sprintf(serBuf + vlen, "Dbl:???") ;
      break;
  case BACNET_APPLICATION_TAG_OCTET_STRING:
  case BACNET_APPLICATION_TAG_CHARACTER_STRING:
  case BACNET_APPLICATION_TAG_BIT_STRING:
		vlen += sprintf(serBuf + vlen, "Str:???") ;
      break;
  case BACNET_APPLICATION_TAG_ENUMERATED:
		vlen += sprintf(serBuf + vlen, "Enum:%lu", value.type.Enumerated) ;
      break;
  case BACNET_APPLICATION_TAG_DATE:
		vlen += sprintf(serBuf + vlen, "Date") ;
      break;
  case BACNET_APPLICATION_TAG_TIME:
		vlen += sprintf(serBuf + vlen, "Time") ;
      break;
  case BACNET_APPLICATION_TAG_OBJECT_ID:
		vlen += sprintf(serBuf + vlen, "Obj") ;
		break ;
	default:
		vlen += sprintf(serBuf + vlen, "???") ;
	} ;
	vlen += sprintf(serBuf + vlen, "\r\n") ;
	serial_bytes_send((uint8_t *) serBuf, vlen) ;
}


int ClientTask( void ) {
	Read_Properties2(); 
  return 0;
}


void ClientInit( void ) {
	
	apdu_set_confirmed_ack_handler(SERVICE_CONFIRMED_READ_PROPERTY, mn_handler_read_property_ack);
	timer_elapsed_start(&timer) ;	// timer is a 32 bit millisecond timer
}


