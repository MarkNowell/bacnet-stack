// #include "include\\80C196.h"
// #include "io\\iopriv.h"

#include <avr/io.h>
#include <avr/interrupt.h>
// #include <avr/signal.h>
#include <avr/eeprom.h>

#include "lan.h"
#include "hardware.h"

#ifdef LANSTATS
TLanStats lst ;
#endif		/* LANSTATS */


/* Number of received message buffers must be sufficient to prevent overflow
 * between applications processing of messages. Ie must take max possible
 * number of messages received during greatest task duration.
 * Max task duration 150ms (see watchdog.h). */
#define NUM_RXBUFS	20
#define NUM_RQBUFS	20

/* AVR USART doesn't appear to match 196 in all respects - it seems to provide a shorter stop bit, so
AVR uses 2-stop-bit mode. Quiet time also has to be extended by 5us - probably something to do with
interrupt latency, but this was figured out during development of the original Slan/LAN board based on
2313 (see mio-lan1-2-0.doc) */

#define LAN_BAUD_UBRR		103			/* UBRR setting for 9600 @ 16MHz (0.2% error) */
#define LAN_QUIET_T1		520 // 520			/* Quiet time: 265us in T1 @ 16MHz / 8 */
#define LAN_FREE_T1			395 // 395			/* Free time: 197.333us in T1 @ 16MHz / 8 */

word lanRxBytes ;
byte
	lanAddr,			/* unit lan addr */
	lanChkSum,		/* Type 1 msg chksum */
  lanPriority ; /* Type 0 tx priority, or Type 1 last addr */
byte
	lanTxAddr,
	lanFreeAddr[4] ;	// used to find a free address

byte lanState ;
#define LS_QUIET				0x01
#define LS_RX 					0x02
#define LS_TX 					0x04
#define LS_IGNORING 		0x08

#define Transmit(b) 		(UDR = b)
#define Receive() 			UDR
#define LAN_ACK 				0

#define SetDE()			{ BIT_SLAN_DE = 1 ; }
#define ClrDE()			{ BIT_SLAN_DE = 0 ; }
#define SetRE()			{ BIT_SLAN_RE = 0 ; }
#define ClrRE()			{ BIT_SLAN_RE = 1 ; }

byte lanByte ; 			/* current byte to transmit/receive */

/* tx/rx/rq buffers, and control data */
TLanBuf *lanMsg, *rqMsg ;
TLanBuf txMsg ;
TLanBuf rqMsgBuf[NUM_RQBUFS] ;       /* rq msg buffer allocated by app in lant */
static TLanBuf rxMsgBuf[NUM_RXBUFS] ;
static  TLanBuf *rxP ;
byte rxAvail, rqAvail ;

static byte lanData ;

// M2560 Rx edge detect is on INT4
// Timer1 can be used as it is in SlanD
// USART2 is used
#define Int0Flag()		(EIFR & _BV(INTF4)) 
#define ClrInt0Flag() (EIFR = _BV(INTF4)) 
#define ClrTOV1()			(TIFR1 = _BV(TOV1))
#define UCSRA 		UCSR2A
#define UCSRB 		UCSR2B
#define UCSRC 		UCSR2C
#define PE				UPE2
#define FE				FE2
#define UDR				UDR2
#define UBRRL			UBRR2L
#define UBRRH			UBRR2H
#define RXCIE			RXCIE2
#define RXEN			RXEN2
#define TXEN			TXEN2
#define TIMSK			TIMSK2


