#ifndef _MN_DEFS_H
#define _MN_DEFS_H

#include <avr/io.h>
#include <inttypes.h>

typedef uint8_t byte ;
typedef uint16_t word ;

// typedef const char *string ;

typedef union {
	struct { byte lo, hi ; } b ;
  unsigned int w ;
  signed int i ;
} wordByte ;

typedef union {
	struct {
		unsigned a : 1 ; unsigned b : 1 ; unsigned c : 1 ; unsigned d : 1 ;
		unsigned e : 1 ; unsigned f : 1 ; unsigned g : 1 ; unsigned h : 1 ;
	} fl ;
	byte b ;
} TFlagSet ;

void memclr( register byte *p, register int count ) ;

#define false					0
#define true					1
#ifndef NULL
#define NULL					0
#endif

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
} io_reg ; 

#define BDF(pt, pn)		(((volatile io_reg*)_SFR_MEM_ADDR(pt))->bit ## pn)

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
#define bUCSR3A(pn)		(((volatile UCSRnA*)_SFR_MEM_ADDR(UCSR3A))->_ ## pn)



#endif		// _MN_DEFS_H
