/* Screen and touch defs 
*/
#ifndef  MYSCREENDEFS_H
#define  MYSCREENDEFS_H

#define GFXFF 1 // font number for GFX
#define TOUCH
#define TOUCH_THRESH 20
#define MIN_TOUCH 1

#include "SPI.h"
// Pin and other Screen parameters set in the TFT_eSPI library's user_setup.h file
#include "TFT_eSPI.h" // #define TOUCH_CS includes touch screen Extensions
#include "myFonts.h" // requires GFXfont definition

// pin and TFT controller setup data in library folder!
TFT_eSPI tft = TFT_eSPI(); // define fonts before this

#define sx(x)	((short)(x))	// no scaling for ILI9488
#define sy(x)	((short)(x))	// no scaling
//  #define sx(x)	((short)(x*2/3))	//  scaling for ILI9341
// #define sy(x)	((short)(x*3/4))	// 

// screen calibration 
// values: minx; max x; miny; max y;  [4]: rot(bit 0), invert_x (bit 1), invert_y (bit 2)
// screen cal sets first 5 elements. screenCalOSK[4] is a bit trial and error!
uint16_t screenCal[6] = {244, 3606, 252, 3645, 4, SCREENROT}; // [0..5] used by TFT_eSPI, SCREENROT is local rotation value
uint16_t screenCalOSK[6] = {244, 3606, 252, 3645, 1, SCREENROT+3}; // OSK setting: rotated +1 : rotInv 7; rot +3 : 1

struct screenCal_OLD {   
  int16_t thmin;  // tft.pixel (0,0)
  int16_t tvmin;
  int16_t thmax;   // tft.pixel (HMAX,VMAX)
  int16_t tvmax;
  uint8_t tsRot;	 // screen rotation - as some touch screens are rotated 180%
  uint8_t scrRot;  // screen rotation - as some touch screens are rotated 180%
};
screenCal_OLD sc = {400, 300, 3900, 3700, TOUCHROT, SCREENROT}; // calibration deprecated (use screenCal[]), scrRot still used. 

//uint16_t touchCal[] = {300, 3600, 300, 3600, 0x04}; // xo, x1, y0, y1, 

// touch units to screen pixel conversions
int16_t pxh (int16_t px){ 
  //int x = ((px-sc.thmin)*HMAX)/(sc.thmax-sc.thmin);
  //if (x < 0) return 0;
  //if (x > HMAX) return HMAX;
  return px;
}
int16_t pxv (int16_t py){ 
  //  int y = ((py-sc.thmin)*VMAX)/(sc.tvmax-sc.tvmin);
  //if (y < 0) return 0;
  //if (y > VMAX) return VMAX;
  return py;  
}


#ifdef ILI9488
  #define BUTSIZ 50 
#else
  #define BUTSIZ 40    
#endif

#define BVSPACE 55
#define BVSPACE2 45

#define LEGX 1
#define LSETX 35
#define LSETX2 22
#define RSETX 135
#define LEG2X 5
#define DWIDTH2 12
#define CIRCX 10
#define VALX 100
#define VAL2X 60
#define CALX 80
#define CAL2X 160
#define CALREADX 240
#define READX 185
#define TOPLINE 95
#define BVTOP (TOPLINE - BVSPACE)
//#define TOPLEG 17 // big enough for a &FONT1 character
#define TOPLEGS 18 // big enough for a &FONT1 character
#define MSGLINE (VMAX - BVSPACE)
#define MAXMSG 26
#define MSG_HT 20

#define IPSPACE 55
#define IPDOT 40
#define ROUNDERR 0.01    // float rounding may stop hi/low set values being attained.

#define LEGLEN 16

// turn these into a struct, include format string?
struct  valFmt {
 volatile void * vp;    // pointer to value - type is determined by fmt, assume all readings will be volatile
 float minVal, maxVal; // limits for settings; hi-low error points for readings (float is OK for most things)
 uint8_t menu;         // which menu are these in?
 uint8_t font;  // font (??-1 for GLCD font (wider character set)
 uint8_t textSize;     // textSize multiplier -- NOT used
 short xpos, ypos;     // screen location
 short legx;            // x pos of legend. font and y same as displayed value
 char  legend[LEGLEN];  // same font size as value
 char  fmt;             // format and value type - see printfloatX() - 'T' gets special treatment when touched
 char  units[UNITLEN]; // units to print on screen
 uint32_t clr;        // colour of text
 uint8_t pre, post;    // decimals before and after the point (post is ignored for fmt != 'F')
 bool *displayIf;		 // only display if TRUE
 bool valIsFloat;
 };
 
