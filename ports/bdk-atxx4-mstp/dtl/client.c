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
#include "led.h"

#include "SlanLAN1.h"

TBACStats bst ;

static struct etimer timer ;

void BACReadPropertyReply( int tag, int16_t lanValue ) ;		// call-back: send (fail) result back to LAN processing
void BACReadPropertyError( int tag ) ;												// call-back: send fail result back to LAN processing
void BACWritePropertyAck( int tag ) ;											// call-back: send (fail) result back to LAN processing
void BACWritePropertyError( int tag ) ;												// call-back: send fail result back to LAN processing

int BMObjExp( BACNET_OBJECT_TYPE objType ) ;			// find exponent from ASM/object-mapping table

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
  int vlen, invokeID ;
	int16_t lanValue = 0, exp, scale ;		

  // (void) src;
  // (void) service_data;        /* we could use these... */
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
  if (len < 0) { // len is number octets consumed by decoding above
		BACReadPropertyReply(-1, lanValue) ;		// send (fail) result back to LAN processing
		return ;	
	} ;

	invokeID = service_data->invoke_id ;
	// if decode was successful, should have something sensible in value here
	serLen = sprintf(serBuf, "\r\nRPA:%d Inv:%03d", src->mac[0], invokeID) ;
	serLen += sprintf(serBuf + serLen, " Ty:%d Ins:%ld Pr:%ld Idx:%ld Tag:%d V:", 
		rp1data.object_type, rp1data.object_instance, rp1data.object_property, rp1data.array_index, value.tag) ;

	exp = BMObjExp(rp1data.object_type) ;	// find the fixed exponent defined for this object type
	for ( scale = 1 ; exp ; exp-- ) scale *= 10 ;		// and calculate resultant scale factor

	switch (value.tag) {
  case BACNET_APPLICATION_TAG_NULL: 
		serLen += sprintf(serBuf + serLen, "null") ;
    break;
  case BACNET_APPLICATION_TAG_BOOLEAN:
		lanValue = (int16_t) value.type.Boolean ;
		serLen += sprintf(serBuf + serLen, "Bool:%d", value.type.Boolean) ;
      break;
  case BACNET_APPLICATION_TAG_UNSIGNED_INT:
		lanValue = (int16_t) value.type.Unsigned_Int * scale ;
		serLen += sprintf(serBuf + serLen, "U32:%lu", value.type.Unsigned_Int) ;
      break;
  // case BACNET_APPLICATION_TAG_SIGNED_INT:		// support can be added
	//	lanValue = (int16_t) value.type.Signed_Int ;
	//	serLen += sprintf(serBuf + serLen, "I32:???") ;
  //    break;
  case BACNET_APPLICATION_TAG_REAL:
		lanValue = (int16_t) (value.type.Real * scale) ;	
		serLen += sprintf(serBuf + serLen, "Real:%d", lanValue) ;
		// serLen += sprintf(serBuf + serLen, "Real:%f", (double) value.type.Real) ;
      break;
  case BACNET_APPLICATION_TAG_DOUBLE:				// support can be added
		serLen += sprintf(serBuf + serLen, "Dbl:???") ;
      break;
  case BACNET_APPLICATION_TAG_OCTET_STRING:
  case BACNET_APPLICATION_TAG_CHARACTER_STRING:
  case BACNET_APPLICATION_TAG_BIT_STRING:
		serLen += sprintf(serBuf + serLen, "Str:???") ;
      break;
  case BACNET_APPLICATION_TAG_ENUMERATED:
		lanValue = (int16_t) value.type.Enumerated * scale ;
		serLen += sprintf(serBuf + serLen, "Enum:%lu", value.type.Enumerated) ;
      break;
  case BACNET_APPLICATION_TAG_DATE:
		serLen += sprintf(serBuf + serLen, "Date") ;
      break;
  case BACNET_APPLICATION_TAG_TIME:
		serLen += sprintf(serBuf + serLen, "Time") ;
      break;
  case BACNET_APPLICATION_TAG_OBJECT_ID:
		serLen += sprintf(serBuf + serLen, "Obj") ;
		break ;
	default:
		serLen += sprintf(serBuf + serLen, "???") ;
	} ;
	BACReadPropertyReply(invokeID, lanValue) ;		// send value and result/tag back to LAN processing
	serial_bytes_send((uint8_t *) serBuf, serLen) ;
}

