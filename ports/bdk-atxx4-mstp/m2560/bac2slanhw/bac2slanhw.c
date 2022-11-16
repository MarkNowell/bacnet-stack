
#include <avr/io.h>
#include <avr/interrupt.h>
// #include <avr/signal.h>
#include <avr/pgmspace.h>

typedef uint8_t byte ;
typedef uint16_t word ;


typedef struct 
{ 
  unsigned char bit0:1; 
  unsigned char bit1:1; 
  unsigned char bit2:1; 
  unsigned char bit3:1; 
  unsigned char bit4:1; 
  unsigned char bit5:1; 
  unsigned char bit6:1; 
  unsigned char bit7:1; 
}io_reg; 

// #define BIT_LED1 						((volatile io_reg*)_SFR_MEM_ADDR(PORTG))->bit5 
// #define BIT_LED2	 					((volatile io_reg*)_SFR_MEM_ADDR(PORTE))->bit3

#define BDF(pt, pn)		(((volatile io_reg*)_SFR_MEM_ADDR(pt))->bit ## pn)
#define BIT_LED1			BDF(PORTB,5)
#define BIT_LED2			BDF(PORTB,6)
#define BIT_BAC_DE		BDF(PORTH,3)
#define BIT_BAC_RE		BDF(PORTH,4)
#define BIT_SLAN_DE		BDF(PORTH,5)
#define BIT_SLAN_RE		BDF(PORTH,6)

typedef struct 
{ 
  unsigned char _mpcm:1; 
	unsigned char _u2x:1; 
  unsigned char _upe:1; 
  unsigned char _dor:1; 
  unsigned char _fe:1; 
  unsigned char _udre:1; 
  unsigned char _txc:1; 
  unsigned char _rxc:1; 
} UCSRnA ; 

#define bUCSR0A(pn)		(((volatile UCSRnA*)_SFR_MEM_ADDR(UCSR0A))->_ ## pn)
#define bUCSR1A(pn)		(((volatile UCSRnA*)_SFR_MEM_ADDR(UCSR1A))->_ ## pn)
#define bUCSR2A(pn)		(((volatile UCSRnA*)_SFR_MEM_ADDR(UCSR2A))->_ ## pn)

void InitIO( void ) {
	// PORTA unused
	DDRA = 0 ; PORTA = 0xff ;
	// PORTB: Arduino yellow LED on PB7, Diag LEDs on PB5/6 otherwise unused
	DDRB = 0xe0 ; PORTB = 0xff ;
	// PORTC: unused
	DDRC = 0 ; PORTC = 0xff ;
	// PORTD				0=NC(I^),		1=NC(I^),			2=RXD1(I^),	3=TXD1(O^),	4=NC(I^) 		5=NC(I^),		6=NC(I^),		7=NC(I^),
	PORTD = (byte) (0x01	 			| 0x02	 			| 0x04    	| 0x08 			| 0x10 			| 0x20  		| 0x40 			| 0x80 ) ;
	DDRD = (byte)  (0     			| 0		 				| 0 				| 0x08 			| 0		 			| 0		  		| 0		 			| 0    ) ;
	// PORTE				0=RXD0(I^)	1=TXD0(O^)		2=NC(I^)		3=NC(I^), 	4=SlRx2(I^)	5=BARx2(I^) 6=NC(I^),		7=NC(I^),
	PORTE = (byte) (0x01	 			| 0x02	 			| 0x04    	| 0x08 			| 0x10 			| 0x20  		| 0x40 			| 0x80 ) ;
	DDRE = (byte)  (0     			| 0x02 				| 0 				| 0x0 			| 0		 			| 0		  		| 0		 			| 0    ) ;
	// PORTF: unused/Jtag
	DDRF = 0 ; PORTF = 0xff ;
	// PORTG: unused
	DDRG |= 0 ; PORTF |= 0xff ;
	// PORTH				0=RXD2(I^)	1=TXD2(O^)		2=NC(I^) 		3=BaDE(O)		4=BaRE(O^),	5=SlDE(O)		6=SlRE(O^) 	7=NC(I^)
	PORTH = (byte) (0x01	 			| 0x02	 			| 0x04    	| 0		 			| 0x10 			| 0		  		| 0x40 			| 0x80 ) ;
	DDRH = (byte)  (0   				| 0x02 				| 0					| 0x08 			| 0x10 			| 0x20  		| 0x40 			| 0    ) ;
	// PORTJ: unused
	DDRJ = 0 ; PORTJ = 0xff ;
	// PORTK: unused
	DDRK = 0 ; PORTK = 0xff ;
	// PORTL: unused
	DDRL = 0 ; PORTL = 0xff ;
}