// SIGNAL(SIG_USART2_RECV) {
ISR(USART2_RX_vect) {
	/* clear the LanActive interrupt pending, set timer to find quiet */
	TCNT1 = -LAN_QUIET_T1 ;
	lanState &= ~LS_QUIET ; 	/* clear quiet status */
	ClrTOV1() ;								/* clear extint bit and pending tmr overflow (clear by writing 1 to bits) */
	ClrInt0Flag() ;
	if (UCSRA & (_BV(PE) | _BV(FE))) { 		 /* parity/framing error */
		lanState = LS_IGNORING ;
#ifdef LANSTATS
		if (lanState & LS_TX) {
			if (lanByte) lst.bodyErrs++ ;
			else lst.addrColl++ ;
		} ;
#endif
	} 
	else lanRxBytes++ ;
	lanData = Receive() ; 		/* get the data whatever we're doing */
	/* enable higher priority interrupts having covered the timing control (IR interrupts here??) */
	// int_mask = 0x0c ; 				/* enable hso pin, hsi data for serial port */
	// enable() ;
	// if (!lanAddr) goto endRx ;
	if (!lanByte) { 		/* 1st (txAddr) byte */
		lanChkSum = lanData ;
		lanPriority = lanData & 0x1f ;
		lanTxAddr = lanPriority ;
		if (lanData != txMsg.b[LB_TXADDR]) { 	/* not our txaddr */
			lanState = LS_RX ;		/* assume rx buffer free - overwrite old msgs */
		#ifdef LANSTATS
			if (lanState & LS_TX) lst.addrColl++ ;
		#endif
		} ;
	}
	else if (lanByte <= LAN_MAXBYTE) lanChkSum += lanData ;
	if (lanState & LS_TX) { 	/* transmitting */
		if (lanByte > LAN_MAXBYTE) {		/* chkSum return */
		#ifdef LANSTATS
			if (lanData == lanChkSum) {
				txMsg.b[LB_TXADDR] = 0 ;		/* tx available */
				lst.tx++ ;
			} else lst.bodyErrs++ ;
		#else
			if (lanData == lanChkSum) txMsg.b[LB_TXADDR] = 0 ;	/* tx available */
		#endif
			lanState = LS_IGNORING ;		/* tx done */
		}
		else if (lanData == txMsg.b[lanByte]) {  /* byte sent OK */
			if (++lanByte <= LAN_MAXBYTE) Transmit(txMsg.b[lanByte]) ; /* next byte */
			else Transmit(lanChkSum) ; /* msg end - tx chksum */
		}
		else {
			lanState = LS_IGNORING ;
		#ifdef LANSTATS
			lst.bodyErrs++ ;
		#endif
		} ;
	}
	else if (lanState & LS_RX) {
		if (lanByte <= LAN_MAXBYTE) {
			((byte *) rxP->b)[lanByte] = lanData ;	 /* store byte */
		#ifndef LANSTATS
			if (lanByte == LB_RXADDR && (lanData &= 0x1f) && lanData != lanAddr)
				lanState = LS_IGNORING ;
		#endif
			lanByte++ ;
		}
		else { 		/* chkSum byte */
			if (lanData == lanChkSum) {
				if (lanAddr == 0)
					lanFreeAddr[lanTxAddr >> 3] |= (byte) 1 << (lanTxAddr & 0x07) ;
			#ifdef LANSTATS
				lst.rxAll++ ;
				if (!(lanData = ((byte *) rxP->b)[LB_RXADDR] & 0x1f) ||
							lanData == lanAddr) {
					lst.rx++ ;
			#endif
					if (rxAvail < NUM_RXBUFS) rxAvail++ ;
					if (++rxP >= rxMsgBuf + NUM_RXBUFS) rxP = rxMsgBuf ;
			#ifdef LANSTATS
					if (rxAvail > lst.rxStack) lst.rxStack = rxAvail ;
				} ;
			#endif
			}
		#ifdef LANSTATS
			else lst.chkErrs++ 
		#endif
			;
			lanState = LS_IGNORING ;
		} ;
	} ;
	if (!(lanState & LS_TX))	
		ClrDE() ;
}

// void LanActive( void ) {	/* LAN falling edge interrupt */
// SIGNAL(SIG_INTERRUPT4) {
ISR(INT4_vect) {
	/* this interrupt not enabled: pending bit only is used */
	return ;
}

