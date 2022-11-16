#ifndef _StdAlarm_h
#define _StdAlarm_h

/* standard network alarm bits */
/* airspace specific alarms use low byte only */
#define AL_SENSORFAIL 		0x01				/* single sensor failure */
#define AL_SENSORVAR			0x02				/* sensor variance too great */
#define AL_UNDERTEMP			0x04				/* temperature below limits */
#define AL_OVERTEMP 			0x08				/* temperature over limits */
#define AL_NOSENSORS			0x10				/* all sensors in airspace failed */
#define AL_TRIP 					0x20				/* trip alarm */
#define AL_FEED 					0x40				/* feed under/over-run */
#define AL_WATER					0x80				/* water under/over-run */

/* general system alarms */
#define AL_MAINSFAIL			0x0100			/* mains failure (for battery-backed) */
#define AL_OUTDEV 				0x0200			/* output device control failure */
#define AL_OVERRIDE 			0x0400			/* automatic control overriden */
#define AL_SYSERROR       0x0800      /* system/configuration (i/o) error */
#define AL_OUTCHAN				0x1000			/* output channel timeout */
#define AL_BATTLOW				0x2000			/* battery low */
#define AL_LANFAIL				0x4000			/* loss of lan comms */
#define AL_RESET					0x8000			/* reset */

#endif

