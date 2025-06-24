/* Screen Drawing routines 
*/
#ifndef MYSCREENDRAW_H
#define MYSCREENDRAW_H

#include "screenDefs.h"
// most button callbacks here
#include "screenCal.h" 
#include "OSK.h"
bool processTouchSwEnc(bool);
void touch_calibrate();
void printX(valFmt * xp,  short highlight, uint16_t txtcol, uint16_t highcol, uint16_t bgcol, bool editing, bool readFloat);
//void printfloatX(void * vp, char* v2, void * ov, uint8_t fNum,  uint8_t textSize, short pre, short post, short highlight, uint16_t x, short y, char fmt, uint16_t txtcol, uint16_t highcol, uint16_t bgcol);
int charpos = 5;
void tftDebugChar(char c) // debug
{
  char sb[2] = " ";
  sb[0] = c;
  tft.setFreeFont(&DejaVu_Sans_Mono_Bold_22);
  tft.setCursor(charpos, 25);
  tft.setTextColor(ILI9488_RED);
  tft.print(sb);
  charpos += 12;
}
#define TEST_TEXT "AaBbCc: ßäöü ñâàå"   // Text that will be printed on screen in the font
void screenBegin(void)
{
	tft.begin();
	tft.setRotation(screenCal[5]); // defaults - reset after reading profile from EEPROM
  tft.setTouch(screenCal);
  tft.setTextDatum(DATUM);
  #ifdef ILI9488_FONTS
   //Serial.println("GFX fonts included");
  #endif
  tft.fillScreen(BGCOL); // BGCOL 
 // touch_calibrate();
 
  (iSet.selChan) ? selB(0) : selA(0); // display buttons
}

// calculate the correct mode for toggles
short tMode(short sv)
{

	// WiFi
	if (sv == WBUT)
	{
	  if (comms.enabled)
	  {
      if(WifiConnected)
      {
        //Serial.println("WBUT Act");
        return WIFIACT;
      }
      else
      {
        //Serial.println("WBUT InAct");
        return WIFIINACT;
      }
	  }
	  else // disabled	  
	  {
		   //Serial.println("WBUT Off");
		   return WIFIOFF;
	  }
	}	
	return -1; // sv not found
}

// colour for button and indicator background - toggles
uint16_t tColour(short sv, char mode)
{
	// indicators as buttons except "off" is screen background colour
	short tm;
	tm = tMode(sv);
	if (tm <= 0)
	{ 
	  if (mode == 'I' || mode == 'U')
	    return BGCOL;
	  else
		  return BBXHIGH;
	}
	return tColours[tm];
}	
#define ABUT (BUTSIZ/3)
#define ABUTR (ABUT/2 -1)
#define ABUTX (BUTSIZ-10)
#define ABUTOFF 2
void drawIndicators(short aMenu)
{
	int8_t tm; 
	uint16_t xcol, xpos, ypos, butht;
	
	if (oskOn || calOn)
		return;
    tft.setTextFont(0);
    tft.setTextSize(2);
	//tft.setFreeFont(FONT1X);
	// every screen indicators
	
	// wireless connected & mode
	//tft.fillRoundRect(VALX+40, 0, ABUT, ABUT,  ABUTR, (WifiConnected)?IENABLE:ITRIG); 
	tft.setCursor(sx(READX + 70), sy(TOPLEGS));
	tft.setTextColor((WifiConnected)?IND_ONC:IND_OFFC);
  if(wifiStatus == WIFI_SOFTAP) 
	  tft.print("AP"); // only of connected as Soft AP
  else
   tft.print("W");
	
	// save to EE pending
	if (needToSaveEE) // save to EE indicator
		tft.setTextColor(MY_YELLOW);
	else
		tft.setTextColor(BGCOL);		
	tft.setCursor(sx(READX + 90), sy(15));
	tft.print("E");

  tft.setTextColor(MY_YELLOW);	// safety

  // voltage measurement mode
  /*
  tft.setCursor(sx(READX + 110), sy(TOPLEGS));
  tft.setTextColor(MY_GREEN);
  tft.print(iSet.vMode);
  tft.print(_cur_vMode);
*/
	tft.setTextColor(BHIGHCOL);
	int ystart;
	int xstart;
  switch (aMenu)
  {
    case 0:		// main menu

		// limiting indicator
		//tm = tMode(LBUT);
		ystart = 1.5 * BVSPACE + 1;
		xstart = ABUTX;
		//xcol = tColour(LBUT, 'I');

		// temp indicator
	

		break;
  
    case 1: // 
		break;
  
    case 2: // 
		break;
	
	case 9: //cal
    tft.setFreeFont(myGFXfonts[1]);
    tft.setTextSize(1);
    tft.setCursor(sx(120), sy(140));
    tft.print(sc.tsRot);
    tft.setCursor(sx(120), sy(180));    
    tft.print(sc.scrRot);
		break;
  }
}

