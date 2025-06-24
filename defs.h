#ifndef DEFS_H
#define DEFS_H

#define ILI9488

#ifdef ILI9488
  // need to include this before the pin (re)definitions
 // #include <Arduino_GFX_Library.h>
#endif

#define FILESYS LittleFS
#define _RP2040W_AWS_LOGLEVEL_     4

// definitions
#define CHANS 2 // IMD uses both sine channels
#define LEFT 0    // A
#define RIGHT 1   // B
#define CHAN_A  LEFT
#define CHAN_B RIGHT
#define BU_SW_CHAN 2  //  used for JSON
#define CONTROL_CHAN 3  //  used for JSON
#define POSTLEN 256 // POST message buffer
#define SCPIERRLEN 128 // SCPI retained message buffer
#define MULTIERRORLEN (SCPIERRLEN *2)
 
// DAC samples
//NOTE: (DDSWIDTH + INTSCALE_BITS <= 32) 
// scale arithmetic for all modes is 32 bit int
// scale values are uint_16 (never more than 1.0 as sine table is maxed out)
#define DDSWIDTH 20   // sine sample width - some extra bits for better accuracy
#define INTSCALE_BITS 10  // used for fractional integer multiply (XX bits of fractional part)
#define INTSCALE_MUL (1 << INTSCALE_BITS)

// Sine sample calculations are done in 32 bit integer arithmetic
int32_t DACval[CHANS];
#if SAMPWIDTH == 16
  #define PACKWIDTH 16  // size of sample presented to I2S.write (16 or 32 bits)
  int16_t offVal = 1; // non-zero to defeat auto mute function
  const int32_t bitmask = 0x0ffff; // 16 bits
  #define DDSRSHIFT (DDSWIDTH - PACKWIDTH) // shift down from DDSWIDTH 16 bit arithmetic
  #define DDSWHITESHIFT 0
#else
  #define PACKWIDTH 32  // 32 for 24 bit samples. Shifted up, bottom 8 bits are ignored.
  int32_t offVal = 1; // non-zero to defeat auto mute function
  const int32_t bitmask = 0x0fffff; // 20 bits
  #define DDSLSHIFT (PACKWIDTH - DDSWIDTH) // shift up from DDSWIDTH bit arithmetic
   #define DDSWHITESHIFT 16
#endif  

// for sine and IMD only
#if PACKWIDTH >= 32
  const int32_t maxDAC = INT_MAX; // avoid overflow error message
#else
  const int32_t maxDAC = (1 << (PACKWIDTH -1)) - 1; 
#endif

const int32_t maxVal = (1 << (DDSWIDTH -1)) - 1;

// for pulses and white
const int32_t maxWhiteVal = (1 << 15) -1;// White always calculated as 16 bits.
const int32_t maxPulseVal = maxDAC; 

// phase accumulator
#define DDS_LOOKUP_BITS     12  // masked with DDS_PHASE_BITS then shifted down
#define DDS_INTER_BITS      18  // if total < 32 bits, don't have to worry about overflow??
#define DDS_TAB_LEN         (1 << DDS_LOOKUP_BITS)  // 2 ^ DDS_LOOKUP_BITS
#define DDS_INTER_MUL       (1 << DDS_INTER_BITS)
#define DDS_INTER_MASK      ((1 << DDS_INTER_BITS) - 1)
#define DDS_PHASE_BITS      (DDS_LOOKUP_BITS + DDS_INTER_BITS)
#define DDS_PHASE_MASK      ((1 << DDS_PHASE_BITS) - 1)

#define DMABUFS   5     // 5 x 512 ???
#define DMABUFLEN 1024 // longer buffers seem better for glitches
#define DMA_TOT (DMABUFS * DMABUFLEN)
#define MUTE_DELAY (1 + DMA_TOT/96) // mS

//#define MAX_TRIG_FREQ 5000  // should be 50

// TFT/Touch SPI pin defs also in arduino\libraries\TFT_eSPI\User_Setup.h
// SPI speeds also there
#if (HARD_VERSION >= 3) //
  #define BOARDVER  3
  #define AMP_MUL   2 // buffer has gain of approx 2. vCal sorts out exact value

  // I2S pins 
  #define pDOUT     18
  #define pBCLK     19        
  #define pWS       (pBCLK+1) // 20 LRCLK
  #define MCLK_MUL  256 // not needed on PCM5102

  // TFT touch pins
  #define TFT_MISO  12
  #define TFT_SCLK 	14
  #define TFT_MOSI 	15

  #define TFT_CS 		6
  #define TFT_RST 	7 
  #define TFT_DC 	  8

  //#define T_IRQ 		255
  #define TOUCH_CS 		13  // do not change name: detected by TFT_eSPI.h 

  // I2C  
  #define SDA1PIN 10
  #define SCL1PIN 11
  #define myWire Wire1
  #define I2CSPEED 400000

  // encoder and buttons
  #define ENC_A 		2  
  #define ENC_B 		1  
  #define ENC_SW    0
  #define L_BUT     4   // highlight digit change buttons (Scrubber knob) 
  #define R_BUT     3
  #define A_BUT    16   // channel on/off
  #define B_BUT    17
  
  //other pins
  #define TRIG_OUTPIN   27 
  #define LEDA  21
  #define LEDB  22