static void mn_handler_RP_error( BACNET_ADDRESS * src, uint8_t invoke_id, BACNET_ERROR_CLASS error_class, BACNET_ERROR_CODE error_code) {
	serLen = sprintf(serBuf, "\r\nRPE:%d Inv:%03d Class:%d Code:%d", src->mac[0], invoke_id, error_class, error_code) ;
	BACReadPropertyError(invoke_id) ;
	serial_bytes_send((uint8_t *) serBuf, serLen) ;
}

static void mn_handler_WP_ack( BACNET_ADDRESS * src, uint8_t invoke_id ) {
	serLen = sprintf(serBuf, "\r\nWPA:%d Inv:%03d", src->mac[0], invoke_id) ;
	BACWritePropertyAck(invoke_id) ;
	serial_bytes_send((uint8_t *) serBuf, serLen) ;
}

static void mn_handler_WP_error( BACNET_ADDRESS * src, uint8_t invoke_id, BACNET_ERROR_CLASS error_class, BACNET_ERROR_CODE error_code) {
	serLen = sprintf(serBuf, "\r\nWPE:%d Inv:%03d Class:%d Code:%d", src->mac[0], invoke_id, error_class, error_code) ;
	BACWritePropertyError(invoke_id) ;
	serial_bytes_send((uint8_t *) serBuf, serLen) ;
}

typedef struct 
{ 
  unsigned char rpAckOk:1; 
  unsigned char rpAckFail:1; 
  unsigned char writeOp:1; 
  unsigned char wpAck:1; 
  unsigned char wpError:1; 
  unsigned char ocLevel:1;		// flags that this is an integer output level so should read/write without applying exponent 
  unsigned char bit6:1; 
  unsigned char bit7:1; 
} TBMFlags ; 
static TBMFlags bmFlags ;

typedef struct {		// bm object map primarily maps ASM airspaces to objects but is also used to apply fixed exponents to object types
	BACNET_OBJECT_TYPE objType ;
	int8_t exp ;
} TBMObjMapEl ;
const TBMObjMapEl bmObjMap[] = {
	{ -1, 0 },				// system ASMs (Airspace 0) don't map to BACnet so this is just a dummy entry 
	{ OBJECT_ANALOG_INPUT,				2 },
	{ OBJECT_ANALOG_OUTPUT,				2 },
	{ OBJECT_ANALOG_VALUE,				2 },
	{ OBJECT_BINARY_INPUT, 				2 },
	{ OBJECT_BINARY_OUTPUT,				2 },
	{ OBJECT_BINARY_VALUE,				2 },
	{ OBJECT_MULTI_STATE_INPUT,		0 },
	{ OBJECT_MULTI_STATE_OUTPUT,	0 },
  { OBJECT_MULTI_STATE_VALUE, 	0 }
} ;
#define NUM_BMOBJMAP		(sizeof(bmObjMap) / sizeof(TBMObjMapEl))
int BMObjExp( BACNET_OBJECT_TYPE objType ) {
	if (bmFlags.ocLevel && objType <= OBJECT_ANALOG_VALUE)		// analog readings translate to/from outchan levels with zero exponent
		return 0 ;
	for ( int i = 1 ; i < NUM_BMOBJMAP ; i++ )
		if (objType == bmObjMap[i].objType) 
			return bmObjMap[i].exp ;
	return 0 ;
} 
static bool BMASObjMap( byte airspace, BACNET_OBJECT_TYPE *objType, int8_t *exp ) {
	if (airspace == 0 || airspace >= NUM_BMOBJMAP) return false ;
	*objType = bmObjMap[airspace].objType ;
	*exp = bmObjMap[airspace].exp ;
	return true ;
} ;

typedef struct {
	int tag ;							// current tag/invoke-id
	BACNET_OBJECT_TYPE objType ;	// current object type
	int16_t objInst ;			// current object instance
	int8_t exp ;					// current exponent to apply
	int16_t rpValue ;			// scaled value returned by RP
	TLanBuf *msg ;				// ptr to current msg being processed on BAC side
	byte lsm, lsv, nAs ; 	// parameters of current LAN request
	int16_t lanValue ;		// value parameter of current LAN request if there is one (scaled, write-op only)
} TBMParams ;
static TBMParams bm ;

// #error Increased NUM_BMBUFS from 20 to 40 - check memory stats on build and possibly increase further 
#define NUM_BMBUFS			40
static TLanBuf bmMsgBuf[NUM_BMBUFS] ;       	// BAC-map msg circular buffer 
static TLanBuf *bmMsg ;												// next available message buffer
static int bmAvail ;													// number messages in above buffer
// static int bmRetries ;	
static struct etimer bmTimer ;

