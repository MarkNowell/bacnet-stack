#include <avr/io.h>
#include <avr/interrupt.h>
// #include <avr/signal.h>
#include <avr/pgmspace.h>

#include "SlanLAN1.h"
#include "lan.h"
#include "timer.h"

static struct etimer timer ;

#define LANT_APPTYPE		AT_SIO
#define TEMP_OFFSET			0

#ifndef LANT_MAXTXSRC
#define LANT_MAXTXSRC   1       /* tx srcs are rq and internal */
#endif
#ifndef NUM_AS
#define NUM_AS          0
#endif
#ifndef Rm
#define Rm( n )         (rm[n])
#endif
#ifndef LANT_NUMRQBUFS
#define LANT_NUMRQBUFS  32
#endif

void HandleRxMsg( void ) ;  /* check for rx msgs, reply/handle as necessary */
void NextTxMsg( void ) ;    /* transmit next message in cycle */
void InitChanTimeouts( void ) ;
void CheckTimeouts( void ) ;
void UpdateLanStats( void ) ;

byte ocTimeout[NUM_IOCHANS] ;

const byte numLanRqBufs = LANT_NUMRQBUFS ;
TLanBuf rqMsgBuf[LANT_NUMRQBUFS] ;

byte txSrc, txItem, txStage, msgUnit, txCycle ;
byte rDevTimeout ;

void InitLanTask( void ) {
  // txSrc = 0 ; txCycle = 0 ;
	txStage = 100 ; 
	timer_elapsed_start(&timer) ;	// timer is a 32 bit millisecond timer
  InitChanTimeouts() ;
}

void LanTask( void ) {
	uint32_t millisecs = timer_elapsed_time(&timer) ;		// get current millisecond time
	static uint32_t last_secs = 0 ;
	uint32_t secs = millisecs / 1000 ;

	if (!lanAddr && secs >= 60 && LanAutoAddr() != 0) {	// try to auto-set lanAddr if not set already
		if (lst.rxAll < 10) timer_elapsed_start(&timer) ;
		else E2Write(Sw(LANADDR), lanAddr) ;
	} ;
  while (LanRxAvail()) HandleRxMsg() ;
  if (secs != last_secs) {
    CheckTimeouts() ;
    txCycle++ ;
    last_secs = secs ;
  } ;
  if (lanAddr && LanTxFree()) NextTxMsg() ;
}

void InitChanTimeouts( void ) {
  for (iTmp.w = 0 ; iTmp.w < NUM_IOCHANS ; iTmp.w++)
    ocTimeout[iTmp.w] = rDevTimeout ;
}
void CheckTimeouts( void ) {
  for (iTmp.w = 0 ; iTmp.w < NUM_IOCHANS ; iTmp.w++)
    if (ocTimeout[iTmp.w]) ocTimeout[iTmp.w]-- ;
    // else OCSetDefault(iTmp.w) ;
}

void OCSetLocal( byte chan, byte level ) {
	OCSet(chan, level) ;
	ocTimeout[chan] = rDevTimeout ;
}

static int handleTxAlarms( void ) {
  msgUnit = 0xff ;      /* for broadcast */
  Lx(LSM) = LSm(VALUE) ;
  Lx(LSV) = LSv(ALARMS) ;
  Lxw(0) = alarmFlags ;
  return true ;
}

static int handleTxICReadings( void ) {
  if (ICActive(txItem)) {		// create a BACMap request to broadcast this channel
    // msgUnit = 0xff ;  -- leave msgUnit=0 to prevent this actually being sent
    Lx(LSM) = LSm(BROADCAST) ;
    Lx(LSV) = LSv(ICRAW) ;
    Lxb(0) = txItem ;
    Lxb(1) = DevAddr(lanAddr, txItem) ;
    Lxw(1) = ID_READERR ;
		BACMap(lanMsg) ;
  } ;
  return (txItem >= NUM_ADCCHANS - 1) ; 
}

#define LANT_TXSTAGES 	2

static void NextInternalTx( void ) {
	register byte stageDone ;
	if (txStage == 0) stageDone = handleTxICReadings() ;
	else if (txStage == 1) stageDone = handleTxAlarms() ;
  else stageDone = true ;
  if (stageDone) {
  	if (++txStage >= LANT_TXSTAGES) {
   		if (txCycle >= St(LANTXCYCLE)) {
     		txStage = 0 ;   /* start slow cycling tasks */
     		txCycle = 0 ;   /* restart cycle */
   		}
   		else txStage = LANT_TXSTAGES ;
   	}
   	txItem = 0 ;
  }
  else txItem++ ;
}