#ifdef SWAP_LED_TW
  #define LEDT   5      
  #define LEDW  TRIG_OUTPIN
#else
  #define LEDT          TRIG_OUTPIN
  #define LEDW  5
#endif
  #define TRIG_INPIN    28
  //#define MUTEPIN       26
  #define SDAPIN        10
  #define SCLPIN        11
  
#else // Mk I
  #define BOARDVER    1
  #define AMP_MUL     1
  // I2S pins 
  #define pDOUT       22
  #define pBCLK       20        
  #define pWS         (pBCLK+1) 
  //  LRCLK           21
  #define MCLK_MUL    256 // not needed on PCM5102

  // TFT touch pins
  #define TFT_SCLK 	  18 //  Match to User_setup.h in TFT-eSPI GRAPHICS LIBRARY
  #define TFT_MISO 	  16 // TFT-eSPI doesn;t like this being called TFT_MISO
  #define TFT_MOSI 	  19

  #define TFT_CS 		  12
  #define TFT_DC 	    14
  #define TFT_RST 	  13 

  //#define T_IRQ 		255
  #define TOUCH_CS 	  15  // do not change name: detected by TFT_eSPI.h 

  // encoder and buttons
  #define ENC_A 		1  
  #define ENC_B 		2  
  #define ENC_SW    0
  #define L_BUT     4 // highlight digit change buttons 
  #define R_BUT     3
  #define A_BUT    8  // channel on/off
  #define B_BUT    9

  //other pins
  #define TRIG_OUTPIN   27
  #define TRIG_INPIN    28
 // #define MUTEPIN       26

  #define SDAPIN 10
  #define SCLPIN 11
#endif

#define PSPIN 23  // turn power save off (HIGH) to reduce switcher 8kHz noise (4dB improvement under low loads)

#define SCR_WD  TFT_HEIGHT // defined in driver library
#define SCR_HT  TFT_WIDTH 
#ifdef ILI9341
	#define HMAX 240
	#define VMAX 320
	#define SCREENROT 0 // SCREEN rotation default
	#define TOUCHROT 0
#else // ILI9488
	#define HMAX 	320// not used everywhere. when using sx() 320 is max
	#define VMAX 480     // not used everywhere. when using sy() 240 is max
	#define SCREENROT 0 // SCREEN rotation default
	#define TOUCHROT 3
#endif

#define SAVE_EE_AFTER 30
// callback routines  should contain non-blocking code (e.g. state machines)
// if blocking code is required, then yield() should be issued regularly to allow background (e.g. WiFi) tasks to process

//  callback routine COMMAND codes
#define CALL_CMD_START  1 // start cmd (first call)
#define CALL_CMD_PROC   0 // continue processing
#define CALL_CMD_STOP  -1 // cease processing

// callback routine RETURN codes - see ScreenCal and OSK for examples
#define CALL_PROC -1  	// Still processing
#define CALL_ERR  -10 	// error exit return any value <= -10

// Ok exit may be any value >= CALL_EX
#define CALL_EX    0
#define CALL_NOEX  1

// callback processing STATUS codes
#define CALL_ACTIVE 1 	// still processing (call me again)
#define CALL_DONE   2	// completed
#define CALL_START  3	// intialising - not yet called
#define CALL_IDLE   0 	// no call in progress

bool trueVal = true;  // some code needs a pointer to a bool
bool falseVal = false;

#define MODENUM 8 // number of modes
#define MODEITEMS 7 // off and WA_NOR have no parameters
enum DDSwaves {WA_SINE, WA_IMD, WA_WHITE, WA_SQR, WA_TRI, WA_PULSE, WA_STEP, WA_SWEEP, WA_BURST, WA_CONT, WA_NOR};
enum DDSmodes {MO_NOR, MO_SWEEP, MO_BURST};
// AC_COUPLE only for Chan B
enum dds_actions {AC_NORMAL, AC_BURST_START, AC_BURST_NEXT, AC_SWEEP_START, AC_SWEEP_NEXT, AC_IDLE};
#define SW_MODES 3
enum sweepMode_t  {SWM_AM, SWM_FR, SWM_DC};  // Amplitude allowed most often, Freq second, duty least