#define LanBMDone()   (bmAvail--)			// remove current message from queue

void BACMapStateMachine( void ) ;
int BACForwardLANRead( void ) ;						// map LAN msg to BAC message, attempt to BAC send, return tag
bool BMReturnLANValue( bool valid ) ;			// complete LAN message with BAC value/novalue and transfer to LAN rq buffer
int BACSendReadProperty( byte objtype, byte instance ) ;		// return tag
// static void BMSysReset( void ) { while (1) ; }					// force AVR reset
int BACMapDeviceRP( byte chan ) ;		// map I/O device to read-property request, return invokeID on success; 0 on busy; -1 on fail
int BACMapASASMRP( byte airspace, byte msg ) ;		// map ASM request to read-property request, return invokeID on success; 0 on busy; -1 on fail
bool BMWriteOpNecessary( void ) ;		// checks RP value against LAN write value
int BACSendWriteOp( void ) ;				// constructs and sends WP using current RP parameters

typedef enum { 
	BM_START = 0,	// start here at reset, shouldn't come back
	BM_IDLE, 
	BM_RPTOKEN,		// waiting for SendRP to return token
	BM_RPACK,			// waiting for read property ack 
	BM_RPACKOK,		// got read ack ok
	BM_RPACKFAIL,	// got error ack
	BM_RPACK_TO,	// timeout waiting for BACnet response 

	BM_WRITEOP,		// ack ok, transfer to write op
	BM_WP,				// write op required, send BAC WP
	BM_WPACK,			// WP sent, wait for ACK (or don't bother)

	BM_FINISHED,	// Finished processing current message

	BM_ERROR,
} TBMState ;
static TBMState bmState ;

const char *bmStateStr[] = { "STA", "IDL", "RPN", "RPA", "RPK", "RPF", "RPT", "WOP", "WPS", "WPA", "FIN", "ERR" } ;

// Timeouts (secs unless specified)
#define BMTO_NOTRANSITION		30	// general catch-all timeout deals with LAN/BAC and unexpected failures
#define BMTO_RPACK					2		// RP ack timeout
#define BMTO_WPACK					1		// WP ack timeout

void ClientInit( void ) {
	apdu_set_confirmed_ack_handler(SERVICE_CONFIRMED_READ_PROPERTY, mn_handler_read_property_ack);
	apdu_set_error_handler(SERVICE_CONFIRMED_READ_PROPERTY, mn_handler_RP_error);
	apdu_set_confirmed_simple_ack_handler(SERVICE_CONFIRMED_WRITE_PROPERTY, mn_handler_WP_ack) ;
	apdu_set_error_handler(SERVICE_CONFIRMED_WRITE_PROPERTY, mn_handler_WP_error) ;
	timer_elapsed_start(&timer) ;	// timer is a 32 bit millisecond timer
	bmState = BM_START ;
}

int ClientTask( void ) {
	BACMapStateMachine() ;
  return 0;
}

//*********************** LAN 2 BACnet message mapping ************************************