#define T0_POSTSCALE		250		// 1ms @ ck/64
#define T0A_10MS				200		// Number post-scaled T0 counts per 10ms
#define NUM_ADCSAMPLES	50		// ADC samples per reading for 10V (50 x 20ms = 1sec)

void memclr( register byte *p, register int count ) {
	for ( ; count > 0 ; count--, p++) 
		*p = 0 ;
}

typedef struct { 
	byte lastT0 ;
	byte ms ;
	byte ledCycle ;
} TRealTime ;
TRealTime rt ;

void InitTimers( void ) {
	TCCR0A = 0 ;
	TCCR0B = _BV(CS01) | _BV(CS00) ;	// ck/64
	memclr((byte *) &rt, sizeof(TRealTime)) ;
}

void UpdateTimers( void ) {
	// update timers
	byte tmp = (byte) TCNT0 - rt.lastT0 ;
	if (tmp < T0_POSTSCALE) return ;
	// 1ms invt here
	rt.lastT0 += T0_POSTSCALE ;
	if (++rt.ms >= 250) {
		rt.ms = 0 ;
		if (++rt.ledCycle >= 16)
			rt.ledCycle = 0 ;
	} ;
}

const char hello[] PROGMEM = "Hello BACnet!!\r\n" ;
int bufp, bufdir ;

void SptInit( void ) {
	static const unsigned baud = 103 ;	// 9600 @ 16MHz
	/* Set baud rate */
	UBRR0 = 25 ;		// Arduino uart on USART0?
	UBRR1 = baud ;		// BACnet on USART1
	UBRR2 = baud ;		// Slan on USART2
	UCSR0B = _BV(TXEN0) | _BV(RXEN0) ;	// enable Tx/Rx on 0
	UCSR1B = _BV(TXEN1) | _BV(RXEN1) ;	// enable Tx/Rx on 1
	UCSR2B = _BV(TXEN2) | _BV(RXEN2) ;	// enable Tx/Rx on 2

	bufdir = 1 ;
	bufp = 0 ;
}

void SptUpdate( void ) {
	char ch ;
	if (bufdir) {	// tx 1 => 2
		BIT_BAC_DE = 1 ;
		BIT_BAC_RE = 1 ;
		BIT_SLAN_DE = 0 ;
		BIT_SLAN_RE = 0 ;
		if (bUCSR1A(udre) && bUCSR0A(udre)) {
			if ((ch = pgm_read_byte(hello + bufp)) != '\0') {
				UDR1 = ch ;
				UDR0 = ch ; 		// debug
				bufp++ ;
			} ;
		} ;
		if (bUCSR2A(rxc)) {
			if ((ch = UDR2) == '\n') {	// finished - switch direction
				bufdir = 0 ;
				bufp-- ;
			} ;
		} ;
	}
	else {		// tx 2 => 1
		BIT_BAC_DE = 0 ;
		BIT_BAC_RE = 0 ;
		BIT_SLAN_DE = 1 ;
		BIT_SLAN_RE = 1 ;
		if (bUCSR2A(udre)) {
			if (bufp >= 0) 
				UDR2 = pgm_read_byte(hello + bufp--) ;
		} ;
		if (bUCSR1A(rxc)) {
			if ((ch = UDR1) == 'H') {	// finished - switch direction
				bufdir = 1 ;
				bufp = 0 ;
			} ;
		} ;
	} ;
}

// #define SetLED(pt, pn, on) { if (on) pt &= ~_BV(pn) ; else pt |= _BV(pn) ; }
#define SetLED( bit, on ) { bit = on ? 0 : 1 ; }

int main( void ) {
	InitIO() ;
	InitTimers() ;
	SptInit() ;
	bufp = 0 ;
	BIT_LED1 = 0 ;
	BIT_LED2 = 1 ;
	while (1) {
		UpdateTimers() ;
		SptUpdate() ;
		SetLED(BIT_LED1, rt.ledCycle & 0x01) ;
		SetLED(BIT_LED2, rt.ledCycle & 0x02) ;		
		// SetLED(PT_BAC_DE, PN_BAC_DE, rt.ledCycle & 0x04) ;		
		// SetLED(PT_SLAN_DE, PN_SLAN_DE, rt.ledCycle & 0x08) ;		
	}
}
