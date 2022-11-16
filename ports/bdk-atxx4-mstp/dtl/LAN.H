/* LAN.H - lan global exports */
#include "mn_defs.h"

#ifndef _LAN_H
#define _LAN_H

#define LANSTATS	true

#include "stdmsg.h"
#include "stdalarm.h"

/* Lan initialisation */
void LanInit( void ) ;

#define LAN_MAXBYTE     7
#define LAN_MAXWORD     3
#define LAN_MAXMSGBYTE  3
#define LAN_MAXMSGWORD  1

typedef union {
  int i[LAN_MAXWORD + 1] ;
  word w[LAN_MAXWORD + 1] ;
  byte b[LAN_MAXBYTE + 1] ;
} TLanBuf ;

/* Send/receive/request message buffers etc */
extern TLanBuf
  txMsg ;      /* single tx buffer */
extern TLanBuf
  *lanMsg,    /* utility ptr to next tx/rx/rq msg/buffer */
  *rqMsg ;    /* next free rq buffer */
extern const byte numLanRqBufs ;  /* number rq bufs allocated by app */
extern TLanBuf rqMsgBuf[] ;       /* rq msg buffer allocated by app in lant */
extern byte lanAddr, lanType ;
extern byte rxAvail, rqAvail ;    /* rx/rq msgs in buf */
extern word lanRxBytes ;
extern byte lanDbg, lanStartTx, lanEndTx ;

#define LanTxFree()   (!txMsg.b[0])
void LanGetRx( void ) ;     /* point lanMsg to next full rx msg in queue */
void LanInsertRx( void ) ;  /* point lanMsg to available rx buf, add rx */
#define LanRxAvail()  (rxAvail)
#define LanRxDone()   (rxAvail--)
void LanGetRq( void ) ;     /* point lanMsg to next full rq msg in queue */
#define LanRqAvail()  (rqAvail)
#define LanRqDone()   (rqAvail--)
void LanAddRq( void ) ;     /* update queue after filling rqMsg */

byte LanAutoAddr( void ) ;		// try and find a spare lanAddr

#define LB_TXADDR		0
#define LB_RXADDR		1
#define LB_LSM      2
#define LB_LSV      3

#define Lx( n )   ((byte *) lanMsg)[LB_ ## n]
#define Lxb( n )  ((byte *) lanMsg)[n + 4]
#define Lxw( n )  ((word *) lanMsg)[n + 2]
#define Lxi( n )  ((int16_t *) lanMsg)[n + 2]

// LANT defs
void InitLanTask( void ) ;
void LanTask( void ) ;
extern byte rDevTimeout ;

#ifdef LANSTATS
typedef struct {
	word
		addrColl, 	/* address collisions */
		bodyErrs, 	/* incorrect body bytes detected on tx */
		tx, 				/* all transmitted msgs */
		rx, 				/* received for this unit */
		rxAll,			/* received for all units */
		timeout,		/* unfinished msgs, rx/tx */
		chkErrs,	 	/* chkSum detected errors */
		rxStack,		// max rx queue
		rqStack ;		// max rq queue
} TLanStats ;

extern TLanStats lst ;
#endif		/* LANSTATS */

#endif