// SETTINGS VARIABLES 
// current set/read vals. SET values will be overwritten on start-up from saved EEPROM values
// only float and int8_t allowed
float dummy = 1;
#define vSetting 	dummy
#define iSetting 	dummy
#define rSetting 	dummy
#define iSetting 	dummy
#define ltSetting	dummy
#define mSetting  iSet.waveForm[CHAN_A] // start with CHAN_A

#define iHost 		comms.instName
#define tsRota		sc.tsRot
float calOffLo_T, calOffHi_T;
// READINGS VARIABLES 
#define vReading 	dummy
#define iReading 	dummy
#define tReading 	dummy
float pReading = 0.0;
float rReading = 0.0;

#define wAutoConn 	comms.enabled
#define IPreading 	IPstring
#define HostName 	myHostName // FQDN (mDNS)
#define dummyRead	xDumVal
#define wReading NULL
int dummySteps;
#define stepSMPS dummySteps

char dummyS[64] = "ABC";
#define LSET 2 // index for LIMIT
#define TSET 3 // TRACK
char dum = '.';
#define TTGERR 0
#define MAIN_MENU 0
#define SWEEP_MENU 1
#define FUNC_MENU 2
#define BURST_MENU 3
#define CONT_MENU 5

#define NUMSETTINGS 18
valFmt setVals[NUMSETTINGS] = {
#define SV_V 0  // ***** DO NOT MOVE *****
   // 7 parameter settings, some values updated by setScrMode() / Sweep / Burst menus
   {&dummy, 0,100, 0,2, 1, sx(RSETX),sy(TOPLINE+0*BVSPACE), LEGX, "YX", 'F', "Z", VOLT_COL, 6,2, NULL, true},  
   {&dummy, 0,100, 0,2, 1, sx(RSETX),sy(TOPLINE+1*BVSPACE), LEGX, "YY", 'F', "X", AMP_COL, 6,2, NULL, true},     
   {&dummy, 0,100, 0,2, 1, sx(RSETX),sy(TOPLINE+2*BVSPACE), LEGX, "YZ", 'F', "C", WATT_COL,6,2, NULL, true},    
   {&dummy, 0,100, 0,2, 1, sx(RSETX),sy(TOPLINE+3*BVSPACE), LEGX, "YA", 'F', "V", RES_COL, 6,2, NULL, true},    
   {&dummy, 0,100, 0,2, 1, sx(RSETX),sy(TOPLINE+4*BVSPACE), LEGX, "YB", 'F', "B", RES_COL, 6,2, NULL, true},    
   {&dummy, 0,100, 0,2, 1, sx(RSETX),sy(TOPLINE+5*BVSPACE), LEGX, "YC", 'F', "N", RES_COL, 6,2, NULL, true},    
   {&dummy, 0,100, 0,2, 1, sx(RSETX),sy(TOPLINE+6*BVSPACE), LEGX, "YD", 'F', "M", RES_COL, 6,2, NULL, true},  
#define FUNCTION_SET 7  
  {&mSetting, 0,100,0,1, 1,  sx(0),sy(TOPLINE+3*BVSPACE), LEGX,"YE",'U', "P", RES_COL, 3,2, &falseVal, true},    // function (hidden)
#define AB_SET 8 
  {&dummy, 0,100,0,1, 1,  sx(0),sy(TOPLINE+4*BVSPACE), 20,"YF",'F', "AB", RES_COL, 3,2, &falseVal, true},    // A / B channel (hidden)
#define SET_COM 9
   //COMMS MENU - 1
   {&editLAN, 		   1, WIFI_STORED,   8, 1, 1,  sx(VALX),sy(TOPLINE+0*BVSPACE),5, "LAN ID",'I', "", CYAN_L,1,0, NULL, false},     // Wifi Entry ID 
   {&dummyS, 0,255, 8, 1, 1,  sx(VALX),sy(TOPLINE+1*BVSPACE), 5,"SSID",'A', "", CYAN_L,32,0, NULL, false},
   {&dummyS, 0,255, 8, 1, 1,  sx(VALX),sy(TOPLINE+2*BVSPACE+10),5,"PASS", 'A', "", CYAN_L,63,0, NULL, false},   // long (32 chars) - offset Y, so under button
   {&wAutoConn, 	   0,1,	  8, 1, 1,  sx(READX+40),sy(TOPLINE+3*BVSPACE), 5,"",'T', "", CYAN_L,2,0, NULL, false},  // auto connect
   {&comms.instName, 0,1,   8, 1, 1,  sx(VALX),sy(TOPLINE+4*BVSPACE),5, "Host",'A', "", CYAN_L,24,0, NULL, false},     // hostname

#define SET_CAL (SET_COM + 5)
   //CAL MENU - 3
   {&vCalTmp[0], 4, 6,9,2, 1, sx(READX-30),sy(TOPLINE+3*BVSPACE), 20,"Cal A",'F', "", VOLT_COL,2,3, NULL, true},     // Calibration (variable => 0.0 on entering Cal menu)
   {&vCalTmp[1], 4, 6,9,2, 1, sx(READX-30),sy(TOPLINE+4*BVSPACE),20,"CAL B",'F', "", VOLT_COL,2,3, NULL, true},   
   //{&dummy, -30, 30,9,2, 1, sx(READX),sy(TOPLINE+2*BVSPACE-5), 20,"YX",'F', "", TEMP_COL,2,1, NULL},   
   //{&dummy, -30, 30,9,2, 1, sx(READX),sy(TOPLINE+3*BVSPACE-5), 20,"YX",'F', "", TEMP_COL,2,1, NULL},   

 #define SET_ID (SET_CAL + 2)
   // ID MENU  - NOT USED
   {&dummyS, 0,255, 99,1, 1, sx(100),sy(TOPLINE), 20,"YX",'A', "kN",CYAN_L,12,0, NULL, false}, // test string edit
   {NULL, 0,0, 0,0, 1,0,0, 20,"YX",'X', "",CYAN_L,0,0, NULL, false} // EOM: .vp == NULL
 };
 
