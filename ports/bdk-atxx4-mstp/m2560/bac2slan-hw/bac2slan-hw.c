#include "bac2slan-hw.h"


int Test0Init() ; int Test0Update() ; int Test0End() ;
int Test1Init() ; int Test1Update() ; int Test1End() ;

struct { int *init() ; int *update() ; int *deinit() ; } tests[] = {
	{ Test0Init, Test0Update, Test0End },
	{ Test1Init, Test1Update, Test1End }
}
#define NUM_TESTS		2

// #define SetLED(pt, pn, on) { if (on) pt &= ~_BV(pn) ; else pt |= _BV(pn) ; }
#define SetLED( bit, on ) { bit = on ? 0 : 1 ; }

int main( void ) {
	int8_t testn = 0, lasttest = -1 ;
	InitIO() ;
	BIT_LED1 = 0 ;
	BIT_LED2 = 1 ;
	while (1) {
		UpdateIO() ;
		if (bUCSR0A(rxc) && (ch = UDR0) >= '0' && ch <= '9') {
			testn = ch - '0' ;
			if (testn >= NUM_TESTS) {
				printf("Tests: 0=blink, 1=UART1-TX, 2=UART2-TX, 3=UART1+2\n") ;
				testn = lasttest ;
			} ;
		} ;
		if (testn != lasttest) { 
			printf("Switching to test %d", testn) ;
			InitPorts() ;			// reset to default IO
		} ;
		lasttest = testn ;
	}
}

//===========================================================================

void Test0Init() {
}
void Test0End() {
}
void Test0Update( void ) {
	SetLED(BIT_LED0, rt.ledCycle & 0x01) ;
	SetLED(BIT_LED1, rt.ledCycle & 0x01) ;
	SetLED(BIT_LED2, rt.ledCycle & 0x02) ;		
}


//===========================================================================

void Test1Init() {
	SptInit() ;
	bufdir = 1 ;
	bufp = 0 ;
}
void Test1End() {
	SptStop() ;
}
void Test1Update( void ) {
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

