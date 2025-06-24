/* 
 *  Mode display and DDS definitions
 *  see also DDS.h 
*/
#ifndef DDSDEFS_H_
#define DDSDEFS_H_

// state variable machine for pulse/ramps/exp...
enum pulseState_t {PU_IDLE, PU_LOW, PU_RISE, PU_HIGH, PU_FALL};
// dds operating values. Saved between cycles, not between sessions.
// all operating values in Integer (DAC value) format for speed.
struct ddsVals { 
 // float freq;
  int64_t stepSize; // size of increment - minimum of 1. (max may exceed int32. can be full uint32 value)
  int delaySteps; // samples between increments (long ramps and WA_STEP)
  uint32_t nextStep; // (circular) reference to DDS LUT (+ interpolation) for SIN  
  int curVal; // last used value in a waveForm (could re-use nextStep?)
  int cyclesToGo; // outer cycles, e.g. a rise/fall pulse cycle.
  int samplesToGo; // for timing flat line delays 
  int stepSamplesToGo; // very slow ramps have one step every few samples
  DDSwaves waveForm;
  bool run;       // running or halted. appears unused. 
  pulseState_t state; // state variable machine flag
} dds[2];

#define MINAC     0
#define MAXAC     11.5 // v P-P (11.877 is FSD * 2)
#define MINDC     (-MAXAC/2)   //(-2*DACVMAX)
#define MAXDC     (MAXAC/2)  //(2*DACVMAX)
#define MINDUTY   0.01
#define MAXDUTY   99.99
#define MINTIME   0.00 // mS
#define MINSTEPTIME   0.1 // mS
#define MAXTIME   99999
#define MINRISE   0.0
//#define MAXPULSE  300000 //  mS
#define MAXRISE   99999
#define MINCYC    -1	    // continue forever
//#define MAXCYC	  50000
#define MINFREQ	  0.01
#define MAXFREQ   70000
#define MINPULFREQ 0.01
#define MAXPULFREQ 20000
#define MINPH	    0
#define MAXPH	    (359.99)
#define MINSTEPS  1
#define MAXSTEPS 10000


#define MODENAMELEN 16
#define MODETEXTLEN 16
#define MODEFMTLEN 16

#define MODEVALUES 8 // number of values per mode
#define MODESETVALS 7  // number of slots in setVals & buttons - may be less than the full number of MODEVALUES
#define XITEMS 3    // sweep burst and control are at end of arrays

#define AMODES 7  // legit modes for Chan A
#define BMODES 7  // Chan B
#define SWEEP_VALS 7  

#define SINCORLEN 5
struct sineCorrect
{
  float freq;
  float factor;
}
  sineCor[SINCORLEN] = {{0,1.0},{6000, 1.0},{45000, 1.037}, {60000, 1.088}, {MAXFREQ + 10, 1.13}};
// parameter position definitions for DDS routines
enum sineVal_t    {SN_FR, SN_AM, SN_DC};

enum whiteVal_t   {WH_AM, WH_DC};
enum imdVal_t     {IM_F1, IM_F2, IM_A1, IM_A2};

enum pulseVal_t   {PU_V1, PU_V2, PU_TL, PU_TR, PU_TH, PU_TF};
enum stepVal_t    {ST_V1, ST_V2, ST_SU, ST_SD, ST_TU, ST_TD, ST_RE}; 
enum sqrVal_t     {SQ_FR, SQ_V1, SQ_V2, SQ_DU}; // SQ_DC
enum triVal_t     {TR_FR, TR_V1, TR_V2, TR_DU}; // TR_DC

//enum burstVal_t   {BU_ON, BU_OFF, BU_CY, BU_BA};
enum burstVal_t   {BU_ON, BU_OFF, BU_BALTA, BU_REP};

enum sweepVal_t   {SW_TYPE, SW_INI, SW_FIN, SW_TIM, SW_STE, SW_LOG, SW_REP};
enum sweepCurve_t {SWP_LIN, SWP_LOG};

enum controlType_t {CP_BA, CP_PH, CP_ET, CP_INPOL, CP_OUTPOL};  // Ext Trig also in here for convenience. CP_PH in degrees

#define MULTI_OFF   0
#define MULTI_BURST 1
#define MULTI_SWEEP 2 

uint16_t modeEnabled[CHANS+XITEMS] = {0x7f,0b1111001, 0x0, 0x07};// bitmap of enabled modes for channels A/B, Control, Burst/Sweep  
uint16_t modeCount[CHANS] = {MODENUM, MODENUM}; // A & B may have different number of possible modes