// draw legends on the menu screen
void drawLegends(short cMenu){
  short i = 0;
  uint16_t tempClr;
  if (oskOn || calOn)
		return;
  //for (short i =0; i< NUMLEGS; i++)
  while (leg[i].menu >= 0) // terminating item has -1 flag
  {
    if (leg[i].menu == cMenu)	  
	{
  	   // check for conditional display - print in Background colour to erase
	   if(leg[i].displayIf == NULL || *(leg[i].displayIf)) 
		  tempClr = leg[i].clr;
	   else
		  tempClr = BGCOL;

	   tft.setFreeFont(&FONT1);//tft.setFreeFont(myGFXfonts[leg[i].font]);// set font and text size first, or cursor baseline will be wrong 
	   tft.setTextSize(leg[i].textSize);	
	   tft.setCursor(leg[i].xpos, leg[i].ypos);
	   tft.setTextColor(tempClr);
	   tft.print(leg[i].text);
	}    
    i++;
  }
}

#define DISP_BUT 0x0100
void drawButtons(short highlight)   
{
  uint16_t  cwid, cht; 
  int16_t  xdum, ydum, xWid, yHt, brnd;
  uint16_t widVal;
  uint16_t tClr, brdClr;
  
   if (oskOn || calOn)
		return;
	
   for (int i = 0; i<NUMBUTS; i++)
     if((but[i].menu == currentMenu) && (but[i].siz & DISP_BUT)) 
     {    
      if( (but[i].enableIf == NULL || *(but[i].enableIf) ) && (but[i].displayIf == NULL || *(but[i].displayIf)) )  // enableIf and displayIF
      {
        
        //xWid = (but[i].siz & 2) ? BUTSIZ : BUTSIZ/2; // bitmapped size variable - only two sizes supported
        widVal = (but[i].siz & 0xf0) >> 4;
        if(widVal == 0) // width
          xWid = BUTSIZ/2;
        else
          xWid = widVal * BUTSIZ;

        if(but[i].siz & 0xf == 0) // height
          yHt = BUTSIZ/2;
        else
          yHt = (but[i].siz & 0xf) * BUTSIZ;

        brnd = (!(but[i].siz & 0xff)) ? BUTROUND : BUTROUND/2; // either dimension is small: less rounding
          
          if(i == highlight)
          {
            tft.setTextColor(BHIGHCOL);
            brdClr = BBORDHIGH;
            tClr = but[i].selColour;              
          }
          else
          {
            tft.setTextColor(BTEXTCOL);
            brdClr = BBORD;
            tClr = but[i].unselColour;
          }
          
          if(but[i].onIf != NULL) // onIF text colour override
          {	
            bool temp = *(but[i].onIf);
            if(temp)
            {
              tft.setTextColor(BONCOL_Y);
              brdClr = BBORDHIGH;	
              tClr = but[i].selColour;
              			
            }
            else
            {
              tft.setTextColor(BTEXTCOL);
              brdClr = BBORD;		
              tClr	= but[i].unselColour;	
            }
          }
          
          // special colour treatment for Toggle buttons
          short tsv = but[i].sv;	
          if (tsv != NOSETVAL) // don't combine ifs - could cause pointer error
            if(setVals[tsv].fmt == 'T')
              tClr = tColour(tsv, 'B');

          
          //if(oldHigh != highlight) // context sensitive buttons get redrawn each time
          tft.fillRoundRect(but[i].xpos, but[i].ypos, xWid, yHt,  brnd, tClr); 
              
          if(but[i].border)
            for (short j = 0; j < BUTEDGETHICK; j++) // extra thickness on outside of basic outline
              tft.drawRoundRect(but[i].xpos-j,but[i].ypos-j, xWid+2*j, yHt+2*j,  brnd, brdClr);
          
          switch (strlen(but[i].text)/widVal) // this will produce tiny chars for widVal == 0
          {
            
            case 0:
            tft.setFreeFont(myGFXfonts[3]); // tft.setTextFont(0); tft.setTextSize(3); 
              break;
            case 1:
            case 2:
              tft.setFreeFont(myGFXfonts[2]); // tft.setTextFont(0); tft.setTextSize(2); 
              break;
            case 3:
                tft.setFreeFont(myGFXfonts[1]); //tft.setTextFont(0); tft.setTextSize(2); 
              break;
            default: // 4 or more chars
              tft.setFreeFont(myGFXfonts[0]); //tft.setTextFont(0); tft.setTextSize(1);
          }           
          cwid =  tft.textWidth(but[i].text, GFXFF); 
          cht  = tft.fontHeight(GFXFF);
          if(lumVis(tClr) >= CLRBRKV) // contrast text on background
            tft.setTextColor(ILI9488_BLACK);
          //tft.setCursor(but[i].xpos + (xWid-cwid)/2, but[i].ypos + (yHt + cht)/2);
          tft.drawString(but[i].text, but[i].xpos + (xWid-cwid)/2, but[i].ypos + (yHt + cht)/2, GFXFF); //tft.print(but[i].text);   
            
      }
	 } // end if, if, for
     oldHigh = highlight;
}