#define NUMREADS 2
valFmt readVals[NUMREADS]= { 
#define RV_I 1
   //{&dummy, 0,50.0,99,1, 1, sx(LSETX2), sy(TOPLINE+1*BVSPACE),20,"YX", 'F', "A", AMP_COL,3,2,  &falseVal},  // output amps
     // COMMS Menu - 8
   {&IPstring,   0, 0, 8,1, 1,   sx(100), sy(TOPLINE+5*BVSPACE),LEGX, "IP",'A', "C", CYAN_M,3,0, NULL, false},    // IP ADDRESS
   {&myHostName, 0, 0, 8,1, 1,   sx(100), sy(TOPLINE+6*BVSPACE),LEGX, "HOST",'A', "C", CYAN_M,3,0, NULL, false},    // fully qualified hostname (mDNS)
   // STEP MENU - 2 (NONE)
   // rotations are displayed as indicators see drawIndicators()
   //{&tsRota,  -99,99,3,  2, 1,  HMAX/2 - 18, VMAX/2 + sy(50), 20,"YX",'I', "", YELLOW_L,1,0, NULL},  // screen rotation
   //{NULL, 0,0,0,0, 1,0,0,  20,"YX",'X', "",0,0, NULL, false} // EOM: .vp == NULL
 };   

 // TOUCH SCREEN BUTTONS
//  sizing

#define BUTEDGETHICK 2
#define BUTROUND 6
#define TEXTDOWN 4
// location
#define BUTX 270  // 275
#define BUTBOTV (240 - BUTSIZ)
#define BUTBOTV2 (240 - BUTSIZ/2)
#define FUNCBUTX 60

#define BVSPACEX BVSPACE
#define BHSPACE (3 + HMAX/5)
#define TOPLINEX (TOPLINE - 5)
#define BUTMIDX ((HMAX - BUTSIZ)/2)    // this is scaled
#define BTOP 0
#define SBTOP (BTOP + BUTSIZ/2) // small buttons - align baseline