// all parameters are stored as float
// milliseconds,  Hz, Volts, degrees
// > SMALL_VAL = true
// integer values will be 'floor' not rounded

// defs.h: enum DDSwaves {WA_SINE, WA_IMD, WA_WHITE, WA_SQR, WA_TRI, WA_STEP, WA_PULSE, WA_OFF};
// save to EEPROM
#define MODMUL (CHANS * (MODEITEMS + XITEMS) * MODEVALUES)
#define TRIG_OUT_POL  (modeVal[CHAN_A][WA_CONT][CP_OUTPOL]  > 0.01) // float value
#define TRIG_IN_POL   (modeVal[CHAN_A][WA_CONT][CP_INPOL]  > 0.01)
#define TRIG_EN       (modeVal[CHAN_A][WA_CONT][CP_ET] > 0.01)
#define BCOUPLE       (modeVal[CHAN_A][WA_CONT][CP_BA] > 0.01) // B=A
#define BPHASE        (modeVal[CHAN_A][WA_CONT][CP_PH]) // Control Phase
#define BALTA         (modeVal[CHAN_A][WA_BURST][BU_BALTA] > 0.01)
#define WAVE_A        (iSet.waveForm[0])
#define BURST_MODE    (iSet.modeA == MO_BURST)
#define SWEEP_MODE    (iSet.modeA == MO_SWEEP)

float modeVal[CHANS][MODEITEMS + XITEMS][MODEVALUES] =  
{
  { // CHAN A
    {1000,1,0,0,0,0,0,0},   //WA_SINE {SN_FR, SN_AM, SN_DC}
    {60,7000,1,.25,0,0,0,0}, //WA_IMD  {IM_F1, IM_F2, IM_A1, IM_A2}
    {1,0,0,0,0,0,0,0},  //WA_WHITE {WH_AM, WH_DC}
    {1000,0,1,50,0,0,0,0}, // WA_SQR  {SQ_FR, SQ_V1, SQ_V2, SQ_DU}
    {1000,0,1,50,0,0,0,0}, // WA_TRI     {TR_FR, TR_V1, TR_V2, TR_DU}
    {0,1,10,10,10,10,0,0}, //WA_PULSE {PU_V1, PU_V2, PU_TL, PU_TR, PU_TH, PU_TF} 
    {0,1,5,5,100,100,0,0}, // WA_STEP   {ST_V1, ST_V2, ST_SU, ST_SD, ST_TU, ST_TD} 
    {0,100, 100,5,5,1,0,1}, // SWEEP DUMMIES - actual vals in sweepVal[] 
    {10,5,0,1,4,5,0,0}, //BURST  {BU_ON, BU_OFF, BU_BALTA, BU_REP};
    {0,180,0,1,1,0,0,0} //CONTROL  {CP_BA, CP_PH, CP_ET, CP_INPOL, CP_OUTPOL}
  }, // CHAN B  modeVal[0][WA_CONT][CP_OUTPOL]
  { 
    {500,1,0,0,0,0,0,0}, // sin
    {0,0,0,0,0,0,0,0}, //imd - not used - chan A only
    {1,2,0,0,0,0,0,0}, //wht
    {500,0,2,50,0,0,0,0}, //sqr
    {500,0,2,50,0,0,0,0}, //tri
    {0,2,10,10,10,10,0,0}, // pul
    {0,2,5,5,100,100,0,0}, //step
    {0,0,0,0,0,0,0,0}, //UNUSED - Sweep Chan A only
    {0,0,0,0,0,0,0,0}, //UNUSED - Burst Chan A only
    {0,0,0,0,0,0,0,0} //Control - Cont Chan A only
  }
}; 

// immediate DDS values copied from modeVal, sweepVal, etc.
struct sweeps sweep = {1000,5000,10,5, SWM_FR, false, false, false}; // dummies copied from core mode. bool == linear, hold, repeat  
struct bursts burst = {5,5,0,false,false,B_IDLE};
struct whitex white[CHANS]; // immediate white noise values

struct sweepLims {
  float min[SW_MODES]; // SW_MODES:FAD
  float max[SW_MODES];
  char units[SW_MODES][UNITLEN];
  int options; // number of modes allowed V, VF or VFD
} sweepLim[MODEITEMS] = { // special case variables for sweep and other type 'V' setVars
  {{0,MINFREQ,0}, {MAXDC,MAXFREQ,0}, { "V", "Hz",""}, 2}, // SINE - F & V
  {{0,0,0}, {0,0,0}, {"","", ""}, 0}, // IMD
  {{0,0,0}, {0,0,0}, {"","", ""}, 0}, // WHITE
  {{MINDC,MINFREQ,MINDUTY}, {MAXDC,MAXFREQ,MAXDUTY}, {"V","Hz",  "%"}, 3}, // SQR
  {{MINDC,MINFREQ,MINDUTY}, {MAXDC,MAXFREQ,MAXDUTY}, {"V","Hz",  "%"}, 3}, // TRI
  {{MINDC,0,0}, {MAXDC,0,0}, {"V","", ""}, 1},  // PULSE - V only
  {{MINDC,0,0}, {MAXDC,0,0}, {"","", ""}, 0} // STEP - Nove

};