void NextTxMsg( void ) {
  msgUnit = 0 ;
  lanMsg = &txMsg ;
  if (txSrc == 0) {   /* source is rq buf */
    if (LanRqAvail()) {
      LanGetRq() ;
      txMsg = *lanMsg ;
      msgUnit = txMsg.b[LB_RXADDR] ;
      LanRqDone() ;
    } ;
  }
  else if (txSrc == 1) NextInternalTx() ;
  if (msgUnit) {    /* msg to go */
    if (msgUnit == 0xff) msgUnit = 0 ;
    txMsg.b[LB_RXADDR] = msgUnit ;
    if (!txMsg.b[LB_TXADDR]) txMsg.b[LB_TXADDR] = lanAddr ;
    if ((msgUnit & 0x1f) == lanAddr) {
      txMsg.b[LB_TXADDR] = 0 ;
    } ;
  } ;
  if (++txSrc > LANT_MAXTXSRC) txSrc = 0 ;
}

static void handleRxValue( void ) {
	// SlanLAN isn't interested in any received VALUE messages
}

/* HandleRxMsg() sorts incoming messages into VALUE, RETURN/BROADCAST, SET
 * or other; checks that the LSV/airspace combination is valid (airspace
 * number in nAs); calls handleRxValue() for VALUE messages; calls
 * handleRx[sv].retVal (.setVal) for RETURN/BROADCAST (SET) messages.
 * Tx message is initialised to Rx message before calling .retVal().
 * Handlers can normally assume that the LSV/airspace combination is valid
 * and proceed without error checking (there are exceptions eg performance
 * values for restricted airspaces such as creeps). handleRxValue is not
 * table-based because control units are only interested in very few
 * value messages.
 */

static byte nAs, lsv ;  /*  and standard value */
static byte lsm ;       /* message category */

static void reset( void ) {
	cli() ;
  // wdt_enable(WDTO_15MS); // -- don't need to do this if global wd enabled
	while (1) ;
}

#define ADDR_ROMID        0x2020  /* used by PC */
#define ADDR_ROMIDEVB     0x6ffe

#define ADDR_APPNAME      0x3000  /* used by PC */
#define ADDR_MENUOPTIONS  0x3011
#define ADDR_ROMOPTIONS		0x3012

#define ADDR_DICAMID      0x7ffe  /* used by PC */

const char appRom[] PROGMEM = 
// 0123456789ABCDEF
	"SLMOD           \x00\x01\x00" ;
	
static void rvMEMBYTE( void ) { Lxb(2) = *((byte *) Lxw(0)) ; }
static void svMEMBYTE( void ) { *((byte *) Lxw(0)) = Lxb(2) ; }

static void rvMEMWORD( void ) {
	if ((iTmp.w = Lxw(0)) >= ADDR_APPNAME && iTmp.w < ADDR_APPNAME + sizeof(appRom)) {
		iTmp.w -= ADDR_APPNAME ;
		memcpy_P(&(Lxw(1)), appRom + iTmp.w, 2) ;
		// Lxb(2) = appRom[iTmp.w] ; Lxb(3) = appRom[iTmp.w + 1] ;
	} 
	else if (iTmp.w == ADDR_ROMID || iTmp.w == ADDR_DICAMID) Lxw(1) = dicamID ;
	else Lxw(1) = *((word *) Lxw(0)) ; 
}

static void svMEMWORD( void ) {
	if (Lxw(0) == 0x18 && Lxw(1) == 0) reset() ;
	*((word *) Lxw(0)) = Lxw(1) ; 
}
static void rvEEBYTE( void ) { Lxb(2) = E2Read(Lxw(0)) ; }
static void svEEBYTE( void ) {
  if (E2Read(Lxw(0)) != Lxb(2)) E2Write(Lxw(0), Lxb(2)) ;
}
static void rvEEWORD( void ) { 
	if (Lxw(0) == 0) Lxw(1) = lanAddr ;
	else Lxw(1) = E2ReadWord(Lxw(0)) ; 
}
static void svEEWORD( void ) {
  if (E2ReadWord(Lxw(0)) != Lxw(1)) E2WriteWord(Lxw(0), Lxw(1)) ;
}
/* static void rvOCLEVEL( void ) { Lxb(1) = OCType(Lxb(0)) ; Lxb(2) = OCGet(Lxb(0)) ; }
#ifndef SV_OCLEVEL
static void svOCLEVEL( void ) {
  OCSetLocal(Lxb(0), Lxb(2)) ;    
}
#define SV_OCLEVEL    svOCLEVEL
#endif */
#define RV_OCLEVEL    NULL
#define SV_OCLEVEL    NULL

