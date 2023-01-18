#pragma once

#include <avr/io.h>
#include <avr/interrupt.h>
// #include <avr/signal.h>
#include <avr/pgmspace.h>
#include <stdio.h> 
#include <stdint.h> 
#include <stdbool.h>

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
#define BIT_LED0			BDF(PORTB,7)
#define BIT_LED1			BDF(PORTB,5)
#define BIT_LED2			BDF(PORTB,6)
#define BIT_BAC_DE		BDF(PORTH,3)
#define BIT_BAC_RE		BDF(PORTH,4)
#define BIT_SLAN_DE		BDF(PORTH,5)
#define BIT_SLAN_RE		BDF(PORTH,6)

// ************************ Serial ports ***********************

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

// UBRR values for 16MHz
#define UBRR_4800		207	// error 0.2
#define UBRR_9600		103	// error 0.2
#define UBRR_14400	68	// error 0.6
#define UBRR_19200	51	// error 0.2
#define UBRR_28800	34	// error 0.8
#define UBRR_38400	25	// error 0.2
#define UBRR_57600	16	// error 2.1
#define UBRR_76800	12	// error 0.2
#define UBRR_115200	8		// error 3.7

#define SptEn(n, baud) 	{  UBRR ## n = baud ; UCSR ## n ## B = _BV(TXEN0) | _BV(RXEN0) ; UCSR ## n ## C = 0x06 | _BV(UPM00) | _BV(UPM01) ; }
#define SptDis(n)				{ UCSR ## n ## B = 0 ; }


// ************************* Timers *********************************

typedef struct { 
	byte lastT0 ;
	byte ms ;
	byte ledCycle ;
} TRealTime ;
extern TRealTime rt ;


// Exported
void InitPorts() ;
void InitIO() ;         // initialise ports, timers and UART0 for debug/printf 
void UpdateIO() ;
