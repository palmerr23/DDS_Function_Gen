
// library version of RGB565() produces anomalous results
#ifndef  COLOURS_H
#define  COLOURS_H

#define RGB16(R, G, B) (((R)<<11)|((G)<<5)|(B))  // RGB565 in library produces strange results. 
#define CLRBRK (MAXLUM/2)
#define CLRBRKV (MAXLUMV/3)

// Standard color definitions
#define ILI9488_BLACK 0x0000       ///<   0,   0,   0
#define ILI9488_NAVY 0x000F        ///<   0,   0, 123
#define ILI9488_DARKGREEN 0x03E0   ///<   0, 125,   0
#define ILI9488_DARKCYAN 0x03EF    ///<   0, 125, 123
#define ILI9488_MAROON 0x7800      ///< 123,   0,   0
#define ILI9488_PURPLE 0x780F      ///< 123,   0, 123
#define ILI9488_OLIVE 0x7BE0       ///< 123, 125,   0
#define ILI9488_LIGHTGREY 0xC618   ///< 198, 195, 198
#define ILI9488_DARKGREY 0x7BEF    ///< 123, 125, 123
#define ILI9488_BLUE 0x001F        ///<   0,   0, 255
#define ILI9488_GREEN 0x07E0       ///<   0, 255,   0
#define ILI9488_CYAN 0x07FF        ///<   0, 255, 255
#define ILI9488_RED 0xF800         ///< 255,   0,   0
#define ILI9488_MAGENTA 0xF81F     ///< 255,   0, 255
#define ILI9488_YELLOW 0xFFE0      ///< 255, 255,   0
#define ILI9488_WHITE 0xFFFF       ///< 255, 255, 255
#define ILI9488_ORANGE 0xFD20      ///< 255, 165,   0
#define ILI9488_GREENYELLOW 0xAFE5 ///< 173, 255,  41
#define ILI9488_PINK 0xFC18        ///< 255, 130, 198

#define MY_DARKGREEN 0x034E0
#define GREEN_VDARK RGB16(0,16,0)
#define MY_RED 0xF800
#define MY_WHITE 0xFFFF
#define MY_YELLOW 0xFFE0
#define MY_GREEN RGB16(0,36,0)

#define YELLOW_L RGB16(31,63,0)
#define YELLOW_VL RGB16(31,63,10)
#define CYAN_L RGB16(15,45,31)
#define CYAN_VL RGB16(21,63,31)
#define CYAN_M RGB16(8,32,24)
#define GREEN_L RGB16(5,63,5)
#define GREEN_VL RGB16(10,63,105)
#define CYAN_M RGB16(8,32,24)
#define PINK_M RGB16(31,28,31)
#define MY_ORANGE RGB16(31,31,0)	// R + G/2
// special colours
#define VOLT_COL YELLOW_L
#define AMP_COL GREEN_L
#define WATT_COL PINK_M
#define RES_COL CYAN_L
#define TEMP_COL 0x0F9CB
// screen background
#define BGCOL 0x03 // probably 0
#define OSKBG RGB16(10,0,0)
#define BGHIGH RGB16(7,15,5)  
#define MIDBG RGB16(5,11,4) 
// value text
#define TEXTCOL  MY_YELLOW
#define HIGHLIGHTCOL MY_WHITE
//menu button items
#define BTEXTCOL RGB16(20,36,20) //light grey
#define BHIGHCOL 0xffff  //YELLOW_L // MY_YELLOW // ILI9341_BLACK 
#define MSGCOL MY_YELLOW // message line
// menu button backgrounds
#define BBORDHIGH BHIGHCOL
#define BBORD BGCOL
#define BBGHIGH RGB16(4,30,31) // MED blue
#define BBXHIGH RGB16(0,0,18)  // DK blue
#define BBSHIGH RGB16(20,10,0) // MED ORANGE
// mode buttons
#define BONCOL MY_WHITE
#define BONCOL_Y RGB16(31,63,0)
#define BBMHIGH RGB16(0,40,0) // bright green
#define BBMXHIGH RGB16(0,16,0) // med green
// cal buttons
#define BBRHIGH RGB16(31,0,0)
#define BBRXHIGH RGB16(15,0,0)

// error screen backgrounds
#define ERR_BG_MSG GREEN_VDARK
#define ERR_BG_ERR MY_RED

// indicator BG colours
#define ITRIG MY_RED 			// Action triggered
#define IENABLE	MY_DARKGREEN	// Function enabled
#define IND_ONC MY_GREEN
#define IND_ONLC GREEN_L		// FAN medium
#define IND_OFFC MY_RED			// FAN High
#define IND_GO_COL RGB666(0,20,63)


// colours for settinmgs
#define COLVOLT ILI9488_PINK // voltages pink
#define COLFREQ CYAN_L  // frequencies lt blue
#define COLTIMPH GREEN_VL  // times, phase green
#define COLCYCSTEP YELLOW_VL  // cycles, steps yellow
#define COLTAU MY_ORANGE//  sweep type orange
#define NUMCOLRS 5
int scnCols[NUMCOLRS] = {COLVOLT, COLFREQ, COLTIMPH, COLCYCSTEP, COLTAU};
// used by other routines
#define DATUM BL_DATUM
#define SMALLSHIFTY 5
#define SMALLSHIFTX 3
int getColor(int rgb565)
{ int i;
  for(i=0; i < NUMCOLRS; i++)
    if(rgb565 == scnCols[i])
      return i;

  return 0; // default
}
#endif