#ifndef RV_ICLEVEL
#define RV_ICLEVEL    NULL
#endif
static void rvDICAMID( void ) { Lxw(0) = dicamID ; }
static void rvAPPTYPE( void ) { Lxw(0) = (byte) LANT_APPTYPE ; }
#ifndef RV_ALARMS
static void rvALARMS( void ) { Lxw(0) = alarmFlags ; }
#define RV_ALARMS   rvALARMS
#endif
#ifndef SV_ALARMS
static void svALARMS( void ) {
  if (Lxw(0) == 0) {
    ClearAlarms() ;
  } ;
}
#define SV_ALARMS   svALARMS
#endif
#ifndef RV_TEMPOFFSET
static void rvTEMPOFFSET( void ) {
  Lxi(0) = TEMP_OFFSET ;
  Lxi(1) = TEMP_OFFSET ;
}
#define RV_TEMPOFFSET   rvTEMPOFFSET
#endif
#ifndef NUM_ROOMS
#define NUM_ROOMS     NUM_AS
#endif
#ifndef NUM_ZONES
#define NUM_ZONES     0
#endif
#ifndef NUM_CREEPS
#define NUM_CREEPS    0
#endif
static void rvASCONFIG( void ) {
  Lxb(0) = NUM_AS ;
  Lxb(1) = NUM_ROOMS ;
  Lxb(2) = NUM_ZONES ;
  Lxb(3) = NUM_CREEPS ;
}
/* static void rvICRAW( void ) {
  if (Lxb(0) >= NUM_ADCCHANS) lsm = LSm(NOVALUE) ;
  else {
    Lxb(1) = DevAddr(lanAddr, Lxb(0)) ;
    Lxw(1) = IOReading(Lxb(0)) ;
  } ;
}
static void rvOCACTUAL( void ) {
  if (Lxb(0) >= NUM_IOCHANS) lsm = LSm(NOVALUE) ;
  else {
    Lxb(1) = DevAddr(lanAddr, Lxb(0)) ;
    Lxb(2) = OCType(Lxb(0)) ;
    Lxb(3) = OCLevel(Lxb(0)) ;
  } ;
} */
#define RV_ICRAW			NULL
#define RV_OCACTUAL		NULL

#ifndef SV_ASM
#define SV_ASM    NULL
#endif
#ifndef RV_ASM
#define SLAN_ASM
#include "asms.h"
static void rvASM( void ) {
	switch (Lxb(0)) {
  case ASM_ALLALARMS: Lxi(1) = alarmFlags ; break ;
	default:
	#ifdef LANSTATS
		if ((iTmp.w = Lxb(0) - ASM_LANSTATS) < sizeof(lst) / sizeof(word))
			Lxi(1) = ((word *) &lst)[iTmp.w] ;
		else
	#endif
		if ((iTmp.w = Lxb(0) - ASM_BAC_STATS) < sizeof(bst) / sizeof(word))
			Lxi(1) = ((word *) &bst)[iTmp.w] ;
		else
			lsm = LSm(NOVALUE) ;
	} ;
}
#define RV_ASM    rvASM
#endif

#ifndef SV_ASASM
#define SV_ASASM    NULL
#endif
#ifndef RV_ASASM
static void rvASASM( void ) {
#if IN_FBW || IN_WNET
	byte un ;
	nAs++ ;	// change nAs back to incoming airspace value (just to make the numbers below more obvious)
#endif
#if IN_FBW
	// Airspaces return different elements of FBWRec:
	// ASMx/1/N = lbs; ASMx/2/N = wtr; ASMx/3/N = err, where N = Bin/FBW address
	// ASMx/4/A = fbwPkt as word[A]		(get current packet)
	// ASMx/5/A = fbwPkt as byte[A]
	un = Lxb(0) - 1 ;
	if (nAs <= ASM_FBWAS_ERROR && (un >= MAX_FBWUNITS || fbwRec[un].timeout == 0))
		lsm = LSm(NOVALUE) ;
	else {
		switch (nAs) {
		case ASM_FBWAS_WLBS: Lxw(1) = fbwRec[un].w_lbs ; break ;
		case ASM_FBWAS_WKGS: Lxw(1) = MulDivU(fbwRec[un].w_lbs, 10000, 22046) ; break ;
		case ASM_FBWAS_WATER: Lxw(1) = fbwRec[un].wtrCount ; break ;
		case ASM_FBWAS_ERROR: Lxw(1) = fbwRec[un].errorStatus ; break ;
		case ASM_FBWAS_WORD: 
			Lxw(1) = ((word *) &fbwPkt)[Lxb(0)] ;	// FBW packet as words
			break ;
		case ASM_FBWAS_BYTE: 
			Lxw(1) = ((byte *) &fbwPkt)[Lxb(0)] ;	// FBW packet as words
			break ;
		default:
			lsm = LSm(NOVALUE) ;
		} ;
	} ;
#else
	lsm = LSm(NOVALUE) ;
#endif
} 
#define RV_ASASM    rvASASM
#endif