short readCntr = 0;
// WAS non blocking code - one reading per call
// ignores legend 
void drawReadings(void) {  
  uint16_t tempClr;
  short i = 0;
  if (oskOn || calOn)
		return;
  //if (readCntr >= NUMREADS)
	  readCntr = 0;	// start again
    //Serial.print("drawReads: ");

  for (short i = 0; i < NUMREADS; i++)
  {
		if(readVals[readCntr].menu == currentMenu)
		{
			// check for conditional display - print in Background colour to erase
			if(readVals[readCntr].displayIf == NULL || *(readVals[readCntr].displayIf)) 
			  tempClr = readVals[readCntr].clr;
			else
			  tempClr = BGCOL;	
      tempClr = readVals[readCntr].clr;	
			printX(&readVals[readCntr], butDigit, tempClr, tempClr, BGCOL, false, false);
      //Serial.printf("ReadVal %i: \n", readCntr);
		}
    readCntr++;
  }
}
  
// selected is the index to setVals[]
// highlight digit for editing
void drawSettings(short selected, short highDigit, bool all) 
{
  short i = 0;
  uint16_t tempClr;
  if (oskOn || calOn)
		return;
	
  //updateVal_Scrn = false;	
  //Serial.printf("DSets menu %i, wave %i[%i], all %i\n", currentMenu, iSet.waveForm[0], iSet.selChan, all); 
  while (setVals[i].vp != NULL)//(short i = 0; i < NUMSETTINGS; i++)
  {
    if(setVals[i].menu == currentMenu) 
    {
	    // check for conditional display - print in Background colour to erase
			if(setVals[i].displayIf == NULL || *(setVals[i].displayIf)) // NULL test first
				tempClr = setVals[i].clr;
			else
				tempClr = BGCOL;

      if(setVals[i].displayIf == NULL || *setVals[i].displayIf)	
      {
        if (i == selected)
        {
            //Serial.print(" Sel ");
            // ensure a legitimate digit is highlighted       
          //numtest = true;  
          //tftDebugChar('R');//delay(1);
          printX(&setVals[i], highDigit, tempClr, HIGHLIGHTCOL, BGHIGH, true, true); 
          //numtest = false;
        }
        else // don't always redraw out of focus settings
          if (all)
          {
            //Serial.printf("A 0x%x, HD %i. ", tempClr, highDigit);//delay(1);
            printX(&setVals[i], highDigit, tempClr, tempClr, BGCOL, false, true);
          }
        
        if(strlen(setVals[i].legend))
        {
          //tftDebugChar('T');//delay(1);
          tft.setFreeFont(myGFXfonts[setVals[i].font]);// set font and text size first, or cursor baseline will be wrong 
          tft.setTextSize(setVals[i].textSize);	
          //tft.setCursor(setVals[i].legx, setVals[i].ypos);
          tft.setTextColor(tempClr, BGCOL);          
          tft.drawString(setVals[i].legend, setVals[i].legx, setVals[i].ypos, GFXFF); //tft.print(setVals[i].legend);
        }
      }   
    }
    i++;
    //Serial.print(i);
  }
  //Serial.printf("DrawSets.done");
  updateVal_Vals = false;	
}

