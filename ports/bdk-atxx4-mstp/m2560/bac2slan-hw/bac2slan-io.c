#include <stdio.h>
#include "bac2slan-hw.h"

void InitPorts( void ) {
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

  // Disable 485 serial ports (not debug port)
  SptDis(1) ;
  SptDis(2) ;
}

#define T0_POSTSCALE		250		// 1ms @ ck/64
#define T0A_10MS				200		// Number post-scaled T0 counts per 10ms
#define NUM_ADCSAMPLES	50		// ADC samples per reading for 10V (50 x 20ms = 1sec)

void memclr( register byte *p, register int count ) {
	for ( ; count > 0 ; count--, p++) 
		*p = 0 ;
}

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

static int uart0_putchar(char c, FILE *stream);
static FILE mystdout = FDEV_SETUP_STREAM(uart0_putchar, NULL, _FDEV_SETUP_WRITE) ; 

#define MAX_UART_BUF      50

static char u0buf[MAX_UART_BUF] ;
static uint8_t u0head = 0, u0tail = 0 ;

static int uart0_putchar(char c, FILE *stream) {
  u0buf[u0head++] = c ;
  if (u0head >= MAX_UART_BUF) u0head = 0 ;
  if ((UCSR0A & UDRE0) != 0)   
    UDR0 = c;
  return 0;
}
static int uart0_update() {
  if (u0head != u0tail && (UCSR0A & UDRE0) != 0) {
    UDR0 = u0buf[u0tail++] ;
    if (u0tail >= MAX_UART_BUF) u0tail = 0 ;
  } ;
}

void InitIO() {
  InitPorts() ;
	SptEn(0, UBRR_115200) ;     // enable Port 0 for debug 
  stdout = &mystdout;
}

void UpdateIO() {
  UpdateTimers() ;
  uart0_update() ;
}