void BACMapStateMachine( void ) {
	TBMState lastState = bmState ;
	uint32_t millisecs, secs ;
	while (true) {
		millisecs = timer_elapsed_time(&bmTimer) ;		// get current millisecond time
		secs = millisecs / 1000 ;
		switch (bmState) {
		case BM_START:
			led_on(0) ; led_on(1) ;		// both LEDs on to indicate LAN and BAC timeouts
			bmAvail = 0 ;
			bmMsg = bmMsgBuf ;
			bm.tag = -1 ;
			bmState = BM_IDLE ;
			break ;			
		case BM_IDLE:				// waiting for LAN message
			*((byte *) &bmFlags) = 0 ;
			if (bmAvail) {
				led_off(0) ;		// LED1 off - LAN ok
				bmState = BM_RPTOKEN ;
			} ;
			break ;
		case BM_RPTOKEN:		// LAN message available, keep trying to send until we get an RP token
			if ((bm.tag = BACForwardLANRead()) > 0) { 				// got a valid token: RP message has 'gone' (or it's buffered in the BACnet stack)
				bst.rd++ ;
				bmState = BM_RPACK ;
			} ;
			break ;
		case BM_RPACK:			// BAC RP message has been sent - reply handler will set ack events
			if (bmFlags.rpAckOk) {
				led_off(1) ; 	// LED2 off - BACnet ok
				if (bmFlags.writeOp) 
					bmState = BM_WRITEOP ;
				else
					bmState = BM_RPACKOK ;
				bst.rdok++ ;
			}
			else if (bmFlags.rpAckFail) {
				bmState = BM_RPACKFAIL ;
				bst.rdfail++ ;
			}
			else if (secs > BMTO_RPACK) {
				bmState = BM_RPACK_TO ;
				bst.rdtimeout++ ;
			} ;
			break ;
		case BM_RPACKOK:
			if (BMReturnLANValue(true)) // RP ack handler got a value back - push the resulting message back to LAN rq buffer
				bmState = BM_FINISHED ;
			break ;
		case BM_RPACKFAIL:
			if (BMReturnLANValue(false)) // RP ack handler didn't get a value back - push the resulting error message back to LAN rq buffer
				bmState = BM_FINISHED ;
			break ;
		case BM_RPACK_TO:		// timeout waiting for BACnet response - Retry?
			// for now just discard the LAN message and allow the originator to apply timeout
			bmState = BM_FINISHED ;
			break ;
		
		case BM_WRITEOP:		// initial RP was successful, need to compare value and execute write if necessary
			if (BMWriteOpNecessary())
				bmState = BM_WP ;
			else
				bmState = BM_FINISHED ;
			break ;
		case BM_WP:				// Write op required to set new value - try to send it
			if ((bm.tag = BACSendWriteOp()) > 0) {		// got a valid WP token
				bmState = BM_WPACK ;
				bst.wr++ ;
			} ;
			break ;
		case BM_WPACK:		// waiting for write-op ack
			if (bmFlags.wpAck) { 
				bst.wrok++ ;
				bmState = BM_FINISHED ;
			} 
			else if (bmFlags.wpError) {
				bst.wrfail++ ;
				bmState = BM_FINISHED ;
			} 
			else if (secs > BMTO_WPACK) {
				bst.wrtimeout++ ;
				bmState = BM_FINISHED ;
			} ;
			break ;
		case BM_FINISHED:
			LanBMDone() ;
			bmState = BM_IDLE ;
			break ;

		default:	// shouldn't get here
			bmState = BM_START ;
		} ;
		if (secs > BMTO_NOTRANSITION) {		// catch-all timeout on no state transition - restart FSM
			bmState = BM_START ;
			bst.stalled++ ;
		} ;
		if (bmState == lastState) break ;
		// else - state has changed - restart timer and print debug info
		timer_elapsed_start(&bmTimer) ;		// restart timeout on any state transition
		serLen = sprintf(serBuf, " ->%s", bmStateStr[(int) bmState]) ;
		serial_bytes_send((uint8_t *) serBuf, serLen) ;
		lastState = bmState ;
	} ;
}

static void dumpLanBuf( const char *prefix, TLanBuf *msg, int avail ) {
	serLen = sprintf(serBuf, "\r\n") ;
	serLen += sprintf(serBuf + serLen, prefix) ;
	for ( int i = 0 ; i < sizeof(TLanBuf) ; i++ )
		serLen += sprintf(serBuf + serLen, " %02X", msg->b[i]) ;
	serLen += sprintf(serBuf + serLen, " (%d)", avail) ;
	serial_bytes_send((uint8_t *) serBuf, serLen) ;
}

void BACMap( TLanBuf *aMsg ) {			// Add a LAN received message to the BACnet mapping circular buffer
	dumpLanBuf("BMRx: ", aMsg, bmAvail + 1) ;
	*bmMsg = *aMsg ;
  lanMsg = bmMsg ;                  	// operate on bm buf 
	byte lsm = Lx(LSM) & 0xf0 ;
  Lx(RXADDR) = lsm == LSm(BROADCAST) ? 0xff : Lx(TXADDR) ;	// set the reply address
  Lx(TXADDR) = 0 ;										// zero our lan address
  if (++bmMsg >= bmMsgBuf + NUM_BMBUFS) bmMsg = bmMsgBuf ;
  if (bmAvail < NUM_BMBUFS) bmAvail++ ;
	if (bmAvail > bst.bmStack) bst.bmStack = bmAvail ;
}

static bool BMGetMsg( void ) { 		 // get next full msg from queue 
	if (!bmAvail) return false ;
  if ((lanMsg = bmMsg - bmAvail) < bmMsgBuf) lanMsg += NUM_BMBUFS ;
	bm.msg = lanMsg ;				// save current message
	return true ;
}

