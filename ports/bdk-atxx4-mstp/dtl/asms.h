/* Application Specific Message (ASM) specifiers */

#ifdef FSC_ASM
/* FSC system messages: */
#define ASM_FANVOL          0   /* Mean room fan volume, 100cfm */
/* FSC airspace messages: */
#define ASM_TMEAN10         0   /* Mean temp in 0.01C */
#define ASM_SETTEMP         1   /* Set temp in 0.1C */
#define ASM_VARSET          2   /* Rate limited set temp in 0.1C */
#define ASM_MINVENT         3   /* Min vent level % */
#define ASM_FRESHLEVEL      4   /* Freshen level % */
#define ASM_ALARMS          5   /* Room alarm status (bit-mapped) */
#define ASM_DAYNUM          6   /* Room curve day number (0 for off) */
#define ASM_PMETER          7   /* Pulse meter total count (unscaled) */
#define ASM_TMETER          8   /* Timer meter total count (unscaled) */
#define ASM_METER           7   /* Start of meters for airspace */
#define ASM_AUTOMIN         15  /* Auto-min reduced min-vent level % */
#define ASM_ANIMALS         16  /* Number of animals (pigs/hogs) in airspace */
#define ASM_AQMEAN          17  /* Mean AQ in 0.1AQ */
#endif

#ifdef MSS_ASM
/* MSS system messages: */
#define ASM_EXTTEMP         3   /* External temp in 0.1C */
/* MSS airspace messages: */
#define ASM_TMEAN10         0   /* Mean temp in 0.01C */
#define ASM_SETTEMP         1   /* Set temp in 0.1C */
#define ASM_VARSET          2   /* Rate limited set temp in 0.1C */
#define ASM_SETRH           3   /* Set RH in % */
#endif

#ifdef NVS_ASM
/* NVS airspace messages: */
#define ASM_TMEAN10         0   /* Mean temp in 0.01C */
#define ASM_SETTEMP         1   /* Set temp in 0.1C */
#define ASM_VARSET          2   /* Rate limited set temp in 0.1C */
#define ASM_MINVENT         3   /* Min vent level % */
#define ASM_FRESHLEVEL      4   /* Freshen level % */
#define ASM_VENTLEVEL       5   /* Current vent level % */
#define ASM_ERRSUM          6   /* Integral of absolute error in 0.1C */
#define ASM_DVENTSUM        7   /* Integral of absolute vent level changes % */
#define ASM_ALARMS          8   /* Room alarm status (bit-mapped) */
#define ASM_DAYNUM          9   /* Room curve day number (0 for off) */
#define ASM_ANIMALS         10  /* Number of animals (pigs/hogs) in airspace */
#endif

#ifdef NVZ_ASM
/* NVZ airspace messages: */
#define ASM_TMEAN10         0   /* Mean temp in 0.01C */
#define ASM_SETTEMP         1   /* Set temp in 0.1C */
#define ASM_VARSET          2   /* Rate limited set temp in 0.1C */
#define ASM_MINVENT         3   /* Min vent level % */
#define ASM_FRESHLEVEL      4   /* Freshen level % */
#define ASM_ALARMS          5   /* Room alarm status (bit-mapped) */
#define ASM_DAYNUM          6   /* Room curve day number (0 for off) */
#define ASM_PMETER          7   /* Pulse meter total count (unscaled) */
#define ASM_TMETER          8   /* Timer meter total count (unscaled) */
#define ASM_METER           7   /* Start of meters for airspace */
#define ASM_ANIMALS         16  /* Number of animals (pigs/hogs) in airspace */
#define ASM_ERRSUM          17  /* Integral of flap error in 0.1C */
#define ASM_DVENTSUM        18  /* Integral of absolute flap level changes % */
#endif

#ifdef SFC_ASM
/* SFC/AVC airspace messages: */
#define ASM_TMEAN10         0   /* Mean temp in 0.01C */
#define ASM_SETTEMP         1   /* Set temp in 0.1C */
#define ASM_VARSET          2   /* Rate limited set temp in 0.1C */
#define ASM_MINVENT         3   /* Min vent%/volume/vol-per-bird */
#define ASM_FANSTAGE        4   /* Current fan stage number */
#if IN_AVC
/* AVC only: */
#define ASM_FANVOL          5   /* Current fan volume */
#define ASM_TARGETVOL       6   /* PID target volume */
#if IN_AVCP
/* AVC poultry only: */
#define ASM_NUMBIRDS        7   /* Number birds in 100s */
#endif
/* All SFC/AVC */
#endif
#define ASM_CURVEDAY        10  /* Curve day number (From: 009-105) */
#define ASM_ALARMS          11  /* Room (system) alarm status (bit-mapped) */
#define ASM_METER           12  /* Start of meters by device (unscaled) */
#endif