// save to EEPROM
struct sweepVals {  
  float val[SW_MODES][SWEEP_VALS]; // 3 opts x 7 vals. First item MUST be sweep type [0..2]
  /* {SW_TYPE, SW_INI, SW_FIN, SW_TIM, SW_STE, SW_LOG, SW_REP};
  float type[SW_MODES] // dummy - used in webCommand returns
  float init[SW_MODES];
  float final[SW_MODES];
  float time[SW_MODES];
  float steps[SW_MODES];
  float log[SW_MODES];
  float repeat[SW_MODES];
  */
} 
// sweep type is stored in sweepVal[wave].val[0][0], val [0]
//{SW_TYPE, SW_INI, SW_FIN, SW_TIM, SW_STE, SW_LOG, SW_REP};
sweepVal[MODEITEMS] = {  // special case values for sweep (type 'V') setVars
  {{ {1,1,2,5,5,0,1},{0,100,500,5,5,0,1},{2,0,0,0,0,0,0}} }, // SINE - FA only 
  {{ {1,0,0,0,0,0,0},{0,0,0,0,0,0,0},{2,0,0,0,0,0,0}} },           // IMD - none
  {{ {0,0,0,0,0,0,0},{1,0,0,0,0,0,0},{2,0,0,0,0,0,0}} },             // WHITE - none
  {{ {1,1,2,5,5,0,1},{0,200,600,5,5,0,1},{2,10,90,5,5,0,1}} }, // SQR - FAD
  {{ {1,1,2,6,6,0,1},{0,300,700,5,5,0,1},{2,10,90,5,5,0,1}} }, // TRI - FAD
  {{ {1,1,2,5,5,0,1},{0,0,0,0,0,0,0},{2,0,0,0,0,0,0}} },           // PULSE - V only
  {{ {1,0,0,0,0,0,0},{0,0,0,0,0,0,0},{2,0,0,0,0,0,0}} }            // STEP  - None
};