// LEGENDS
#define LEGMAX 19
struct legend {
   uint8_t font;  // font
   uint8_t textSize;     // textSize
   short xpos, ypos;     // screen location
   int8_t menu;          // which menu are these in?
   char text[LEGMAX];    // text to write
   uint32_t clr;         // colour of text
   bool *displayIf;		 // only display if TRUE
};
#define NUMLEGS 7	// also some static screen items in redrawScreen()
legend leg[NUMLEGS] = {
//{1,  1,12, 150, 0, "TEST", HIGHLIGHTCOL, NULL}, //test
   // COMMS MENU - 1
   {1,  1,sx(2), sy(TOPLEGS), 8, "COMMS", HIGHLIGHTCOL, NULL},
   {1,  1,sx(2), sy(TOPLINE+3*BVSPACE), 8, "WiFi Enabled", CYAN_L, NULL},
   {1,  1,sx(2), sy(TOPLINE+5*BVSPACE), 8, "IP", CYAN_M, NULL},
   {1,  1,sx(2), sy(TOPLINE+6*BVSPACE), 8, "HOST", CYAN_M, NULL},
   // TRACK MENU - 2
  // {1,  1,sx(2), sy(TOPLEGS), 99, "SWEEP", HIGHLIGHTCOL, NULL},
   // CAL MENU -3
   {1,  1,sx(2), sy(TOPLEGS), 9, "SETTINGS", HIGHLIGHTCOL, NULL},
   //{1,  1,sx(READX), sy(TOPLEGS+40), 9, "OFFSET", HIGHLIGHTCOL, NULL},
   //{1,  1,sx(LEG2X+34), sy(TOPLEGS+40), 9, "READING", HIGHLIGHTCOL, NULL},
   //settings menu 
   {1,  1,sx(5), sy(TOPLEGS), 4, "SETTINGS", HIGHLIGHTCOL, NULL},
  // {1,  1,sx(BUTX-80), sy(TOPLEGS+8), 4, "VOLT", HIGHLIGHTCOL, NULL},
  // {1,  1,sx(BUTX-80), sy(TOPLEGS+24), 4, "MEAS", HIGHLIGHTCOL, NULL},
   {0,  0,0, 0, -1, "0", 0, NULL} // EOM: .menu == -1
};

uint8_t callStatus = CALL_IDLE;
boolean wasTouched = false;
// also used by switches
bool swPressed = false, digSwPressed = false; 
short butDigit = 0; // start at 10^0
short butDir = 0;
uint8_t currentMenu = 0; // which menu are we in?

// button call back declarations
int setSine(int);
int setIMD(int);
int setWhite(int x);
int setSqr(int x);
int setTri(int x);
int setStep(int x);
int setPulse(int x);
int setIdle(int x);

int setSweepMenu(int x);
int setBurstMenu(int);
int setFuncMenu(int);
int setContMenu(int);
int setSetMenu(int);

int setComMenu(int);
int toggleSweep(int);
int toggleBurst(int);
int setNor(int);
//int cycleLAN(int);

int selA(int);
int selB(int);
int setNor(int);
int setSweep(int);
int setBurst(int);
//int sweepEx(int);
//int burstEx(int);
int subEx(int);
int setEx(int);

int setScreenCal(int);
int exitCal(int);
int tsRotate(int);
int scrRotate(int);
int factReset(int);

int setZeroCal(int);
int calSave(int);
//int calEntry(int);
int toggleWconn(int);
//int vAuto(int);

#define BUTTEXT 12 // 3-5 chars for a standard width button
struct button {
  int (* callback)(int cmd); // see above
  int8_t  menu;
  uint8_t nextMenu;   // same as menu, unless changing screens 
  uint8_t sv;     // index to the SetVal item to be edited
  uint16_t siz;     // val 0..7 - bit 0..3 height (0 = half height); bit 4..7 width; bit 8 = display
  short   xpos, ypos;
  bool    border;
  char text[BUTTEXT];
  uint32_t selColour;	// some buttons are also indicators - so provide ability to change bg colours
  uint32_t unselColour;
  bool *onIf;			// also display as "selected" if this is set
  bool *displayIf;		// only display button if TRUE, invisible if false (may still be enabled)
  bool *enableIf;      // ignore button if FALSE
};

#define CHOFFY 15 // offsets for invisible buttons
#define CHOFFX (10)
// some setVal indices - used in tMode
#define LBUT 2
#define TBUT 3
#define WBUT 6
#define GSVBUT SET_STEP 
#define GSABUT (SET_STEP +1)
#define NOSETVAL 255


