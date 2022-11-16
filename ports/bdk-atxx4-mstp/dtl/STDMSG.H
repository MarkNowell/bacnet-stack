/* STDMSG.H */

#ifndef _STDMSG_H
#define _STDMSG_H

/* standard message data elements */
typedef enum {
  LSV_NULL=0,       /* no value content */
  LSV_MEMBYTE,      /* word addr, byte value */
  LSV_MEMWORD,      /* word addr, word value */
  LSV_EEBYTE,       /* word addr, byte value */
  LSV_EEWORD,       /* word addr, word value */
  LSV_OCLEVEL,      /* byte outchan, byte type, byte level (percent) */
  LSV_ICLEVEL,      /* byte inchan, byte type, int level */
  LSV_DICAMID,      /* word dicamid */
  LSV_APPTYPE,      /* word application-type AT_XXX */
  LSV_ALARMS,       /* word system alarmFlags */
  LSV_DATETIME,     /* word day-number, byte hour, byte minute */
  LSV_TEMPOFFSET,   /* int temp offset */
  LSV_CURVEPOINT,   /* byte point num, byte day, byte set temp, byte min vent */
  LSV_ASCONFIG,     /* byte airspaces, byte rooms, byte zones, byte creeps */
  LSV_ICRAW,        /* byte inchan, byte devAddr, word rawLevel */
  LSV_OCACTUAL,     /* byte outchan, byte devAddr, byte type, byte realLevel */
  LSV_ASM,          /* byte msg, byte idx, int val */
  LSV_CLRMAXMINS,   /* system: clear all airspace max mins */
  LSV_NSD,          /* network startup delay: int delay, word priority */
  /* LSV_LANADDR,      network address */
  LSV_LOGEVENT,			/* logging event flag */
  LSV_SYSEND,

  LSV_ASSTART = 0x80,   /* start of air-space specific values */
  LSV_ACTSETTEMP = LSV_ASSTART,   /* int act temp, int set temp */
  LSV_ASALARMS,     /* word a/s alarmFlags, Type0: word sys alarmFlags */
  LSV_SETTEMP,      /* int set temp (non-curve setting) */
  LSV_MINVENT,      /* byte min vent rate (non-curve setting) */
  LSV_STARTDAY,     /* word curve start day */
  LSV_MAXMINTEMP,   /* int max temp, int min temp */
  LSV_MAXMINHRS,    /* word max/min hours */
  LSV_PERFMETER,    /* byte band, null, word count */
  LSV_USERMETER,    /* byte band, null, word count */
  LSV_ASASM,        /* byte msg, byte idx, int val */
  LSV_END
} TStdVal ;

#define LSv( v ) ((byte) LSV_ ## v)

/* application types */
typedef enum {
  AT_FSC,
  AT_SFC,
  AT_MON,
  AT_PSC,
  AT_NVZ,
  AT_NVS,
  AT_PCI,
  AT_MSS,
  AT_SIO,
  AT_ITK,
  AT_CRP,
  AT_MN8,
  AT_LGR,
  AT_GSC,
  AT_FMX,
  AT_ACR,
  AT_CCR,
  AT_PSC1,
  AT_RMS,
  AT_DCS,
  AT_CFC,
  AT_MMX,
	AT_GHC,
	AT_SMS,
	AT_CCL
} TAppType ;

/* Standard message categories
 * high nibble is message category
 * low nibble is used for airspace number, 0 for system */
#define LSM_NULL      0x10  /* just a message - no action */
#define LSM_NOVALUE   0x20  /* <sv> this unit does not have value */
#define LSM_VALUE     0x30  /* <sv> <val> [...] this units value */
#define LSM_RETURN    0x40  /* <sv> [<arg> ...] return value to sender */
#define LSM_BROADCAST 0x50  /* <sv> [<arg> ...] broadcast this value */
#define LSM_SET       0x60  /* <sv> [<arg> ...] <val> [...] set rx sv to val */
#define LSM_SETRET    0x70  /* <sv> set and return */

#define LSm( m ) ((byte) LSM_ ## m)
#define MAX_AIRSPACE    15
#define MAX_UNIT        30

#endif
