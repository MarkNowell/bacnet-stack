// SlanLAN1a.h - Slan module behaving as LAN Slave I/O 

#ifndef _SlanLAN1a_h
#define _SlanLAN1a_h

#define SLANLAN_APPID	0x030b

#define ID_READERR  	-0x7fff

typedef enum {
	ST_UNUSED = 0, ST_UNUSEDHI,		// app fudges procid when asked for word @ 0

  // Program ID
  ST_APPID, ST_APPIDHI,
	
	// Resets
	ST_RESETS, ST_RESETSHI,

  // System config
	ST_LANADDR,
	ST_LANTXCYCLE,
	ST_TIMEOUT,									// I/O timeout
	ST_ICACTIVE,								// ICActive mask - applies to ADC chans only
	
	// I/O channels etc
	// I/O bit-masks allow for up to 16 chans of configurable I/O
	// Low bytes are Analogue-capable, High bytes digital/output only
	ST_OCMASK, ST_OCMASKHI,					// output chans
	ST_ADCMASK, ST_ADCMASKHI,				// analog chans
	ST_PULSEMASK, ST_PULSEMASKHI,		// pulse chans
	ST_TIMEDMASK, ST_TIMEDMASKHI,		// timed chans
	ST_INVERTMASK, ST_INVERTMASKHI,	// inverted inputs/outputs
	ST_SCALEMASK, ST_SCALEMASKHI,		// (pulse) chans to apply scaling to
	ST_OCDEFAULT, ST_OCDEFAULTHI,		// output default levels

	ST_PULLUP,											// pull-up timing
	ST_PULSESCALE,									// pulse pre-scale (number bits to shift)
	ST_ADCREF,							// ADC reference channel 0-7 (>7 => inactive)
	ST_MAINSDELAY,						// Mains fail delay (secs)

	ST_STARTUPDELAY,					// Reset and mains restart delay on output startup
	ST_OCDELAY,						// Output startup delay for individual chans
	ST_OCDELAYEND=ST_OCDELAY+16-1,

  ST_END
} TStatus ;

#endif	// _SlanLAN1a_h