#ifdef CCR_ASM
/* CCR system messages */
#define ASM_VANESTATS				0		/* Vane stats start here */
/* CCR airspace messages: */
#define ASM_TMEAN           0   /* Mean temp in 0.1C */
#define ASM_AQMEAN          1   /* Mean AQ in 0.1 */
#define ASM_SETAQ           2   /* Set AQ in 0.1 */
#define ASM_MINTEMP         3   /* Set Min temp in 1C */
#define ASM_MAXTEMP         4   /* Set Max temp in 1C */
#define ASM_MINSPEED        5   /* Set fan Min speed % */
#define ASM_MINTIME         6   /* Set Min fan on time % (of cycle) */
#define ASM_AQERRSUM        7   /* Current air quality error integral */
#define ASM_VENTRATE        8   /* Current ventilation rate target %x10 */
#endif

#ifdef PSC_ASM
/* PSC airspace messages (1 - available for PSC1): */
#define ASM_TCROP           0   /* Crop mean temp in 0.1C */
#define ASM_TSTORE          1   /* Store mean temp in 0.1C (1) */
#define ASM_TEXT            2   /* External mean temp in 0.1C (1) */
#define ASM_TDUCT           3   /* Duct mean temp in 0.1C */
#define ASM_TCROPBASE       4   /* Crop base mean temp in 0.1C */
#define ASM_TCROPTOP        5   /* Crop top mean temp in 0.1C */
#define ASM_TCROPMIN        6   /* Minimum crop sensor reading in 0.1C */
#define ASM_TCROPMAX        7   /* Maximum crop sensor reading in 0.1C */
#define ASM_RHEXT           8   /* External RH in % */
#define ASM_DPEXT           9   /* External Dewpoint temp in 0.1C */
#define ASM_VENTSTATUS     10   /* Ventilation status flags (1) */
#define ASM_FRIDGESTATUS   11   /* Refrigeration status flags (1) */
#define ASM_RECIRCSTATUS   12   /* Recirculation status flags */
#define ASM_TSTORETARGET   13   /* Store air target temp in 0.1C */
#define ASM_TDUCTTARGET    14   /* Duct target temp in 0.1C */
#define ASM_TSET           15   /* User: Crop set temp in 0.1C (1) */
#define ASM_TMINWH         16   /* User: Wound healing air temp in 0.1C */
#define ASM_TMIN           17   /* User: Frost point air temp in 0.1C (1) */
#define ASM_TDUCTMIN       18   /* User: Duct min temp in 0.1C */
#define ASM_TEXTOFFSET     19   /* User: Min crop-ambient differential in 0.1C */
#define ASM_MODE           20   /* User: Operating mode */
#define ASM_USEDEVS        21   /* User: Device selection (1) */
#define ASM_DEVICEMODE     22   /* Device mode bitmap (used by DCS) */
#define ASM_BULBTSET			 23		/* Bulb: Set point 1C */
#define ASM_BULBRHSET			 24		/* Bulb: RH set point 1% */
#endif

#ifdef GSC_ASM
/* GSC airspace messages: */
#define ASM_MAXRH           0   /* Maximum duct RH */
#define ASM_TARGETRH        1   /* Target duct RH */
#define ASM_DEVSON          2   /* Maximum output device currently on */
#define ASM_DHDROP          3   /* Measured external-duct RH drop */
#define ASM_DEBUG           20  /* Room data by address offset */
#endif

#ifdef MON_ASM
/* MON/LGR (and Slave-I/O) system messages: */
#define ASM_ALLALARMS       0   /* Network (MON only) or local alarm status */
#define ASM_TIMESTAMP       1   /* Day of month and hour combined: DDHH */
#define ASM_LANSTATS        2   /* Network performance statistics */
#define ASM_BAD0X           50  /* Cumulative number false zero-crossings */
#define ASM_SENS1NOISE      51  /* Sensor chan 1 noise level ... */
#define ASM_SENS8NOISE      58  /* Sensor chan 8 noise level */
#define ASM_AQ1							60	/* AQ rdg 1 ... */

#define ASM_SPTSTATS0		 	 	70	/* SPT2 driver stats array */
#define ASM_USER0						80	/* User ASMs start */

#define ASM_WINDMEAN				120	/* 15min wind-speed mean reading */
#define ASM_WINDMIN					121	/* 15min wind-speed min reading */
#define ASM_WINDMAX					122	/* 15min wind-speed max reading */