// display info and value limits for the various modes
struct modes 
{
	char name[MODENAMELEN]; // mode name
	char text[MODEVALUES][MODETEXTLEN];	// setting name
	char units[MODEVALUES][MODETEXTLEN]; // setting units
  char fmt[MODEVALUES]; // setting type
  int post[MODEVALUES]; // no of decimals to display
  int clr[MODEVALUES]; // needs a # prepended and 6 digit hex rendering for HTML colour code
	float lowLim[MODEVALUES];
	float highLim[MODEVALUES];
	int	nValues;	// number of values (MODEVALUES) enabled 	
} modeDefs[MODEITEMS+XITEMS] = 
// defs.h: enum DDSwaves {WA_SINE, WA_IMD, WA_WHITE, WA_SQR, WA_TRI, WA_STEP, WA_PULSE, WA_OFF};
{ 
	{ "Sine", 
    {"Freq","Amp","DC off","","","","",""}, 
  	{"Hz","V","V","","","","",""}, 
  	{'F','F','F',' ',' ',' ',' ',' '},
    {2,2,2,0,1,0,0,0},
    {COLFREQ,COLVOLT,COLVOLT,0,0,0,0,0}, 
		{MINFREQ,0,MINDC,0,0,0,0,0},
		{MAXFREQ,MAXAC,MAXDC,0,0,0,0,0}, 3
	},  
  { "IMD", 
   {"Freq 1","Freq 2","Amp 1","Amp 2","","","",""}, 
   {"Hz","Hz","V","V","","","",""}, 
   {'F','F','F','F',' ',' ',' ',' '},
   {2,2,2,2,0,0,0,0},
   {COLFREQ,COLFREQ,COLVOLT,COLVOLT,0,0,0,0}, 
   {MINFREQ,MINFREQ,0,0,0,0,0,0},
   {MAXFREQ,MAXFREQ,MAXAC,MAXAC,0,0,0,0}, 4
  }, 
  { "White", 
   {"Ampl","DCoff","","","","","",""}, 
   {"V","V","","","","","",""}, 
   {'F','F',' ',' ',' ',' ',' ',' '},
   {2,2,0,0,0,0,0,0},
   {COLVOLT,COLVOLT,0,0,0,0,0,0}, 
   {MINAC,MINDC,0,0,0,0,0,0},
   {MAXAC,MAXDC,0,0,0,0,0,0}, 2
  }, 
  { "Square", 
    {"Freq","V Low","V High","Duty","","","",""}, 
    {"Hz","V","V","%","","","",""},   
    {'F','F','F','F',' ',' ',' ',' '},
    {2,2,2,2,0,0,0,0},
    {COLFREQ,COLVOLT,COLVOLT,COLTIMPH,0,0,0,0}, 
    {MINPULFREQ,MINDC,MINDC,MINDUTY,0,0,0,0},
    {MAXPULFREQ,MAXDC,MAXDC,MAXDUTY,0,0,0,0}, 4
  }, 
  { "Triangle", 
    {"Freq","V Low","V High","Duty","","","",""}, 
    {"Hz","V","V","%","","","",""},   
    {'F','F','F','F',' ',' ',' ',' '},
    {2,2,2,2,0,0,0,0},
    {COLFREQ,COLVOLT,COLVOLT,COLTAU,0,0,0,0}, 
    {MINPULFREQ,MINDC,MINDC,MINDUTY,0,0,0,0},
    {MAXPULFREQ,MAXDC,MAXDC,MAXDUTY,0,0,0,0}, 4
  },  
  	{ "Pulse",  
    {"V Low","V High","T Low","T Rise","T High","T Fall","",""}, 
  	{"V","V","mS","mS","mS","mS","",""},   
  	{'F','F','F','F','F','F',' ',' '},
    {2,2,2,2,2,2,0,0},
    {COLVOLT,COLVOLT,COLTIMPH,COLTIMPH,COLTIMPH,COLTIMPH,0,0}, 
		{MINDC,MINDC,MINTIME,MINTIME,MINTIME,MINTIME,0,0},
		{MAXDC,MAXDC,MAXRISE,MAXRISE,MAXRISE,MAXTIME,0,0}, 6
	}, 
  { "Step", 
    {"V Low","V High","Steps U","Steps D","Time U","Time D","Repeat",""}, 
    {"V","V","","","mS","mS","FT",""},   
    {'F','F','I','I','F','F','R',' '},
    {2,2,0,0,2,2,0,0},
    {COLVOLT,COLVOLT,COLCYCSTEP,COLCYCSTEP,COLTIMPH,COLTIMPH,COLCYCSTEP,0}, 
    {MINDC,MINDC,MINSTEPS,MINSTEPS,MINSTEPTIME,MINSTEPTIME,0,0},
    {MAXDC,MAXDC,MAXSTEPS,MAXSTEPS,MAXTIME,MAXTIME,1,0}, 7
  },
 
  { "Sweep", 
   {"V/F/D","Initial", "Final","Time","Steps","Lin/Log","Rept",""}, //{SW_TYPE, SW_INI, SW_FIN, SW_TIM, SW_LOG, SW_LOG, SW_REP};
   {"VFD","","","S","","Lin Log","FT",""}, 
   {'R','F','F','F','I','V','R',' '},
   {0,2,2,2,0,0,0,0},
   {COLVOLT,COLFREQ,COLFREQ,COLTIMPH,COLCYCSTEP,COLFREQ,COLCYCSTEP,0}, 
   {0,0,0,MINTIME,1,0,0,0}, 
   {2,0,0,MAXTIME,MAXSTEPS,1,1,0}, SWEEP_VALS
  },
   { "Burst", 
    {"Cyc On","Cyc Off","B alt A","Rept","","","",""}, 
    {"","","FT","FT","","","",""}, 
    {'F','F','R','R',' ',' ',' ',' '},
    {2,2,0,0,0,0,0,0},
    {COLVOLT,COLVOLT,COLTIMPH,COLCYCSTEP,0,0,0,0}, 
    {MINSTEPS,MINSTEPS,0,0,0,0,0,0},
    {MAXSTEPS,MAXSTEPS,1,1,0,0,0,0}, 4
  },
   { "Control", 
    {"B=A","B Phase","Ext Trig","T In Pol","T Out Pol","","",""}, 
    {"FT","deg","FT","-+","-+","","",""}, // en dash ?
    {'R','F','R','R','R',' ',' ',' '},
    {0,1,0,0,0,0,2,2},
    {COLFREQ,COLVOLT,COLTIMPH,COLTIMPH,COLTIMPH,0,0,0}, 
    {0,MINPH,0,0,0,0,0,0},
    {1,MAXPH,1,1,1,0,0,0}, 5
  } 
  // no values for WA_NOR
};

#endif