// void LanTimer( void ) { /* T2 timeout interrupt used for LAN timing */
// SIGNAL(SIG_OVERFLOW1) {
ISR(TIMER1_OVF_vect) {
	if (lanState & LS_QUIET) {	/* free now */
		if (txMsg.b[LB_TXADDR] && !Int0Flag()) {  /* quiet, free: tx */
			SetDE() ;
			Transmit(txMsg.b[LB_TXADDR]) ;
			lanState = LS_TX ;
		}
		else { /* missed free, or no tx waiting - wrap free timer */
			/* Type1 wrap delay = (MAX_UNIT + 1) * free */
			TCNT1 -= (MAX_UNIT + 1) * LAN_FREE_T1 ;
			// if (txMsg.b[LB_TXADDR] && (GIFR & _BV(INT0))) lanDbg++ ;
		} ;
	}
	else if (!Int0Flag()) {	/* quiet now */
		/* update timer2 with free-time */
		/* Type1 free = (addr - lastAddr + MAX_UNIT - 1) % MAX_UNIT + 1 */
		TCNT1 -= ((lanAddr - lanPriority + MAX_UNIT - 1) % MAX_UNIT + 1) *	LAN_FREE_T1 ;
		lanByte = 0 ;
	#ifdef LANSTATS
		if (lanState & (LS_TX | LS_RX)) lst.timeout++ ;
	#endif
		lanState = LS_QUIET ; /* clear any other active states */
	} ;
	/* else, rx should reset quiet timer */
	ClrInt0Flag() ;		/* clear lan active bit */
	if (!(lanState & LS_TX))	
		ClrDE() ;
}

void LanInit( void ) {
	static const unsigned baud = 103 ;	// 9600 @ 16MHz
	ClrDE() ;
	/* Set baud rate */
	UCSRB = 0 ; UCSRA = 0 ;
	/* Set frame format: 9bits inc even parity, 1stop bit */
	UCSR2C = 
		_BV(UPM21) | _BV(USBS2) |			// even parity enabled, TWO stop bits
		_BV(UCSZ20) | _BV(UCSZ21) ;		// 9bit (with UCSZ2 in UCSRB)
	UBRRL = (unsigned char) baud ;
	UBRRH = (unsigned char) (baud>>8) ;
	/* Enable rxintr, receiver, transmitter */
	UCSRB = _BV(RXCIE) | _BV(RXEN) | _BV(TXEN) ; // | _BV(UCSZ2) ;

	TCCR1B = 0x02 ;			// Timer1: normal operation, ck/8
	TIMSK1 |= _BV(TOIE1) ;	// enable T1 ovf intr
	lanPriority = 0 ;		// really need something random here
	lanState = LS_IGNORING ;
	TCNT1 = 0 ;
	EICRB |= _BV(ISC40) ;	// INT4 on any edge
	rxAvail = 0 ; rxP = rxMsgBuf ;
	rqAvail = 0 ; rqMsg = rqMsgBuf ;
	txMsg.b[0] = 0 ;		/* tx available */
	lanRxBytes = 0 ;
#ifdef LANSTATS
	memclr((byte *) &lst, sizeof(lst)) ;
	memclr((byte *) lanFreeAddr, sizeof(lanFreeAddr)) ;
#endif
	SetRE() ;
}

void LanGetRx( void ) { 		 /* get next rx msg from queue */
	cli() ; 	/* prevent rxP/rxAvail changing during calculation */
	lanMsg = rxP - rxAvail ;
	sei() ;
	if (lanMsg < rxMsgBuf) lanMsg += NUM_RXBUFS ;
}

void LanInsertRx( void ) {	 /* point lanMsg to available rx buffer */
	cli() ;
	lanMsg = rxP ;
	if (rxAvail < NUM_RXBUFS) rxAvail++ ;
	if (rxAvail > lst.rxStack) lst.rxStack = rxAvail ;
	if (++rxP >= rxMsgBuf + NUM_RXBUFS) rxP = rxMsgBuf ;
	if (lanState & LS_RX) *rxP = *lanMsg ;
	sei() ;
}

void LanGetRq( void ) { 		 /* get next full rq msg from queue */
  if ((lanMsg = rqMsg - rqAvail) < rqMsgBuf) lanMsg += NUM_RQBUFS ;
}
void LanAddRq( void ) {
  if (++rqMsg >= rqMsgBuf + NUM_RQBUFS) rqMsg = rqMsgBuf ;
  if (rqAvail < NUM_RQBUFS) rqAvail++ ;
	if (rqAvail > lst.rqStack) lst.rqStack = rqAvail ;
}

byte LanAutoAddr( void ) {
	for (lanAddr = 1 ; lanAddr <= MAX_UNIT ; lanAddr++)
		if ((lanFreeAddr[lanAddr >> 3] & ((byte) 1 << (lanAddr & 0x07))) == 0)
			return lanAddr ;
	return lanAddr = 0 ;
}