/* Truckmon Airspace (zone) messages */
#define ASM_ZMODE						0		/* Zone mode */
#define ASM_ZPIGS						1		/* Zone pigs */
#define ASM_ZALARMS					2		/* Zone alarm status (bitmap) */
#define ASM_ZALARMLEVEL			3		/* Zone alarm level 0=off, 1=yellow, 2=red (bitmap) */
#define ASM_ZLEDLEVEL				4		/* Zone LED level % */
#define ASM_ZTEMP						5		/* Zone temperature reading */
#define ASM_ZRH							6		/* Zone RH reading */
#endif

#ifdef DCS_ASM
/* Duct control system: system messages (based on MON/LGR) : */
#define ASM_ALLALARMS       0   /* Network (MON only) or local alarm status */
#define ASM_TIMESTAMP       1   /* Day of month and hour combined: DDHH */
#define ASM_LANSTATS        2   /* Network performance statistics */
#define ASM_BAD0X           50  /* Cumulative number false zero-crossings */
#define ASM_SENS1NOISE      51  /* Sensor chan 1 noise level ... */
#define ASM_SENS8NOISE      58  /* Sensor chan 8 noise level */
#define ASM_DCSRAMLEVEL     60  /* Average of ram positions for all ducts */
#endif

#ifdef RMS_ASM
/* RMS system messages: */
#define ASM_VALVESTATS      0   /* Valve control stats */
#define ASM_CONDSTATS				30	/* Condensor control stats */
#define ASM_BAD0X           50  /* Cumulative number false zero-crossings */
#define ASM_SENS1NOISE      51  /* Sensor chan 1 noise level ... */
#define ASM_SENS8NOISE      58  /* Sensor chan 8 noise level */
#define ASM_USERMODE        60  /* User set mode */
#define ASM_OPMODE          61  /* Operating mode (bit-mapped) */
#define ASM_DEFROSTMODE     62  /* Defrost sub-mode (bit-mapped) */
#define ASM_DEVICEMODE      63  /* Device mode (bit-mapped) */
#define ASM_ICELEVEL        64  /* Estimated ice level 0-100% */
#define ASM_COMPTIME        65  /* Compressor run mins */
#define ASM_EVAPTIME        66  /* Evap fan run mins */
#define ASM_AUTOSET         67  /* Auto mode set-temp 0.1C */
#define ASM_ICINGTEMP				68	/* RMS2 Icing temp 0.1C */
#endif

#ifdef FMX_ASM
/* FMX airspace messages: */
#define ASM_ARATIO          0   /* Current A-auger ratio % */
#define ASM_DAYNUM          1   /* Curve day number */
#define ASM_SWITCHES        2   /* Level switch status (bit-mapped) */
#endif

#ifdef MMX_ASM
/* MMX system messages (based on MON/LGR): */
#define ASM_ALLALARMS       0   /* Network (MON only) or local alarm status */
#define ASM_TIMESTAMP       1   /* Day of month and hour combined: DDHH */
#define ASM_LANSTATS        2   /* Network performance statistics */

#define ASM_WATERTEMP				10	/* Actual water temp (10C) */
#define ASM_TOTALPOWDER			11	/* Cumulative powder run (excluding calibration) */
#define ASM_TOTALWATER			12	/* Cumulative water run (excluding calibration) */
#define ASM_HEATERSECS			13	/* Cumulative heater run */
#define ASM_MIXES						14	/* Total mixes started */
#define ASM_WHISKSECS				15	/* Cumulative whisk run */
#define ASM_EMPTYRDG				16	/* Last valid empty reading (raw sensor val) */

#define ASM_POWDERGRAMS			17	/* Cumulative grams figure */
#define ASM_FPHMEAN1				18	/* Rolling feed per head, mean yesterday, 0.1g/hr */
#define ASM_FPHMEAN2				19	/* Rolling feed per head, mean today, 0.1g/hr */
#define ASM_FPHCHANGE				20	/* Feed per head change: today as % of yesterday */
#define ASM_FPHPTS					21	/* Number points in rolling hourly totals */
#define ASM_DAYTOT0					22	/* Last day total feed in 0.01kg */
#define ASM_DAYTOTPH0				23	/* Last day mean feed per head in g */
#define ASM_HOURTOT0				24	/* Last hour mean feed per head 0.1g */
#define ASM_POWDERCAL				25	/* Powder calibration figure: 10ms per kg */
#define ASM_WATERCAL				26	/* Water calibration figure: 10ms per 0.5l */

#define ASM_AUGERTICKS			27	/* Auger ticks per rev, snapshot of last running rev */
#define ASM_AUGERRPM				28	/* Auger rpm (damped) 0.1rpm */