#define DACVMAX   2.96985  // 3.00 +/- V
#define CALSET 5.000
int DDStab_i[DDS_TAB_LEN]; // use 32 bit int, only fill to 16/24 bit values
float vCal[CHANS] = {DACVMAX, DACVMAX}; // volts @ maxVal
float vCalTmp[CHANS] = {CALSET,CALSET}; // for cal creen

#define WHITE_MUL 1.0  // digital filter overshoots on white noise
#define SMALL_VAL 0.01    // used for testing float non-zero for booleans

// pre-calculated sine parameters
// all in steps and counts
struct sinvals {
  int phAcc;
  int inc;
  int ph;   // (samples) start phase for channel B only in coupled mode
  int scale;  // amplitude / vCal. Shifted left by INTSCALE_BITS
  int dcOff;
  int calcVal;  // ONLY CHAN A used
} sinval[CHANS];

// pre-calculated pulse parameters
// all in steps and counts
struct pulses {
  int maxVal;         // DAC counts
  int minVal;
  int maxSamps;       // sample counts
  int minSamps;
  int riseSamps;      // total
  int riseSampsStep;  // per step
  int64_t riseInc;        // amount to add - can exceed int32 (i.e. can be uint32 max)
  int riseSteps;      // no of samples before inc ( = 1 unless very long ramp or stepped)
  int fallSamps;
  int fallSampsStep;
  int64_t fallInc;
  int fallSteps;
} pulse[CHANS];

// save to EEPROM
enum burstStates {B_ACTIVE, B_IDLE};
struct bursts {
  int cyclesOn;
  int cyclesOff;
  int repeat; 
  bool bAltA; // unused
  bool run;
  burstStates state;
};

struct whitex {
  int range;
  int offset;
  int shift;
};

// sweep displayed settings: Ch A only.
// real world values
struct sweeps 
{ 
  float start; //  real world values
  float end;
  float time; // total duration mS?
  int steps;  // total number of steps / cycle
  sweepMode_t mode;
  bool log;    // linear or log
  bool hold; // NO LONGER USED
  bool run;  // sweep active 
  bool repeat;  // 0 = endless
};

// dynamic sweep variables
// not saved to EEPROM
struct swControl 
{
  int incEvery; // samples 
  int incCount;   
  int stepCount;
  float currAmp;
  float currFreq;
  float currDuty;
  float incVal; // will be multipliers for log sweeps
  //float incFreq;
  //float incDuty;
  bool done;  // sweep cycle complete, may restart
  bool run; // all sweeping is not complete 

} swCont =  {0,0,0,0,0,0,0,true,false}; // startup values

#define UNITLEN 16 // unit string

bool valChanged = false;
#define  VAL_CHANGE_A   1 // maybe can do without stopping. Don't reset phase accumulator.
#define  VAL_CHANGE_B   (1<<1)
#define  FUNC_CHANGE_A  (1<<2) // (CHAN A, B CONT, BURST/SWEEP) stop and restart channel - may take some time
#define  FUNC_CHANGE_B  (1<<3)
#define  START_A        (1<<4) // can do immediately
#define  START_B        (1<<5)
#define  STOP_A         (1<<6)
#define  STOP_B         (1<<7)
#define  START_SWEEP    (1<<8) // should send MODE_CHANGE before. This just starts a pre-set sweep.
#define  STOP_SWEEP     (1<<9)
#define  CHANGE_SWEEP   (1<<10)
#define  START_BURST    (1<<11)
#define  STOP_BURST     (1<<12)
#define  CHANGE_BURST   (1<<13)
#define  CHANGE_CONTROL (1<<14)

#define TRIG_ACTIVE   0 // only used for Trig out 
#define TRIG_IDLE     1
#define TRIG_DISABLE  2 

// on screen keyboard active - disable various functions
bool oskOn = false;
// flags for what needs updating when a settings changes from various sources (TFT, Web, SCPI) 
bool updateVal_DDS = true, updateVal_Scrn = true, updateVal_Vals = true, updateVal_EE = false, updateVal_remote = true, updateVal_web = true; 
//bool updateVal_Scrn = true; // any routine can cause the screen to be redrawn
char messageLine[MULTIERRORLEN]; 
uint32_t messageExpires = 0;

struct settings {
	uint8_t  selChan;			// currently selected channel
  DDSwaves waveForm[CHANS];     // channel DDS function
 	DDSmodes  modeA;	// Normal, Sweep, Burst
  uint8_t touchRot;
} iSet = {CHAN_A, {WA_SINE, WA_SINE}, MO_NOR, TOUCHROT}; // false, false, true,0, false,  true, true,

