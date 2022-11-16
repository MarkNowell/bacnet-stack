
#include <avr/pgmspace.h>
#include <avr/eeprom.h>
#include "SlanLAN1.h"
#include "lan.h"


void LoadEEDefaults( void ) ;

extern byte txCycle ;
wordByte iTmp ;
  
void SlanInit(void) {
 	if (Stw(APPID) != SLANLAN_APPID) { // || (IOResetCheck() && IOResetCheck())) {
 		LoadEEDefaults() ;
   	E2WriteWord(Sw(APPID), SLANLAN_APPID) ;
	} ;
	// lanAddr = St(LANADDR) ;
	lanAddr = St(LANADDR) ;
	rDevTimeout = St(TIMEOUT) ;
	LanInit() ;
	InitLanTask() ;
	if ((iTmp.w = Stw(RESETS)) < 0xffff) 
	 	E2WriteWord(Sw(RESETS), iTmp.w + 1) ;
}

void SlanUpdate( void ) {
	LanTask() ;
}

void memclr( register byte *p, register int count ) {
	for ( ; count > 0 ; count--, p++) 
		*p = 0 ;
}

int MulDiv( int a, int b, int d ) {
	return (int) ((long) a * b / d) ;
}
word MulDivU( word a, word b, word d ) {
	return (word) ((unsigned long) a * b / d) ;
}

const byte eeDefaults[] PROGMEM = {
	42, 43, 		// unused

	// BACnet basic config
	10, 									// NV_EEPROM_MAC,					// this is the MAC (MSTP) address on the BACnet side 
	38, 									// NV_EEPROM_BAUD_K, 			// BACnet baud 9=9.6k, 19=19.2k, 38=38.4k, 57=57.6k, 76=76.8k, 115=115.2k 
	20, 									// NV_EEPROM_MAX_MASTER,	// BACnet max-master
	
	0xff, 0xff, 0xff, 0xff, 	// NV_EEPROM_DEVICE_0-3 - 32-bit device instance FFFFFFFF will force MAC to be used

	// Slan config
	0, 0,									// App ID gets overwritten once full EE load has completed so set an incorrect value here

	// Resets
	0, 0,
	
  // System config
	10,							// ST_LANADDR,
	3,							// ST_LANTXCYCLE,
	15,							// ST_RDEVTIMEOUT
	0x0,						// ICACTIVE

	// ============ BACnet server config ==================
	18, 						// SERVERMAC
#if 1		// Trane default table
	// LAN-2-BAC device mapping table - maps sensor and output channels to BACnet object/instances
	// LAN-2-BAC device mapping table - maps sensor and output channels to BACnet object/instances
	0, 15,				// Sensor1	AI (0)	15	2	R	Space Air Temp
	0, 50, 				// Sensor2	AV (0)	50	2	R	Space Air Setpoint Active
	3, 165,				// Sensor3	BI (3) 165  0   R   Alarm relay status
	2, 24,				// Sensor4	AV (2)	24	0	R	Warning Alarm	Enumerated
	2, 25,				// Sensor5	AV (2)	25	0	R	Problem Alarm	Enumerated
	2, 13,				// Sensor6	AV (2)	13	2	R/W	Discharge Air Clg Setpoint	39.992-100.0F or 4.44-37.78C
	2, 17, 				// Sensor7	AV (2)	17	2	R/W	Discharge Air Htg Setpoint	39.992-100.0F or 4.44-37.78C
	3, 15, 				// Sensor8	BI (3)  15  0   R   Supply Fan Config
	19, 5, 				// Output1	MSV (19) 5  0	R/W	Application Mode	1=Off, 2=Heat, 3=Cool, 4=FanOnly, 5=Auto
	19, 13,				// Output2	MSV (19)13	0	W	Clear Alarms	1=None, 5=ClrAllAlms
	19, 15,				// Output3	MSV (19)15	0	R	Unit State	1=Off,...8=Clg
	2, 13, 				// Output4	AV  (2) 13  0 R/W Discharge Air Clg Setpoint 0-100F
	0xff, 0xff,			// Output5 - none
	1, 2, 				// Output6	AO (1)	 2      R/W Space Air Temp Set BAS
	1, 11, 				// Output7	AO (1)  11      R/W Space Sensor Temp
	3, 15 				// Output8	BI (3)  15      R/W Supply Fan Config
#else		// Daikin default table (possibly out of date but currently unused, because there are 2 Trane units vs 1 Daikin)
	// LAN-2-BAC device mapping table - maps sensor and output channels to BACnet object/instances
	// LAN-2-BAC device mapping table - maps sensor and output channels to BACnet object/instances
	0, 1,					// Sensor1	AI (0)	1	2	R	Discharge Air Temp	-50.0-249.998F
	2, 39, 				// Sensor2	AV (2)	39	2	R	Effective Discharge Air Setpt	-83.2..+147.2F
	2, 27,				// Sensor3	AV (2)	27	0	R	Alarm Value	Enumerated
	2, 24,				// Sensor4	AV (2)	24	0	R	Warning Alarm	Enumerated
	2, 25,				// Sensor5	AV (2)	25	0	R	Problem Alarm	Enumerated
	2, 13,				// Sensor6	AV (2)	13	2	R/W	Discharge Air Clg Setpoint	39.992-100.0F or 4.44-37.78C
	2, 17, 				// Sensor7	AV (2)	17	2	R/W	Discharge Air Htg Setpoint	39.992-100.0F or 4.44-37.78C
	0xff, 0xff, 	// Sensor8
	19, 5, 				// Output1	MSV (19)	5	  0	R/W	Application Mode	1=Off, 2=Heat, 3=Cool, 4=FanOnly, 5=Auto
	19, 13,				// Output2	MSV (19)	13	0	W	Clear Alarms	1=None, 5=ClrAllAlms
	19, 15,				// Output3	MSV (19)	15	0	R	Unit State	1=Off,...8=Clg
	2, 13, 				// Output4	AV  (2)   13  0 R/W Discharge Air Clg Setpoint 0-100F
	0xff, 0xff, 	// Output5
	0xff, 0xff, 	// Output6
	4, 0, 				// Output7	BO  (4)   0   2 R/W Server test output 0 (0-100% <=> 0/1 binary value)
	4, 1 					// Output8	BO  (4)   1   2 R/W Server test output 1 (0-100% <=> 0/1 binary value)
#endif
} ;

void LoadEEDefaults( void ) {
	word i ;
	byte addr ;
  	for ( addr = Sb(UNUSED), i = 0 ; addr < Sb(END) ; )
  		E2Write(addr++, pgm_read_byte(eeDefaults + i++)) ;
}