typedef struct {
	byte bacmap ;		// true if incoming message maps to BACnet
  void (*retVal)( void ) ; void (*setVal)( void ) ;
} TRxHandler ;
const TRxHandler handleRx[] = {
  /* LSV_NULL */          { 0, NULL, NULL },
  /* LSV_MEMBYTE */       { 0, rvMEMBYTE, svMEMBYTE },
  /* LSV_MEMWORD */       { 0, rvMEMWORD, svMEMWORD },
  /* LSV_EEBYTE */        { 0, rvEEBYTE, svEEBYTE },
  /* LSV_EEWORD */        { 0, rvEEWORD, svEEWORD },
  /* LSV_OCLEVEL */       { 1, RV_OCLEVEL, SV_OCLEVEL },
  /* LSV_ICLEVEL */       { 0, RV_ICLEVEL, NULL },
  /* LSV_DICAMID */       { 0, rvDICAMID, NULL },
  /* LSV_APPTYPE */       { 0, rvAPPTYPE, NULL },
  /* LSV_ALARMS */        { 0, RV_ALARMS, SV_ALARMS },
  /* LSV_DATETIME */      { 0, NULL, NULL },
  /* LSV_TEMPOFFSET */    { 0, RV_TEMPOFFSET, NULL },
  /* LSV_CURVEPOINT */    { 0, NULL, NULL },
  /* LSV_ASCONFIG */      { 0, rvASCONFIG, NULL },
  /* LSV_ICRAW */         { 1, RV_ICRAW, NULL },
  /* LSV_OCACTUAL */      { 1, RV_OCACTUAL, NULL },
  /* LSV_ASM */           { 0, RV_ASM, SV_ASM },
#define MAX_SYSLSV	LSv(ASM)
  /* LSV_ASASM */         { 1, RV_ASASM, SV_ASASM }
#define IDX_ASASM	(MAX_SYSLSV+1)
} ;

void HandleRxMsg( void ) {
  LanGetRx() ;    	/* lanMsg -> top full msg in queue */
  lsm = Lx(LSM) ;    /* get message category */
  nAs = lsm & 0x0f ;
  lsm &= 0xf0 ;
  lsv = Lx(LSV) ;
  if ((msgUnit = Lx(RXADDR)) && msgUnit != lanAddr) {   /* not for us */
  #ifdef LANT_HANDLENONRX
    LANT_HANDLENONRX ;
  #endif
    goto rxDone ;
  } ;
  msgUnit = Lx(TXADDR) ;
#ifdef LANT_HANDLERXALL
  LANT_HANDLERXALL ;
#endif
  if (lsm <= LSm(NOVALUE)) goto rxDone ;
  if (lsm == LSm(VALUE)) {
    nAs-- ;
    handleRxValue() ;
    goto rxDone ;
  } ;

  /* else - return, broadcast or set: check valid lsv/nAs first */
  /* convert lsv to handler index */
  if (lsv <= MAX_SYSLSV) {
    if (nAs) lsv = 0 ;   /* no a/s for system lsv's */
    /* handler index = lsv */
  }
  else if (lsv == LSV_ASASM) {
  	lsv = IDX_ASASM ;
  }
  else {  /* lsv >= MAX_SYSLSV */
    lsv = 0 ;
  } ;
  nAs-- ;   /* change to a/s index for a/s ops */

	if (handleRx[lsv].bacmap) {		// Mapped BACnet messages are passed intact to BACnet client management
		BACMap(lanMsg) ;
		goto rxDone ;
	} ;

  if (lsm == LSm(SET) || lsm == LSm(SETRET)) {
    if (lsm == LSm(SETRET)) lsm = LSm(RETURN) ;
    if (handleRx[lsv].setVal) (handleRx[lsv].setVal)() ;
    else lsv = 0 ;
  } ;
  if (lsm == LSm(RETURN) || lsm == LSm(BROADCAST)) {
    /* if (LanRqFull()) goto rxDone ; no rq buf available */
    *rqMsg = *lanMsg ;                /* copy rx to rq */
    lanMsg = rqMsg ;                  /* operate on rq buf */
    Lx(RXADDR) = lsm == LSm(BROADCAST) ? 0xff : Lx(TXADDR) ;
    Lx(TXADDR) = 0 ;
    lsm = LSm(VALUE) ;
    if (handleRx[lsv].retVal) (handleRx[lsv].retVal)() ;
    else lsm = LSm(NOVALUE) ;
    nAs++ ;
    Lx(LSM) = lsm | nAs ;
    LanAddRq() ;
  } ;

rxDone:
  LanRxDone() ;
}


