
// library version of RGB565() produces anomalous results
#ifndef  MYFONTS_H
#define  MYFONTS_H


#ifdef ILI9488
  #define ILI9488_FONTS
	//#include "myFonts/DejaVu_Sans_Mono_Bold_16.h"
	#include "myFonts/DejaVu_Sans_Mono_Bold_18.h"
	#include "myFonts/DejaVu_Sans_Mono_Bold_22.h"
	//#include "myFonts/DejaVu_Sans_Mono_Bold_24.h"
	//#include "myFonts/DejaVu_Sans_Mono_Bold_26.h"
  #include "myFonts/DejaVu_Sans_Mono_Bold_28.h"
	//#include "myFonts/DejaVu_Sans_Mono_Bold_32.h"
	#include "myFonts/DejaVu_Sans_Mono_Bold_42.h"
	
	#define FONT0 DejaVu_Sans_Mono_Bold_18  // smallest
 // #define FONT0X &DejaVu_Sans_Mono_Bold_18  // smallest
	#define FONT1 DejaVu_Sans_Mono_Bold_22  // small
  //#define FONT1X &DejaVu_Sans_Mono_Bold_22 
	//#define FONT1 DejaVu_Sans_Mono_Bold_22  // larger
	//#define FONT1 DejaVu_Sans_Mono_Bold_24  // larger
	//#define FONT2 DejaVu_Sans_Mono_Bold_32  // larger
 // #define FONT2X &DejaVu_Sans_Mono_Bold_32  // larger
	#define FONT2 DejaVu_Sans_Mono_Bold_28  // larger
	#define FONT3 DejaVu_Sans_Mono_Bold_42    // biggest
  //#define FONT3X &DejaVu_Sans_Mono_Bold_42 
#else // ILI9341
  #define ILI9341_FONTS
	#include "myFonts/DejaVu_Sans_Mono_Bold_16.h"
	#include "myFonts/DejaVu_Sans_Mono_Bold_18.h"
	//#include "myFonts/DejaVu_Sans_Mono_Bold_22.h"
	#include "myFonts/DejaVu_Sans_Mono_Bold_24.h"
	//#include "myFonts/DejaVu_Sans_Mono_Bold_26.h"
	#include "myFonts/DejaVu_Sans_Mono_Bold_28.h"
	//	#include "myFonts/DejaVu_Sans_Mono_Bold_32.h"
	//#include "myFonts/DejaVu_Sans_Mono_Bold_38.h"
	
	#define FONT0 DejaVu_Sans_Mono_Bold_16  // smallest
  //#define FONT0X &DejaVu_Sans_Mono_Bold_16 
	#define FONT1 DejaVu_Sans_Mono_Bold_18  // small
	//#define FONT1 DejaVu_Sans_Mono_Bold_22  // larger
	#define FONT2 DejaVu_Sans_Mono_Bold_24  // larger
	//#define FONT2 DejaVu_Sans_Mono_Bold_26  // larger
	#define FONT3 DejaVu_Sans_Mono_Bold_28  // larger
	//#define FONT3 DejaVu_Sans_Mono_Bold_32  // biggest
	//#define FONT3 DejaVu_Sans_Mono_Bold_38        // biggest
#endif

#define NUMFONTS 4
const GFXfont *myGFXfonts[NUMFONTS] = {&FONT0, &FONT1, &FONT2, &FONT3};

#endif