uint32_t holdScreenUntil = 0;  // don't update screen while this is > 0 (screenError)
bool holdWait = false;  // hold until screen touch (screenError)
#define HOLDSCREEN ((holdScreenUntil > millis()) || holdWait)
bool _scrTouched = false;  // was the screen touched?
bool EEpresent = false; // is EEprom available?
bool needToSaveEE = false;
bool _outOn = false;
bool waveOn = true;//, burst.run = false, swCont.run = false; // changed by setScrMode()
volatile bool cpu0ready = false;
short  highButton = -1;	// currently selected screen button
short  oldHigh = -2;	// previously selected screen button
bool Asel = true, Bsel = false; 
bool startSweep_DDS = false;
uint32_t fifoCmd = 0;
bool ext_trig = false;
bool lastTrigInPol = 0; // (pull_up)
int trigOutActive = TRIG_DISABLE; // is trig-out active?
int trigInActive = false;
bool burstTrigState = false;
bool factoryReset = false;
bool webServerOn = false;
int lastRemoteChan = CHAN_A;
bool _sineCorrect = true;
#define SQRCOMP 1 // precomensation for sqr (50%)
int _sqrPreshoot = SQRCOMP;
//int _sqrComp = SQRCOMP;
bool lastBCOUPLE = false;
char postMsg[POSTLEN];
char lastSCPIerr[MULTIERRORLEN];

#define VSMALL_DIFF 0.00001	// generic value for "significant" difference to floats
#define SMALL_DIFF 0.001  // generic value for "significant" difference to floats
#define SMALLISH_DIFF 0.008 // a bit less than MEDIUM
#define MEDIUM_DIFF 0.01	// 99%
#define MIDDLE_DIFF 0.1
#define LARGE_DIFF  0.5   // used for coarse limiting 
#define BIG_DIFF    1.0   // used for coarse limiting

#define SSIDLEN 34		// > 32 + null
#define NETPASSLEN 64	// > 63 + null
#define NAMELEN 16		// displayed name, used to identify
uint8_t netCIDR = 24;	// default Class C network - to calculate netmask 
bool IamAP = false;	// am I the AP?

#define INST_NAME "DDS"
#define AP_NETNAME "PICOW"
#define AP_NETPW 	"PW123456"

#ifdef REALNET
  #define STA_SSID0 "PalHome"
  #define STA_PASS0 "AF18181F9C4516EE36AF9D79D3"
#else
  #define STA_SSID0 "MySSID"
  #define STA_PASS0 "MyPass"
#endif
/*
struct netParams { // general network spec
  IPAddress local, gateway, subnet;
  bool enabled; // maybe not relevant, other than to enable AP mode?
};
*/
#define HOSTLEN 64
char myHostName[HOSTLEN] = "DISCONN";
char IPstring[16] = "0.0.0.0";
uint8_t myNetCDIR = 24;
int16_t editLAN = 1; // one more than ID
char ssidTemp[SSIDLEN];
char passTemp[NETPASSLEN];
#define SCPI_PORT 5025
int lastAddedSSID = 0;

#define WIFI_STORED 5 // stored WiFi credentials

#define WIFIMAXCONN	127

struct commsID {
  char STA_ssid[WIFI_STORED][SSIDLEN];
	char STA_pass[WIFI_STORED][NETPASSLEN];	// requires DHCP on local WiFi
	char AP_ssid[SSIDLEN];
	char AP_pass[NETPASSLEN];	// provides DHCP 
	bool enabled;	
  uint8_t lastConn;		
	char instName[NAMELEN];
} comms = {{STA_SSID0, "","","",""}, {STA_PASS0,"","","",""}, AP_NETNAME, AP_NETPW, false, 0, INST_NAME};

// updated on connection
IPAddress myIP 			= {192,168,1,200};
IPAddress myBroadcastIP = {192,168,1,255};
IPAddress mySubnetMask  = {255,255,255,0};

IPAddress myAP_IP(192,168,50,1);
IPAddress myAP_GATEWAY(192,168,50,1); // same as IP for mDNS
IPAddress myAP_BroadcastIP = {192,168,50,255};
IPAddress myAP_SubnetMask(255,255,255,0);
IPAddress myAP_DHCP(192,168,50,10);;
#define WIFI_DISCON 0
#define WIFI_CONN   1
#define WIFI_SOFTAP 2
bool    WifiConnected;  // flag set by WiFi (merge with wifiStatus?)
uint8_t wifiStatus = WIFI_DISCON;

struct TS_Point{
  uint16_t x, y, z;
} p;
//char * webString; // big malloc buffer for web requests. Can it be re-used by web code (ONLY) as each request is satisfied before the next is processed????

// LEDW status
struct blinker
{
  int t[2]; // [0] = off time; [1] = on time (100mS units)
  int counter; // normal mmode counter, alternates between t[0] and t[1]
  int sCount; // single mode countdown - set before single
  bool single; // set me for single override
} bl = {{10,0}, 0, false}; // solid off

char xxbuf[10] = "";
union longlong {
    long long ll;
    long l[2];
};
#endif