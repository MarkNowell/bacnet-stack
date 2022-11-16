
#ifndef _SlanLAN1_h
#define _SlanLAN1_h

// #include <avr/eeprom.h>
// #include <avr/wdt.h>
#include "eeprom.h"
#include "mn_defs.h"
#include "nvdata.h"		// includes mix of bacnet and slan eeprom locations
#include "lan.h"

#define SLANLAN_APPID	0x030b

#define ID_READERR  	-0x7fff

// #include "SlanLAN1a.h"
#include "StdAlarm.h"

#define NUM_IOCHANS		10				// total allows for 8xADC/I/O chans plus 2 aux-I/O chans, in that order
#define NUM_ADCCHANS	8					// ADC chans map into Chans 0-7
#define NUM_DIGCHANS	2					// Digital chans map into Chans 8-15

#define E2Read(addr)							ee_read_byte((word) addr)
#define E2ReadWord(addr)					ee_read_word((word) addr)
#define E2Write(addr, val)				ee_write_byte((word) addr, val)
#define E2WriteWord(addr, val)		ee_write_word((word) addr, val)
#define Sb( st )	((byte) ST_ ## st)
#define Sw( st )	((word) ST_ ## st)
#define St( st ) 	(E2Read(Sb(st)))
#define Stw( st ) 	(E2ReadWord(Sw(st)))

// *********** System and Room data
word alarmFlags ;
#define dicamID 				SLANLAN_APPID
#define ClearAlarms()		{ alarmFlags = 0 ; }

extern byte lanCycle ;
extern wordByte iTmp ;

int MulDiv( int a, int b, int d ) ;
word MulDivU( word a, word b, word d ) ;

// ************ I/O defs and routines ************
byte IOResetCheck( void ) ;
word IOReading( byte chan ) ;
void OCSet( byte chan, byte level ) ;
void OCSetAll( word onMask ) ;
byte OCGet( byte chan ) ;
void OCSetDefault( byte chan ) ;
void OCSetLocal( byte chan, byte level ) ; 	// implemented in lant.c - clears timeout for local control
byte IOCDigital( byte chan ) ;		// true/false on digital level


/******************* DEVICE ACCESS METHODS ***********************/
#define DevAddr( u, c ) 		((byte) (u) << 3 | (byte) (c))
#define DevChan( addr )     ((byte) (addr) & 0x07)
#define DevUnit( addr )     ((byte) (addr) >> 3)
#define DevNull( addr )     ((byte) (addr) == DEV_NULL)

#define OCType( chan )			2		// on/off
#define OCLevel( chan )			OCGet(chan)
#define ICActive( chan )		(St(ICACTIVE) & (0x01 << chan))
#define ICRead( chan )			ADCRaw(chan)

// Tasks and initialisation
void SlanInit( void ) ;
void SlanUpdate( void ) ;

// BACnet client interface

void BACMap( TLanBuf *lanMsg ) ;			// Add a LAN received message to the BACnet mapping circular buffer

typedef struct {
	word
		rd,					// read property requests sent 
		rdok,				// RP ok 
		rdfail,			// RP fail
		rdtimeout,	// RP ack timeout
		wr,					// write property requests sent 
		wrok,				// WP ok 
		wrfail,			// WP fail
		wrtimeout,	// WP ack timeout
		stalled,		// FSM timed-out waiting for transition
		bmStack ;		// max number messages in BAC request stack
} TBACStats ;

extern TBACStats bst ;

#endif	// _SlanLAN1_h