int BACForwardLANRead( void ) {
	int tag = -1 ;
	byte lsm ;
	if (!BMGetMsg()) return tag ;
	// Construct BACnet RP or WP from LAN msg
	// Sensors (ICRAW) and outputs (OCLEVEL) map through the L2B device mapping table
	// Airspace ASMs (ASASM) map directly to BACnet messages based on airspace (object) and message-number (instance)
  lsm = Lx(LSM) ;    /* get message category */
  bm.nAs = lsm & 0x0f ;
  lsm &= 0xf0 ;
  bm.lsv = Lx(LSV) ;
	bm.lsm = lsm ;
	// All read/write requests start off as read requests - a write request will only write if the read doesn't match
	bmFlags.ocLevel = false ;
	if (lsm == LSm(SET) || lsm == LSm(SETRET)) {
		lsm = LSm(RETURN) ;
		bmFlags.writeOp = true ;
	} ;
	if (lsm == LSm(RETURN) || lsm == LSm(BROADCAST)) {
		if (bm.lsv == LSv(ICRAW)) {
			tag = BACMapDeviceRP(Lxb(0)) ;
			bm.lanValue = Lxi(1) ;		// capture the value field here for completeness, in case this is a write op
		}
		else if (bm.lsv == LSv(OCLEVEL) || bm.lsv == LSv(OCACTUAL)) {
			tag = BACMapDeviceRP(Lxb(0) + L2B_SENSORS) ;
			bm.lanValue = Lxb(2) ;		// OCLEVEL set level is in D2
			bmFlags.ocLevel = true ;	// flags that this is an integer output level so should read/write without applying exponent
		}
		else if (bm.lsv == LSv(ASASM)) {
			tag = BACMapASASMRP(bm.nAs, Lxb(0)) ;
			bm.lanValue = Lxi(1) ;
		}
	} ;
	if (tag < 0) // invalid request or mapping - discard message
		bmAvail-- ;
	return tag ;
}

int BACMapDeviceRP( byte chan ) {		// map I/O device to read-property request, return invokeID on success; 0 on busy; -1 on fail
	if (chan >= L2B_DEVS) return -1 ;
	bm.objType = St(L2B_OBJ0 + chan * L2B_BYTES) ;
	bm.objInst = St(L2B_INST0 + chan * L2B_BYTES) ;
	if (bm.objType == 0xff) return -1 ;
	return BACSendReadProperty(bm.objType, bm.objInst) ;
}
int BACMapASASMRP( byte airspace, byte msg ) {		// map ASM request to read-property request, return invokeID on success; 0 on busy; -1 on fail
	bm.objInst = msg ;		// ASM msg parameter is instance
	if (BMASObjMap(airspace, &bm.objType, &bm.exp)) 
		return BACSendReadProperty(bm.objType, bm.objInst) ;		
	return -1 ;
}

int BACSendReadProperty( byte objtype, byte instance ) {
  uint32_t device_id = St(SERVERMAC) ;
  BACNET_ADDRESS dest ;
	int tag ;
  dlmstp_fill_bacnet_address(&dest, device_id) ;
	address_add(device_id, MAX_APDU, &dest) ;
	// Send_Read_Property_Request( destination-device-id, object-type, instance, property, array index or -1 ) 
	// returns invoke id of outgoing message, or 0 if device is not bound or no tsm available
	tag = Send_Read_Property_Request(device_id, objtype, instance, PROP_PRESENT_VALUE, -1) ; 
	if (tag) {
		serLen = sprintf(serBuf, "\r\nRP:%ld Obj:%d Ins:%d => %03d", device_id, objtype, instance, tag) ;
		serial_bytes_send((uint8_t *) serBuf, serLen) ;
	} ;
	return tag ;
}

bool BMReturnLANValue( bool valid ) {			// complete LAN message with BAC value/novalue and transfer to LAN rq buffer
	lanMsg = rqMsg ;				// lanMsg -> next free rq buffer
	*lanMsg = *bm.msg ;			// most of our required content is already in bm.msg
	Lx(LSM) = (valid ? LSm(VALUE) : LSm(NOVALUE)) | (Lx(LSM) & 0x0f) ;
	if (bm.lsv == LSv(OCACTUAL))
		Lxb(3) = (byte) bm.rpValue ;	// OCACTUAL unfortunately puts its return level in D3 whereas ...
	else
		Lxi(1) = bm.rpValue ;				// ... other returned values map into D2/3 in the returned packet
  LanAddRq() ;					// update rq buffer
	dumpLanBuf("BMTx: ", lanMsg, rqAvail) ;
	return true ;
}