void redrawScreen(void)
{
  short selVal; 
  
  if (oskOn || calOn)
		return;
  
  tft.setRotation(screenCal[5]);
  tft.setTouch(screenCal);
  tft.fillScreen(BGCOL); // 
  tft.setTextColor(TEXTCOL, BGCOL);
  drawLegends(currentMenu);
  //Serial.printf("RDRW: Asel %i, Bsel  %i\n", iSet.waveForm[0],iSet.waveForm[1]);
  if(highButton < 0 || highButton > NUMBUTS)
	  selVal = -1;
  else
	  selVal = but[highButton].sv;
//Serial.printf("3: %i ", selVal);

  drawSettings(selVal, butDigit, true); 
  drawButtons(highButton);
  // readings and indicators redrawn each cycle in main code
  drawReadings();
  //strcat(messageLine,"ABCDE");
    
  // bits and pieces of screen drawing
  switch (currentMenu)
  {
    case 0: // main
     // tft.setFreeFont(FONT1X); // tft.setFreeFont(myGFXfonts[1]);
      //tft.setCursor(sx(5), sy(TOPLEGS));
	    // do this here, rather than as a Reading to save update time.
      //tft.print(comms.instName);
      //tft.drawFastVLine(sx(100), sy(25), sy(170), CYAN_M); // vertical line between readings and settings
      break;
  
    case 8: // COMMS
	    tft.drawFastHLine(sx(2), sy(340), sx(310), CYAN_M); // horz line separating panels
      break;
  
    case 2: //SWEEP
	    //tft.drawFastVLine(sx(BUTX*0.55), sy(20), sy(170), CYAN_M); // vertical line between readings and settings
      break;
	  
	case 3: //CAL
	   //tft.drawFastHLine(LEG2X, TOPLINE+3*BVSPACE2,  HMAX -  2 * LEG2X, CYAN_M);
	   //tft.drawFastHLine(LEG2X, TOPLINE+7*BVSPACE2,  HMAX -  2 * LEG2X, CYAN_M);
     break;
  }
 // Serial.println("RDSx");
  updateVal_Scrn = false;
}

