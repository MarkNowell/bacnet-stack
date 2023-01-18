#include "bac2slan-hw.h"

const char VersStr[] PROGMEM = "0.2" ;

const char TestStr[] PROGMEM = "BACNet/SLAN-bridge\r\n" ;	// first char of TestStr needs to be unique in string
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
#define DEF_TEST		3

uint32_t testCount, failCount ;

// #define SetLED(pt, pn, on) { if (on) pt &= ~_BV(pn) ; else pt |= _BV(pn) ; }
#define SetLED( bit, on ) { bit = on ? 0 : 1 ; }

int main( void ) {
	int8_t testn = DEF_TEST, lasttest = -1 ;
	uint8_t ch ;
	bool bSwitch = true ;
	InitIO() ;
	puts_P(PSTR("\r\nBAC2Slan hardware test v")) ; puts_P(VersStr) ; puts_P(PSTR("\r\n")) ;
	BIT_LED1 = 0 ;
	BIT_LED2 = 1 ;
	while (1) {
		UpdateIO() ;
		if ((UCSR0A & _BV(RXC0)) != 0) {
			ch = UDR0 ;
			if (ch >= '0' && ch <= '9') {
				testn = ch - '0' ;
				bSwitch = true ;
			} ;
			if (testn >= NUM_TESTS) {
				puts_P(PSTR("Tests: 0=blink, 1=UART1-TX, 2=UART2-TX, 3=UART1+2\r\n")) ;
				testn = lasttest ;
			} ;
		} ;
		if (bSwitch) { 
			printf_P(PSTR("Test %u: %lu / %lu\r\n"), lasttest, failCount, testCount) ;
			if (lasttest >= 0)
				tests[lasttest].deinit() ;
			printf_P(PSTR("Switching to test %d\r\n"), testn) ;
			lasttest = testn ;  
			failCount = 0 ; testCount = 0 ;
			InitPorts() ;			// reset to default IO
			tests[testn].init() ;
			bSwitch = false ;
		} ;
		tests[testn].update() ;
		SetLED(BIT_LED0, rt.ledCycle & 0x01) ;
	} ;
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
		UDR1 = pgm_read_byte(&TestStr[bufp]) ;
		if (++bufp >= TestStrLen)
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
		UDR2 = pgm_read_byte(&TestStr[bufp]) ;
		if (++bufp >= TestStrLen)
			testCount++ ;
	} ;
}

//===========================================================================
// Test3 UART1/2 TX/RX
static uint8_t bufdir ;
uint32_t txCount, rxCount ;

void Test3Init() {
	SptEn(1, UBRR_9600) ;
	SptEn(2, UBRR_9600) ;
	bufp = 0 ;
	bufdir = 1 ;
	txCount = 0 ; rxCount = 0 ;
}
void Test3End() {
	printf_P(PSTR("Test3 (bi-directional tx/rx): Tx %lu, Rx %lu\r\n"), txCount, rxCount) ;
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
				txCount++ ;
				printf("t%02X", ch) ;
			} ;
		} ;
		if (bUCSR2A(rxc)) {
			ch = UDR2 ; 
			if (ch == '\n') {	// finished - switch direction
				bufdir = 0 ;
				bufp-- ;
				testCount++ ;
			} ;
			rxCount++ ;
			printf("r%02X", ch) ;
		} ;
	}
	else {		// tx 2 => 1
		BIT_BAC_DE = 0 ;
		BIT_BAC_RE = 0 ;
		BIT_SLAN_DE = 1 ;
		BIT_SLAN_RE = 1 ;
		if (bUCSR2A(udre)) {
			if (bufp >= 0) {
				ch = pgm_read_byte(&TestStr[bufp--]) ;
				UDR2 = ch ;
				txCount++ ;
				printf("w%02X", ch) ;
			} ;
		} ;
		if (bUCSR1A(rxc)) {
			ch = UDR1 ; 
			if (ch == pgm_read_byte(TestStr)) {	// finished - switch direction
				bufdir = 1 ;
				bufp = 0 ;
				testCount++ ;
			} ;
			rxCount++ ;
			printf("r%02X", ch) ;
		} ;
	} ;
}