#define NUMBUTS 49
button but[NUMBUTS] = {
  // MAIN MENU - 0
  #define MAIN_BUTS 0
  {NULL, 0, 0, 0, 0x161, sx(10), sy(BVTOP+0*BVSPACE), false,"A", BBGHIGH, BBXHIGH, NULL, &falseVal, NULL}, // invisible buttons: Setval[0..6] ***** DO NOT MOVE *****
  {NULL, 0, 0, 1, 0x161, sx(10), sy(BVTOP+1*BVSPACE), false,"B", BBGHIGH, BBXHIGH, NULL, &falseVal, NULL},
  {NULL, 0, 0, 2, 0x161, sx(10), sy(BVTOP+2*BVSPACE), false,"C", BBGHIGH, BBXHIGH, NULL, &falseVal, NULL},
  {NULL, 0, 0, 3, 0x161, sx(10), sy(BVTOP+3*BVSPACE), false,"D", BBGHIGH, BBXHIGH, NULL, &falseVal, NULL},
  {NULL, 0, 0, 4, 0x161, sx(10), sy(BVTOP+4*BVSPACE), false,"E", BBGHIGH, BBXHIGH, NULL, &falseVal, NULL},
  {NULL, 0, 0, 5, 0x161, sx(10), sy(BVTOP+5*BVSPACE), false,"F", BBGHIGH, BBXHIGH, NULL, &falseVal, NULL},
  {NULL, 0, 0, 6, 0x161, sx(10), sy(BVTOP+6*BVSPACE), false,"G", BBGHIGH, BBXHIGH, NULL, &falseVal, NULL},
  // CHANNEL SETTING 
#define AB_BUTS 7
  {&selA, 0, 0, AB_SET, 0x111, sx(0), sy(BTOP+0*BVSPACEX), false,"A", BBGHIGH, BBXHIGH, &Asel, NULL, NULL},
  {&selB, 0, 0, AB_SET, 0x111, sx(BUTX), sy(BTOP+0*BVSPACEX), false,"B", BBGHIGH, BBXHIGH, &Bsel, NULL, NULL},
  
  // FUNCTION Setting 
#define FUNC_BUTTON 9
  {&setFuncMenu,0, 2, FUNCTION_SET, 0x141, sx(FUNCBUTX), sy(BTOP+0*BVSPACEX), false,"FUNC", BBMHIGH, BBMXHIGH, NULL, NULL, NULL}, // go to Function menu (text overwritten)

  {&setNor, 0, 0, NOSETVAL, 0x111, sx(0),VMAX - BUTSIZ, false,"Wav", BBMHIGH, BBMXHIGH, &waveOn, NULL, NULL}, //  cancels Sweep, etc
  {&setSweepMenu, 0, 1, NOSETVAL, 0x111, sx(BHSPACE),VMAX - BUTSIZ, false,"Swp", BBMHIGH, BBMXHIGH, &swCont.run, NULL, NULL},     // bright Green when active
  {&setBurstMenu, 0, 3, NOSETVAL, 0x111, sx(2*BHSPACE), VMAX - BUTSIZ, false,"Bur", BBMHIGH, BBMXHIGH, &burst.run, NULL, NULL},	//
  {&setContMenu, 0, 5, NOSETVAL, 0x111, sx(3*BHSPACE), VMAX - BUTSIZ, false,"Con", BBMHIGH, BBMXHIGH, NULL, NULL, NULL},	// Control menu
  {&setSetMenu, 0, 9, NOSETVAL, 0x111, sx(4*BHSPACE), VMAX - BUTSIZ, false,"Set", BBMHIGH, BBMXHIGH, NULL, NULL, NULL},	// Set Menu
  
  // Sweep Menu 1
  {&toggleSweep, 1, 1, NOSETVAL, 0x121, (HMAX)/3, sy(BTOP), false,"SWEEP", BBMHIGH, BBMXHIGH, &swCont.run, NULL, NULL}, // SWEEP
  {&subEx,1, 0, NOSETVAL, 0x111, sx(BUTX), VMAX - BUTSIZ, false,"X", BBGHIGH, BBXHIGH, NULL, NULL, NULL}, // Sweep  exit

  // Function MENU - 2
  {&selA,    2, 2, AB_SET, 0x111, sx(0), sy(BTOP+0*BVSPACEX), false,"A", BBGHIGH, BBXHIGH, &Asel, NULL, NULL}, // duplicated channel buttons
  {&selB,    2, 2, AB_SET, 0x111, sx(BUTX), sy(BTOP+0*BVSPACEX), false,"B", BBGHIGH, BBXHIGH, &Bsel, NULL, NULL},
  {&setSine, 2, 0, NOSETVAL, 0x141, sx(FUNCBUTX), sy(BTOP+0*BVSPACE), false,"Sine", BBGHIGH, BBXHIGH, NULL, NULL, NULL},  //exit immediately
  {&setIMD,  2, 0, NOSETVAL, 0x141, sx(FUNCBUTX), sy(BTOP+1*BVSPACE), false,"IMD", BBGHIGH, BBXHIGH, NULL, NULL, &Asel},  // not CHAN_B
  {&setWhite,2, 0, NOSETVAL, 0x141, sx(FUNCBUTX), sy(BTOP+2*BVSPACE), false,"White", BBGHIGH, BBXHIGH, NULL, NULL, NULL},  
  {&setSqr,  2, 0, NOSETVAL, 0x141, sx(FUNCBUTX), sy(BTOP+3*BVSPACE), false,"Square", BBGHIGH, BBXHIGH, NULL, NULL, NULL}, 
  {&setTri,  2, 0, NOSETVAL, 0x141, sx(FUNCBUTX), sy(BTOP+4*BVSPACE), false,"Triangle", BBGHIGH, BBXHIGH, NULL, NULL, NULL},  
  {&setStep, 2, 0, NOSETVAL, 0x141, sx(FUNCBUTX), sy(BTOP+5*BVSPACE), false,"Step", BBGHIGH, BBXHIGH, NULL, NULL, NULL},  
  {&setPulse,2, 0, NOSETVAL, 0x141, sx(FUNCBUTX), sy(BTOP+6*BVSPACE), false,"Pulse", BBGHIGH, BBXHIGH, NULL, NULL, NULL}, 
  {&subEx,   2, 0, NOSETVAL, 0x111, sx(BUTX), VMAX - BUTSIZ, false,"X", BBGHIGH, BBXHIGH, NULL, NULL, NULL}, // Function  exit
  
  // Burst menu - 3
  {&toggleBurst, 3, 3, NOSETVAL, 0x121, (HMAX)/3, sy(BTOP), false,"BURST", BBMHIGH, BBMXHIGH, &burst.run, NULL, NULL}, // BURST
  {&subEx,3, 0, NOSETVAL, 0x111, sx(BUTX), VMAX - BUTSIZ, false,"X", BBGHIGH, BBXHIGH, NULL, NULL, NULL}, // Burst  exit
  // Settings menu - 4 (See CAL menu)
  //{&calEntry, 4, 9, NOSETVAL, 0x111, (HMAX)/3, sy(BTOP+2*BVSPACEX), false,"CAL", BBSHIGH, BBSHIGH, NULL, NULL, NULL}, //CAL
  // {NULL,      4, 4, NOSETVAL, 0x111,  (HMAX)/3, sy(BTOP+5*BVSPACEX), false,"AUTO", BBGHIGH, BBXHIGH, NULL, NULL, NULL}, //v Measurement
  //{NULL,      4, 4, NOSETVAL, 0x111,  sx(BUTX-80), sy(BTOP+2*BVSPACEX), false,"MAIN", BBGHIGH, BBXHIGH, NULL, NULL, NULL}, 
  //{NULL,      4, 4, NOSETVAL, 0x111,  sx(BUTX-80), sy(BTOP+3.5*BVSPACEX), false,"KEL", BBGHIGH, BBXHIGH, NULL, NULL, NULL}, 
  {&setEx,4, 0, NOSETVAL, 0x111, sx(BUTX), VMAX - BUTSIZ, false,"X", BBGHIGH, BBXHIGH, NULL, NULL, NULL}, // SET exit
  
  // CONTROL menu - 5
  {NULL,      5, 0, NOSETVAL, 0x121, (HMAX)/3, sy(BTOP), false,"CONTROL", BBSHIGH, BBSHIGH, NULL, NULL, NULL}, // CONTROL
  {&subEx,      5, 0, NOSETVAL, 0x111, sx(BUTX), VMAX - BUTSIZ, false,"X", BBGHIGH, BBXHIGH, NULL, NULL, NULL}, // Control exit

  // CAL MENU - 9
  {&factReset,  9, 0, NOSETVAL, 0x111, sx(10), sy(BVTOP+0*BVSPACE), false,  "RST", BBSHIGH, BBSHIGH, NULL, NULL, NULL}, // Reset  
  {NULL,9, 9, SET_CAL,   0x161, 		sx(10), sy(BVTOP+3*BVSPACE), 	false,	"VA" , BBGHIGH, BBXHIGH, NULL, &falseVal, NULL}, 
  {NULL,9, 9, SET_CAL+1, 0x161, 		sx(10), sy(BVTOP+4*BVSPACE), 	false,	"VB" , BBGHIGH, BBXHIGH, NULL, &falseVal, NULL}, 
  {&setScreenCal,9, 9, NOSETVAL, 0x111, sx(10), VMAX - BUTSIZ, 		false,	"Tch", BBSHIGH, BBSHIGH, NULL, NULL, NULL}, 
  {&setComMenu,  9, 8, NOSETVAL, 0x111, BUTSIZ+40, VMAX - BUTSIZ, false,  "Com", BBSHIGH, BBSHIGH, NULL, NULL, NULL}, // COM
  {&calSave,9, 0, NOSETVAL, 0x111, 	 HMAX - 3* BUTSIZ, VMAX - BUTSIZ, false,"Save" , BBGHIGH, BBXHIGH, NULL, NULL, NULL}, // save and exit - Cancel is below
  {&exitCal,9, 0, NOSETVAL, 0x111,  sx(BUTX), VMAX - BUTSIZ, false,"X", BBGHIGH, BBXHIGH, NULL, NULL, NULL},  // CAL (cancel) exit
  //{&tsRotate,9, 9, NOSETVAL, 0x111, (HMAX - BUTSIZ)/2-12, (VMAX - BUTSIZ)/2+10, false,"TROT" , BBGHIGH, BBXHIGH, NULL, NULL, NULL}, // in middle of screen - as rotation may be reversed
  //{&scrRotate,9, 9, NOSETVAL, 0x111, HMAX - BUTSIZ/2, VMAX - BUTSIZ/2, false,"SROT" , BBGHIGH, BBXHIGH, NULL, NULL, NULL}, // below TROT

  // COM MENU - 8
  #define LANBUTTON 38
  {NULL,   8, 8, SET_COM, 			0x111, sx(BUTX), sy(BTOP + 1*BVSPACE), false,"LAN", BBGHIGH, BBXHIGH, NULL, NULL, NULL}, // LAN ID#
  {NULL,        8, 8, SET_COM + 1, 	0x111, sx(BUTX), sy(BTOP + 2*BVSPACE), false,"SS", BBGHIGH, BBXHIGH, NULL, NULL, NULL}, // SSID
  {NULL,        8, 8, SET_COM + 2, 	0x111, sx(BUTX), sy(BTOP + 3*BVSPACE), false,"PA", BBGHIGH, BBXHIGH, NULL, NULL, NULL}, //WiFi Pass
  {&toggleWconn,8, 8, SET_COM + 3,  0x111, sx(BUTX), sy(BTOP + 4*BVSPACE), false,"AC", BBGHIGH, BBXHIGH, NULL, NULL, NULL}, // Toggle WiFi enabled
  {NULL,        8, 8, SET_COM + 4, 	0x111, sx(BUTX), sy(BTOP + 5*BVSPACE), false,"HO", BBGHIGH, BBXHIGH, NULL, NULL, NULL}, // Hostname
  {&subEx,      8, 0, NOSETVAL,     0x111, sx(BUTX), VMAX - BUTSIZ, false,"X", BBGHIGH, BBXHIGH, NULL, NULL, NULL}, // COMMS exit

  {NULL,-1, 0, NOSETVAL, 0, 0, 0, false,"", BBGHIGH, BBXHIGH, NULL, NULL, NULL} // EOM
};

/*
Colour tables for Toggle buttons and Indicators
Modes 
	Off [0] 
	On [1]: Inactive, e.g. not limiting or tracking = 100%
	On [2]: Active, e.g. limiting or tracking < 100%
for indicators, Off is BGCOLOR, rather than tColours[0]
*/
#define BBOFF 0 // track and limit (disabled)
#define BBINACT 1 // function enabled, but not active
#define BBACT 2 // function enabled, and active
#define WIFIACT 1 // green (enabled + connected)
#define WIFIOFF 0 // blue  (enabled off = idle)
#define WIFIINACT 2 // red (enabled + not connected = error)
uint32_t tColours[3] = {BBXHIGH, MY_DARKGREEN, MY_RED};

#endif