#define BORDER_HL 3 // extra space beyond char footprint of highlight
// Fancy print - with highlighting for currently editing value.
// Only works with readings or settings
// Assumes fixed width font, of width "N".
// prints leading spaces to pad to desired size (align decimals)
// fmt: 'F' print as float; 'I' 16 signed int 'U' 'L' - unsigned 8,  16, 32  bit int (decimal justified); 
//  'P' - float as percentage, 'B' - bracketed Int; 'T' = On/off (1/0; T > 0.1); 
//  'S' = String; 'A' = safe string (no unprintable chars)
//  'Q' - boolean value prints 0, 1 char of setVals.units string
//  'R' - float: multi-select, displays indexed chars from setVals.units
//  'V' - float: multi-select, displays space-separated tokens from setVals.units
// ReadFloat = read float value and display it as required by valFmt. Deprecated, now in xp->valIsFloat
void printX(valFmt * xp, short highlight, uint16_t txtcol, uint16_t highcol, uint16_t bgcol, bool editing, bool readFloat = false)
{  
  if(HOLDSCREEN)
  {
    //Serial.printf("(HWT %i > %i, %i) ",holdScreenUntil, millis(), holdWait);
    return;
  }
  //bgcol = RGB16(0,20,0); // testing block out
  char ch, buf[128], fmt;
  uint16_t xcursor;
  uint16_t  cwid, cht, x, y, pre, post; 
  int16_t  xdum = 100, ydum = 100;
  short digit, powVal;
  float val, pownum, tempVal, max, min;
  uint8_t cCount = 0, fNum;
  short i;
  int maxi;
  x = xp->xpos;
  y = xp->ypos;
  pre = xp->pre;
  post = xp->post;
  fNum = xp->font;
  fmt = xp->fmt;
  max = xp->maxVal;  // always floats
  min = xp->minVal; 
  readFloat = xp->valIsFloat;
  xcursor = x;
  powVal = pre - 1;
  bool printedDot = false;
  bool leadZero = false; 	// true == zeros, false == spaces. Should be a parameter
  bool overflow = false;	// value is greater than allowed
    //Serial.print("H = ");Serial.println(highlight);
  // find the size of the "en" character block - assumes mono-spaced font
  tft.setFreeFont(myGFXfonts[fNum]);
  tft.setTextDatum(DATUM);
  tft.setTextSize(1);
  if (fNum >= 0) // GFX font
  {
    cwid = tft.textWidth("MMMMMMMMMM", GFXFF); 
    cwid = (cwid / 10) -1; // space between characters
    cht = tft.fontHeight(GFXFF);
    cht++;	// a bit more height
  }
  else // GLCD font
  {
    cwid = 5 * xp->textSize; // 8 or 7?
    cht = 8 * xp->textSize;
  }
  // assumes  fmt in "%[pre].[post]f"

//Serial.print("Decimal ");Serial.println(decimal);
  // shift to array index value and skip "."
  if (highlight <= pre)
    highlight--;
    
  // wipe out previous text: n characters + decimal (BG highlight for value being edited)
  //tft.fillRect(x - BORDER_HL, y - cht - BORDER_HL , (pre+post+2)* cwid + BORDER_HL * 2, cht + BORDER_HL * 2, bgcol ); 
  
  //tft.setCursor(x, y); // start of block
  // ignore decimals for anything but Float format
  short nChars = (fmt == 'F')?(pre + post + 1) : pre;
     //Serial.print(fmt);Serial.print(" nChars ");Serial.println(nChars);
  short blocklen = pre + post;
  char bufx[MODETEXTLEN] = "";
  char bufy[MODETEXTLEN] = "";
  char bufch[2] = " ";
  char *sp;
  int k;
  switch (fmt)
  {
    case 'T' : // Toggle
      tft.fillRect(x  + cwid * cCount, y - cht - BORDER_HL , cwid*3 , cht + BORDER_HL * 2, bgcol); //bgcol
	    tft.setTextColor(txtcol, bgcol);       
      if(readFloat)
        val = round(*(float*)(xp->vp));
      else
        val = *(uint8_t*)(xp->vp);
      tft.drawString((val > 0)? "On " : "Off", x, y, GFXFF); //tft.print((*(uint8_t*)(xp->vp) > 0)? "On " : "Off");
      return;
	  case 'Q' :  // boolean: print the first char in units string for T second for F, right justified
      if(readFloat)
        val = round(*(float*)(xp->vp));
      else
        val = *(bool*)(xp->vp);
	    i = (val) ? 0 : 1;
      //	  Serial.printf("X: %i %i %i %i %i\n", i, iSetX.trackSv, iSetX.trackSa, vTrack, cTrack);
      tft.setTextColor(txtcol, bgcol); 
      bufch[0] = xp->units[i];    
      strcat(bufx, bufch);
	    tft.drawString(bufx, x, y, GFXFF); //tft.print((char*)(xp->fmt + i));
      //tft.drawString("   ", x, y, GFXFF);//tft.print("  "); // rubbish erase!!!
      return;
    case 'R' :  // float: print the nth char in units string 
	    i = *((float*)(xp->vp)) + 0.5; // round     
      for(int j = 0; j < pre - 1; j++) // right justify
        strcat(bufx, " ");
      tft.setTextColor(txtcol, bgcol);
      bufch[0] = xp->units[i];
      strcat(bufx, bufch);
      //Serial.printf("DS R: %i [%s]\n", i, bufx, x, y);
	    tft.drawString(bufx, x, y, GFXFF); //tft.print((char*)(xp->fmt + i));;

      return;
    case 'V' :  // float: print the nth token in units string 
	    i = *((float*)(xp->vp)) + 0.5; // round    
     // Serial.printf("DS V: %i, units [%s], ", i, xp->units);
      tft.setTextColor(txtcol, bgcol);
      strcpy(bufx, xp->units);
      sp = strtok(bufx," "); // first token, needs to be a real string
      //Serial.printf("sp [%s]: 0 [%s], ", bufx, sp);
      for(k = 0; k < i; k++) // i-th token
      {
        sp = strtok(0," ");
        //Serial.printf("%i [%s], ", k, sp);
      }
      if (strlen(sp) < 5) // safety at end of string
      {
      for(int j = 0; j < pre - strlen(sp); j++) // right justify
        strcat(bufy, " ");
        Serial.printf("%i [%s], ", j, bufx);        
      }
      strcat(bufy, sp);
      //Serial.printf("print[%s]\n",  sp);  
	    tft.drawString(bufy, x, y, GFXFF); //tft.print((char*)(xp->fmt + i));;
    
      return;
    // use SAFE 'A' format for all string variables
    case 'A' : // SAFE string text (may have characters outside 0x20 .. 0x7e range)
      strcpy(buf, (char*)(xp->vp)); 
       //Serial.println(buf);
      for (i = 0; i < strlen(buf); i++) // substitute unprintable chars
        if (buf[i] < 0x20 || buf[i] > 0x7d)
          buf[i] = '*';
      //Serial.println(buf);
      //delay(500);
	    tft.setTextColor(txtcol, bgcol);
      tft.drawString(buf, x, y, GFXFF); //tft.print(buf);
      //tft.drawString("   ", x, y, GFXFF);//tft.print(" "); // rubbish erase!!!
      return;
    case 'S' :  // unvalidated string - really only for const strings
      tft.setTextColor(txtcol, bgcol);
	    tft.drawString((char*)(xp->vp), x, y, GFXFF); //  tft.print((char*)(xp->vp)); 
      //tft.drawString("   ", x, y, GFXFF);//tft.print(" "); // rubbish erase!!!
      return;
    case 'B': // Bracketed 
      tft.setTextColor(txtcol, bgcol);
	    tft.drawString("[", x, y, GFXFF);//tft.print("[");
      cCount++;
    case 'I': // 16-bit  Integer: convert value to float for display)
      if(readFloat)
        val = round(*(float*)(xp->vp));
      else
        val = *(int16_t*)(xp->vp);          	  
  	  if (val > max + MEDIUM_DIFF)
  	  {
  	    overflow = true;
  	  //  Serial.printf("prinfX I overflow: V %1.1f, M %1.1f\n",val, max) ;
  	  }
      break;
	  case 'U': // 16-bit unsigned Integer: convert value to float for display
      if(readFloat)
        val = round(*(float*)(xp->vp));
      else
        val = *(uint16_t*)(xp->vp);  
      if (val > max + MEDIUM_DIFF)
      {
        overflow = true;
      //  Serial.printf("prinfX U overflow: V %1.1f, M %1.1f\n",val, max) ;
      }
      break;
   case 'L': // 32-bit unsigned Integer: convert value to float for display   
      if(readFloat)
        val = round(*(float*)(xp->vp));
      else
        val = *(uint32_t*)(xp->vp);
      if (val > max + MEDIUM_DIFF)
      {
        overflow = true;
        //Serial.printf("prinfX L overflow: V %1.1f, M %1.1f\n",val, max) ;
      }
      break; 
    case 'F': // Float
      val = *((float*)(xp->vp));
      blocklen++; // decimal point	  
      if (val > max)
      {
      overflow = true;
      //Serial.printf("prinfX F overflow: V %3.2f, M %3.2f\n",val, max) ;
      }
    // Serial.printf("PFX: %6.4f [round %i]\n", val, post);
      val = myRound(val, post); // round to appropriate number of digits for display (stop the 2.99 .. 3.00 flashing issue	  
      break;
    case 'P': // Float as percentage
      val = *((float*)(xp->vp)) * 100;
	  //Serial.printf(" %5.1f%\n", val);
      break;
  }
  // clear display block. Sign is to left of defined display block (only displayed if min value < 0)
  tft.fillRect(x-cwid, y - cht, cwid * (nChars+1), cht, bgcol); // erase block  bgcol + BORDER_HL * 2

  if (overflow)	// blank space, print ***
  {	 
	  //tft.fillRect(x-cwid, y - cht - BORDER_HL , (cwid * blocklen + 1), cht + BORDER_HL * 2, bgcol); //bgcol
	  tft.setTextColor(txtcol, bgcol);
	  tft.drawString("***", x - cwid, y, GFXFF);
	  return;
  }

	 tft.setTextColor(txtcol, bgcol);
   bool negVal = false;
   if (val < 0) // sign
   {
     val = -val;
     negVal = true;
     if(editing)
      tft.drawString("-", x - cwid, y, GFXFF);
   }
   else
     tft.drawString("  ", x - cwid, y, GFXFF);
    
//if(numtest)  Serial.printf("\nPFX %5.3f \n", val);
	// print the value one character at a time, highlight digit if indicated
  // start from most significant digit
  bool stillLeadingSpaces = true;
  for (short i = 0; i < nChars ; i++)
  {
    // y is baseline, erase block as each char is redrawn
    if(powVal == -1 && post > 0) //decimal point precedes numeral except when post == 0
    {
      tft.setTextColor(txtcol, bgcol);     
      tft.drawString(".", x, y, GFXFF);
      x += cwid;
      cCount++;
      i++; // added in an extra char for floats
      printedDot = true; // no spaces after decimal
      stillLeadingSpaces = false;
    }
	  //if(cCount < nChars) // allow for decimal		 
	
    if (powVal == highlight + 1) // highlighted digit
      tft.setTextColor(highcol, bgcol ); 
    else
      tft.setTextColor(txtcol, bgcol);

    // from most signif digit; extract the digit; subtract its value from the number (i.e. calc remainder)
    pownum = pow(10, powVal);
    //if(numtest) Serial.printf("PN %6.3f | ", pownum);
    tempVal = val/pownum; // myRound(pownum, -powVal);
    //if(numtest) Serial.printf("TV %8.5f | ", tempVal);
    //if(numtest && powVal == -post) Serial.print("*");
	  //if(i == nChars -1)
	   //digit =  myRound(tempVal + SMALL_DIFF,1); //  truncate to single digit integer (round **final** digit to one decimal place to avoid trunc issue)
    //else
  	digit = tempVal + VSMALL_DIFF;  // float to integer truncate, avoid rounding errors 
    //if (digit < 0)        digit = 0;
    val = val - digit * pownum;    
    //if(numtest) Serial.printf("D[%i] %i | RV %6.3f\n", powVal,digit, val);
  
    // print the digit, or a leading space
    itoa(digit, buf, 10);

    // print leading spaces when leading zeros are not required.
    // always leading zeros for Bracketed format 
    if(digit == 0 && !leadZero && !printedDot && fmt != 'B') 
    { 
      if (powVal == 0) // single zero before decimal
	    {           
        tft.drawString(buf, x, y, GFXFF);
        stillLeadingSpaces = false;
        //if(numtest) Serial.printf("[%s]", buf);
      }
      else // is a 0 and not leading
	    {                  
        if(editing) // always draw leading 0 
        {     
          tft.drawString(buf, x, y, GFXFF);          
        }       
        else 
        {  
          tft.drawString("  ", x, y, GFXFF);           
        }
	    }
    }
    else // non-zero digit
    {  
      if(stillLeadingSpaces && negVal && !editing)  // "-" sign in front of first digit
         tft.drawString("-", x - cwid, y, GFXFF);   
      stillLeadingSpaces = false;  
      if(!(powVal < 0 && post == 0)) // no trailing zero for post == 0
      {  
        tft.drawString(buf, x, y, GFXFF);
      }
	    //if(numtest) Serial.printf("[%s]", buf);
      printedDot = true;
    }
    powVal--;
    cCount++;
    x += cwid;
  } // end character loop

  if(strlen(xp->units) && xp->fmt != 'R' && xp->fmt != 'V') // text post-nominal (usually unit type, e.g. mA) R & V use units as value display
  {
    // TextSize and extended character set only work for inbuilt font.
    if (fNum > 0) 
      tft.setFreeFont(myGFXfonts[fNum -1]);	
 
    tft.setTextColor(highcol, BGCOL);
    tft.drawString(xp->units, x+SMALLSHIFTX, y-SMALLSHIFTY, GFXFF);//tft.print(xp->units);
    tft.setFreeFont(myGFXfonts[fNum]);
  }

  if (fmt == 'P')
  {
    tft.print("%");
  }

  if (fmt == 'B')
    tft.drawString(xp->units, x, y, GFXFF);
}  