void BACReadPropertyReply( int tag, int16_t lanValue ) {		// send value and result back to LAN processing
	// arriving here we should have a LAN message at the head of the bmMsgBuf queue awaiting reply, otherwise we just ignore this reply
	if (tag == bm.tag) {
		bmFlags.rpAckOk = true ;
		bm.rpValue = lanValue ;
	}
	else 
		bmFlags.rpAckFail = true ;
}
void BACReadPropertyError( int tag ) {
	bmFlags.rpAckFail = true ;
}

// ************** Write Op handling ***********************

bool BMWriteOpNecessary( void ) {		// called after successful associated RP 
	// check if the (scaled) RP value matches the (scaled) LAN value
	if (bm.rpValue != bm.lanValue) {
		serLen = sprintf(serBuf, "\r\nWP Required, Old:%d New:%d", bm.rpValue, bm.lanValue) ;
		serial_bytes_send((uint8_t *) serBuf, serLen) ;
		return true ;
	} ;
	return false ;
}
int BACSendWriteOp( void ) {				// construct 
  uint32_t device_id = St(SERVERMAC) ;
  BACNET_ADDRESS dest ;
	int tag ;
  BACNET_APPLICATION_DATA_VALUE oval ;

  dlmstp_fill_bacnet_address(&dest, device_id) ;
	address_add(device_id, MAX_APDU, &dest) ;

	// Construct BACnet WP from LAN msg
	// need to construct a suitably scaled BACnet-Value from our current object-type and lan-value
	// this should be the inverse of the value deconstruction in RP ack handler
	oval.context_specific = false ;
	oval.context_tag = 0 ;
	oval.next = NULL ;
	serLen = sprintf(serBuf, "\r\nWP:%ld Obj:%d Ins:%d ", device_id, bm.objType, bm.objInst) ;

	switch (bm.objType) {
	case OBJECT_ANALOG_INPUT:
	case OBJECT_ANALOG_OUTPUT:
	case OBJECT_ANALOG_VALUE:
		// all real and scaled by real factor
		oval.tag = BACNET_APPLICATION_TAG_REAL ;
		oval.type.Real = (float) bm.lanValue ;
		if (!bmFlags.ocLevel) oval.type.Real = oval.type.Real / 100.0 ;
		serLen += sprintf(serBuf + serLen, "Real/%f", (double) oval.type.Real) ;
		break ;

	case OBJECT_BINARY_INPUT:
	case OBJECT_BINARY_OUTPUT:
	case OBJECT_BINARY_VALUE:
		oval.tag = BACNET_APPLICATION_TAG_ENUMERATED ;
		oval.type.Enumerated = bm.lanValue / 100 ;		// binary values scale to 0-100 so ocLevel logic doesn't need to be applied
		serLen += sprintf(serBuf + serLen, "Enum/%ld", oval.type.Enumerated) ;
		break ;

	case OBJECT_MULTI_STATE_INPUT:
	case OBJECT_MULTI_STATE_OUTPUT:
  case OBJECT_MULTI_STATE_VALUE:
	default:
		oval.tag = BACNET_APPLICATION_TAG_ENUMERATED ;
		oval.type.Enumerated = bm.lanValue  ;	
		serLen += sprintf(serBuf + serLen, "EnumU/%ld", oval.type.Enumerated) ;
	} ;
	tag = Send_Write_Property_Request(device_id, bm.objType, bm.objInst, PROP_PRESENT_VALUE, &oval, MESSAGE_PRIORITY_NORMAL, -1) ; 
	if (tag) {
		bm.tag = tag ;
		serLen += sprintf(serBuf + serLen, " => %03d", tag) ;
		serial_bytes_send((uint8_t *) serBuf, serLen) ;
	} ;
	return tag ;
}

void BACWritePropertyAck( int tag ) {											// call-back: send (fail) result back to LAN processing
	bmFlags.wpAck = true ;
}
void BACWritePropertyError( int tag ) {												// call-back: send fail result back to LAN processing
	bmFlags.wpError = true ;
}


#if 0
static void Read_Properties2( void ) {
    uint32_t device_id = St(SERVERMAC) ;
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
#endif