#define ASM_BAD0X           30  /* Cumulative number false zero-crossings */
#define ASM_SENS1NOISE      31  /* Sensor chan 1 noise level ... */
#define ASM_SENS8NOISE      38  /* Sensor chan 8 noise level */

/* Med asms override bad0x etc when med enabled */
#define ASM_MEDMIXES				30	/* Medication mixes rolling total */
#define ASM_MEDSECS					31	/* Medication total secs */
#define ASM_MEDCAL					32	/* Medication calibration, secs per kg */
#define ASM_MEDGRAMS				33	/* Medication total grams (rolling) */

#define ASM_USERSET0				40	/* User settings by byte */
#define ASM_CONFIGSET0			60	/* Config settings by byte */
#define ASM_CONFIGSETEND	  90	/* ... end */

#define ASM_SPTSTATE			 	91	/* SPT2 driver current state */
#define ASM_SMSSENT					92	/* SMS msgs sent (from EE) */
#define ASM_SMSRCVD					93	/* SMS msg received (from EE) */
#define ASM_SPTSTATS0		 	 100	/* SPT2 driver stats array */
#endif

#ifdef SAL_ASM
/* SAL system messages (based on MON/LGR): */
#define ASM_ALLALARMS       0   /* Network (MON only) or local alarm status */
#define ASM_TIMESTAMP       1   /* Day of month and hour combined: DDHH */
#define ASM_LANSTATS        2   /* Network performance statistics */
#define ASM_BAD0X           50  /* Cumulative number false zero-crossings */
#define ASM_SENS1NOISE      51  /* Sensor chan 1 noise level ... */
#define ASM_SENS8NOISE      58  /* Sensor chan 8 noise level */

#define ASM_USER0						60	/* User ASMs start */
#endif

#ifdef SLAN_ASM
/* SLAN system messages (as Mon/Lgr): */
#define ASM_ALLALARMS       0   /* Network (MON only) or local alarm status */
#define ASM_TIMESTAMP       1   /* Day of month and hour combined: DDHH */
#define ASM_LANSTATS        2   /* Network performance statistics */
/* FBW sys messages */
#define ASM_FBW_CHKERRS			20	/* Chksum errors */
#define ASM_FBW_TIMEOUTS		21	/* Timeout errors */
/* FBW Airspace codes for bin weights etc */
#define ASM_FBWAS_WLBS			1		/* ASMu/1/N = Bin weight lbs/20, where N = bin address */
#define ASM_FBWAS_WKGS			2		/* ASMu/2/N = Bin weight kgs/20, where N = bin address */
#define ASM_FBWAS_WATER			3		/* ASMu/3/N = Water count */
#define ASM_FBWAS_ERROR			4		/* ASMu/4/N = Error status */
#define ASM_FBWAS_WORD			5		/* ASMu/5/A = Current bin packet word value, A = word offset */
#define ASM_FBWAS_BYTE			6		/* ASMu/6/A = Current bin packet byte value, A = byte offset */

/* WNET sys messages */
#define ASM_WNET_CHKERRS		20	/* Chksum errors */
#define ASM_WNET_TIMEOUTS		21	/* Timeout errors */
/* WNET Airspace codes for wnet node data */
#define ASM_WNETAS_LQI			1		/* ASMu/1/N = LQI */
#define ASM_WNETAS_RSSI			2		/* ASMu/2/N = RSSI */
#define ASM_WNETAS_BATTVOLTS	3		/* ASMu/3/N = Battery Voltage */
#define ASM_WNETAS_PKTTX		4		/* ASMu/4/N = Packets Sent */
#define ASM_WNETAS_RECON		5		/* ASMu/5/A = Network Reconnections */
#define ASM_WNETAS_UPTIME		6		/* ASMu/6/A = Uptime */
#define ASM_WNETAS_RDG1			7		/* ASMu/7/A = Reading 1 */
#define ASM_WNETAS_RDG2			8		/* ASMu/8/A = Reading 2 */
#define ASM_WNETAS_AGEMINS		9		/* ASMu/9/A = Age mins */
#define ASM_WNETAS_AGESECS		10		/* ASMu/10/A = Age secs */

#define ASM_WNETAS_TXERRS		11		/* ASMu/11/A = Tx Errors */
#define ASM_WNETAS_PARENT		12		/* ASMu/12/A = Parent short address */
#define ASM_WNETAS_SHORTADDR	13		/* ASMu/12/A = Module short address */
/* (airspace value can not be > 15 - see lant.c) */

/* Biovator sys messages */
#define ASM_BIOV_STATE			22	/* Biovator state */
#define ASM_BIOV_TIMER			23	/* Biovator timer (secs) */

#define ASM_BAC_STATS				30	/* BACnet network stats */

#endif