#define HLEFT_E (HMAX/10)
#define WIDTH_E (HMAX*4/5)
#define VTOP_E (VMAX/10)
#define HEIGHT_E (VMAX*4/5)
#define LINESP_E 24
#define TEXTSIZ_E 2
#define BORDER_E 2
#define MARGIN_E 10
#define ROUND_E 10
// onTime in seconds
#include "platyImg2.h"

void waitOne(void) // mS
{
  delay(1);
  yield();
}

// notifications and minor errors
// displays on TFT TXTLINE, web message area and goes into SCPI errors.
// Should be a single line of less than MMSGLEN chars to fit on TFT
void errMessage(const char * message, int16_t onTime)
{
    // avoid buffer overruns
   // if(strlen(messageLine) + strlen(message) < MULTIERRORLEN)
    //  strcat(messageLine, message);  
    //else
      strncpy(messageLine, message, MULTIERRORLEN);  
    Serial.printf("errM %i: [%s]\n",strlen(messageLine), messageLine);
    messageExpires = millis() + (onTime * 1000);
}
// used for major messages and errors - TFT only
// onTime < 0 == wait for screen touch
void screenError(const char * message, uint16_t bgcol, int16_t onTime, bool logo)
{    
   Serial.printf("%s\n",message);
   if(bgcol == ERR_BG_ERR)
    strcat(postMsg,message);
#ifndef NO_SCREEN_ERRORS
   if(strlen(message) == 0)
#endif 
     return;

   char scrBuf[128];
   strcpy(scrBuf, message);  
   if(onTime < 0)
   {
     strcat(scrBuf, "\n\nWaiting for\nscreen touch...");
   }
	 int i, ycursr = VTOP_E + LINESP_E;
   tft.setRotation(screenCal[5]);
	 tft.fillRoundRect(HLEFT_E, VTOP_E, WIDTH_E, HEIGHT_E, ROUND_E, bgcol);
	 for(i = 0; i < BORDER_E; i++)
		tft.drawRoundRect(HLEFT_E + i, VTOP_E + i, WIDTH_E - 2 * i, HEIGHT_E - 2 * i, ROUND_E, HIGHLIGHTCOL);
	 tft.setTextColor(HIGHLIGHTCOL);
	 tft.setFreeFont(&FONT1); 
	 //tft.setTextSize(TEXTSIZ_E);
	 tft.setCursor(HLEFT_E + MARGIN_E, ycursr );
	 for (i = 0; i < strlen(scrBuf); i++)
	 {
		 if (scrBuf[i] == '\n' || scrBuf[i] == '\r' )
		 { 
			ycursr += LINESP_E;
			tft.setCursor(HLEFT_E + MARGIN_E, ycursr);
		 } else
			 tft.print(scrBuf[i]);
	 }  

	 if(logo) // 87w x 64h pixels
#ifdef ILI9488 // needs FG colour as well as BG
		 tft.drawBitmap(HMAX/2-96/2, VTOP_E + HEIGHT_E - 96 - MARGIN_E, platy_img2, 128, 96, bgcol, MY_WHITE);
#else
		 tft.drawBitmap(HMAX/2-87/2, VTOP_E + HEIGHT_E - 64 - MARGIN_E, platy_img2, 128, 96, BGCOL);
#endif
   if(onTime <= 0) // display message and wait
   {
      holdWait = true;
      holdScreenUntil = 0; // release immediately on touch
   }
   else    
      holdScreenUntil = millis() + onTime * 1000;  	
	 updateVal_Scrn = true;
}

void splashScreen(void)
{
	char buf[256];
	sprintf(buf, "WiFi Programmable\nFunction Generator\nModel %s\nVersion %i\nFor EEPROM V%i", MODEL, SOFT_VERSION, EE_VERSION);
	screenError(buf, ERR_BG_MSG ,5, true);
}

#endif
