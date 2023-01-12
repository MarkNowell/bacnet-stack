#include "bac2slan-hw.h"

PGM_P VersStr = "0.2" ;

PGM_P TestStr = "BACNet/SLAN-bridge\n" ;
const unsigned TestStrLen = sizeof(TestStr) ;

void Test0Init() ; void Test0Update() ; void Test0End() ;
void Test1Init() ; void Test1Update() ; void Test1End() ;
void Test2Init() ; void Test2Update() ; void Test2End() ;
void Test3Init() ; void Test3Update() ; void Test3End() ;

struct { void (*init)() ; void (*update)() ; void (*deinit)() ; } tests[] = {
	{ Test0Init, Test0Update, Test0End },
	{ Test1Init, Test1Update, Test1End },
	{ Test2Init, Test2Update, Test2End },
	{ Test3Init, Test3Update, Test3End }
} ;
#define NUM_TESTS		4

uint32_t testCount, failCount ;

// #define SetLED(pt, pn, on) { if (on) pt &= ~_BV(pn) ; else pt |= _BV(pn) ; }
#define SetLED( bit, on ) { bit = on ? 0 : 1 ; }

int main( void ) {
	int8_t testn = 0, lasttest = -1 ;
	// uint8_t ucsr = 0 ;
	uint16_t udr ;
	uint8_t ch ;
	InitIO() ;
	printf_P(PSTR("\r\nBAC2Slan hardware test v%s\r\n"), VersStr) ;
	BIT_LED1 = 0 ;
	BIT_LED2 = 1 ;
	while (1) {
		UpdateIO() ;
		// if (UCSR0A != ucsr) { ucsr = UCSR0A ;	printf(" %02X", ucsr) ;	} ;
		// if (bUCSR0A(rxc)) {
		if ((UCSR0A & _BV(RXC0)) != 0) {
			udr = UDR0 ;
			ch = udr & 0x7f ;		// problems with top bit being set all the time on rx - TODO3 - fix this
			printf("Rx0: %c %02X %04X\r\n", ch, ch, udr) ;
			printf("UCSR0A/B/C: %02X / %02X / %02X\r\n", UCSR0A, UCSR0B, UCSR0C) ;
			if (ch >= '0' && ch <= '9') 
				testn = ch - '0' ;
			if (testn >= NUM_TESTS) {
				printf_P(PSTR("Tests: 0=blink, 1=UART1-TX, 2=UART2-TX, 3=UART1+2\r\n")) ;
				testn = lasttest ;
			} ;
		} ;
		if (testn != lasttest) { 
			printf_P(PSTR("Test %u: %lu / %lu\r\n"), lasttest, failCount, testCount) ;
			printf_P(PSTR("Switching to test %d\r\n"), testn) ;
			if (lasttest >= 0)
				tests[lasttest].deinit() ;
			lasttest = testn ;  
			failCount = 0 ; testCount = 0 ;
			InitPorts() ;			// reset to default IO
			tests[testn].init() ;
		} ;
		tests[testn].update() ;
		SetLED(BIT_LED0, rt.ledCycle & 0x01) ;
	}
}

//===========================================================================
// Test0 LED blink cycle only
void Test0Init() {
}
void Test0End() {
}
void Test0Update( void ) {
	SetLED(BIT_LED1, rt.ledCycle & 0x01) ;
	SetLED(BIT_LED2, rt.ledCycle & 0x02) ;		
	testCount++ ;
}


//===========================================================================
// Test1 UART1 TX 
static uint8_t bufp ;

void Test1Init() {
	SptEn(1, UBRR_9600) ;
	BIT_BAC_DE = 1 ;
	bufp = 0 ;
}
void Test1End() {
}
void Test1Update( void ) {
	if (bUCSR1A(udre)) {
		UDR1 = pgm_read_byte(TestStr[bufp++ % TestStrLen]) ;
		testCount++ ;
	} ;
}

//===========================================================================
// Test2 UART2 TX 

void Test2Init() {
	SptEn(2, UBRR_9600) ;
	BIT_SLAN_DE = 1 ;
	bufp = 0 ;
}
void Test2End() {
}
void Test2Update( void ) {
	if (bUCSR2A(udre)) {
		UDR2 = pgm_read_byte(TestStr[bufp++ % TestStrLen]) ;
		testCount++ ;
	} ;
}

//===========================================================================
// Test3 UART1/2 TX/RX
static uint8_t bufdir ;

void Test3Init() {
	SptEn(1, UBRR_9600) ;
	SptEn(2, UBRR_9600) ;
	bufp = 0 ;
	bufdir = 1 ;
}
void Test3End() {
}
void Test3Update( void ) {
	char ch ;
	if (bufdir) {	// tx 1 => 2
		BIT_BAC_DE = 1 ;
		BIT_BAC_RE = 1 ;
		BIT_SLAN_DE = 0 ;
		BIT_SLAN_RE = 0 ;
		if (bUCSR1A(udre)) {
			if ((ch = pgm_read_byte(&TestStr[bufp])) != '\0') {
				UDR1 = ch ;
				bufp++ ;
				testCount++ ;
			} ;
		} ;
		if (bUCSR2A(rxc)) {
			failCount++ ;
			ch = UDR2 ; ch &= 0x7f ;
			if (ch == '\n') {	// finished - switch direction
				bufdir = 0 ;
				bufp-- ;
			} ;
			printf("+%c", ch) ;
		} ;
	}
	else {		// tx 2 => 1
		BIT_BAC_DE = 0 ;
		BIT_BAC_RE = 0 ;
		BIT_SLAN_DE = 1 ;
		BIT_SLAN_RE = 1 ;
		if (bUCSR2A(udre)) {
			if (bufp >= 0) {
				UDR2 = pgm_read_byte(&TestStr[bufp--]) ;
				testCount++ ;
			} ;
		} ;
		if (bUCSR1A(rxc)) {
			failCount++ ;
			ch = UDR1 ; ch &= 0x7f ;
			if (ch == pgm_read_byte(TestStr)) {	// finished - switch direction
				bufdir = 1 ;
				bufp = 0 ;
			} ;
			printf("-%c", ch) ;
		} ;
	} ;
}

