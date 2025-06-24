/* Touch Screen Processing and Value Editing 
myTouchProcess.h
*/
#ifndef MYLTOUCHPROC_H
#define MYLTOUCHPROC_H

void postProcess(void);
int blockTouched(TS_Point p);
void updateVal(short highButton);
void printModes(void);
bool netBegin(void);

TS_Point touch;
bool touched(void)
{
  uint8_t res = tft.getTouch(&touch.x, &touch.y, TOUCH_THRESH);
  //if(res >= MIN_TOUCH) Serial.printf("Touched() [%i, %i]\n",touch.x, touch.y);
  return (res >= MIN_TOUCH);
}

TS_Point getPoint(void)
{
  touch.z = TOUCH_THRESH + 1;
  return touch;
}

/*
#define TOUCH_OVERFLOW 5000
bool getTouch(uint16_t *x, uint16_t *y)
{
  bool res;
  res = tft.getTouchRaw(x, y);
  tft.convertRawXY(x, y);
  if (*x > TOUCH_OVERFLOW)
    *x = 0;
  if (*y > TOUCH_OVERFLOW)
    *y = 0;
    return res;
}
*/
// wireless enabled
int toggleWconn(int x)
{
	comms.enabled = !comms.enabled;
//Serial.printf("Wifi Auto = %i\n",comms.enabled);
#ifdef WIFI_ON
	if(comms.enabled) // restart Wifi
	{
		netBegin();
	}
	else
	{
		wifiEnd(false); // leave the wifi adapter on
	}
#endif
	valChange(VAL_CHGD_LOCAL); // no broadcast
	return CALL_EX; // CALL_NOEX ??
}

/*globals used by other routines
  highButton - the button currently highlighted on screen
  updateVal_Scrn - something has changed that requires the entire screen to be re-drawn (e.g. menu change)
  valChange(VAL_CHGD_LOCAL) - requires settings to be redrawn/ changed in EEPROM / SCPI etc
 * parameter anyTouch: set TRUE to wait for touch anywhere on the screen (e.g. to clear error message)
 * returns: TRUE when anyTouch is TRUE and the screen has been touched, FALSE otherwise
 */
// process touch screen, switches and encoder
bool firstCall = true;
short procBut = -1, oldButton = -1;
bool processTouchSwEnc(bool anyTouch = false)
{
  short blockT, cur_sv = NOSETVAL, retVal, cmd;
  char lfmt = '\0';
  int cb = CALL_EX;	// default callback result
  bool oskStatus;
  newPosition = getEncPos(); // encoder change?
  
  // anyTouch just tests for a screen touch. Blocks any other processing 
  if (anyTouch) 
  {
    if (touched()) 
      return true;
    return false;
  }
  if (touched())       // keep checking until released - then use value.
  {
    p = getPoint(); 
    wasTouched = true;    
  } else 
  {
    if (wasTouched) 
    {   
      wasTouched = false;
      blockT = blockTouched(p); 
      //Serial.printf("Touch B %i M %i (%i, %i) [%i, %i]\n",blockT, currentMenu, p.x, p.y, p.x, p.y);
      // keep last touched block until another legal block is touched - ignored for OSK and ScreenCal
      if (blockT >= 0 && !oskActive()) // ignore touches outside buttons and while OSK is displayed.
      {	
        //Serial.printf("Button text: %s\n", but[blockT].text);
        //Serial.printf("Touch: block = %i [M %i], status %i, %s\n",blockT, currentMenu, callStatus, (but[blockT].callback != NULL) ? "CB" : "No CB" );
        // callbacks may persist for multiple cycles (state-variable) and must be non-blocking
        // not currently in an active callback
        if (callStatus == CALL_IDLE) 
        {	
          highButton = blockT; // persistent record of last menu button pressed
          cur_sv = but[blockT].sv; 
          if(cur_sv != NOSETVAL)
            lfmt = setVals[cur_sv].fmt; 
          else
            lfmt = 'T'; // T is fairly safe
          //Serial.printf(" setting %i[%i], format[%c]\n", cur_sv, highButton, lfmt);	delay(10);
          
          if(but[blockT].callback != NULL)  //need to process a *new* callback?
          {												
            procBut = blockT;
            firstCall = true;	
            callStatus = CALL_START;
            //Serial.printf("Callback starting\n");					
          }	

          //  OSK needs special initialisation: touched block's related setVal.sv has ASCII ('A' or 'S') format
          if ((lfmt == 'A'|| lfmt == 'S') && !oskOn )
          {
            Serial.printf("OSK starting on [%s]\n",(char *)setVals[cur_sv].vp);
            oskStart((char *)setVals[cur_sv].vp, 100, cur_sv); // nominal ypos, OSK could calc this for bottom part of the screen
          }					
        }
      }

      if (oskActive())
      {
        oskStatus = oskProcess(CALL_CMD_PROC);
      }
      // callback on originally highlighted button (screenCal and OSK) ??? for toggles?
      // use procBut here to maintain thread, despite what's touched on the screen
      if (but[procBut].callback != NULL && callStatus != CALL_IDLE)
      {
        if (firstCall) 
        {
          cmd = CALL_CMD_START;
          firstCall == false;
        }
        else
          cmd = CALL_CMD_PROC;
        
       // Serial.print("C ");  delay(10);
        cb = but[procBut].callback(cmd); // issue callback
        
        switch (cb)
        {
          case CALL_PROC:
            callStatus = CALL_ACTIVE;
            retVal = procBut; // UNUSED VALUE. possibly some post-processing 
          break;
          case CALL_EX:
            callStatus = CALL_IDLE;
            wasTouched = false;
            retVal = -1; // possibly some post-processing 
            break;
          case CALL_NOEX: 
            callStatus = CALL_IDLE;
            wasTouched = false;
            retVal = procBut; // retain focus on this button
            break;
          case CALL_ERR:
            callStatus = CALL_IDLE;
            wasTouched = false;
            return false; // it's broken, so just exit						
        }
      } // end callback processing 					
    } // end wasTouched
  } // end screen button not touched now or before
	// encoder and buttons 	- independent of screen touch
	if (callStatus == CALL_IDLE)  // not during active callbacks 
	{
		// establish setVal and format for this button
		if (highButton >= 0 && highButton < NUMBUTS)
		{	
			cur_sv = but[highButton].sv; 	
			if(cur_sv == NOSETVAL)
				lfmt = 'T'; // toggle is farily safe
			else
				lfmt = setVals[cur_sv].fmt;  			
		}
		else 
		{
			cur_sv = NOSETVAL;
			lfmt ='T';
		}
	  // encoder processing	  
		if (newPosition != oldPosition) // encoder rotated, update value associated with last block touched
		{
	  //Serial.printf("Enc R %i <- %i. ", newPosition,oldPosition); 
		  updateVal(highButton);   
		  oldPosition = newPosition; 		
      // just update the current setting
      int svx = but[highButton].sv;  
      printX(&setVals[svx], butDigit, setVals[svx].clr, HIGHLIGHTCOL, BGHIGH, true, true); 
      if(highButton == LANBUTTON)
      {
        setComMenu(99);
        valChange(VAL_CHGD_LOCAL); // needs to update SSID/PASS
      }
      else
		    valChange(VAL_CHGD_LOCAL_NOUPD);    
		}
		
		if (swPressed && lfmt == 'T' && cur_sv != NOSETVAL && setVals[cur_sv].vp != NULL) // L or R switch pressed (Toggles)
		{	
	  //Serial.print("Sw T ");
			*(uint8_t*)setVals[cur_sv].vp = (butDir > 0) ? false : true;  	
			//  button colours are handled dynamically in drawIndicators and drawButtons
			//Serial.print("P ");
			valChange(VAL_CHGD_LOCAL_NOUPD);	 				
			swPressed = false;						
		}	  
    
    if (digSwPressed  && cur_sv != NOSETVAL && setVals[cur_sv].vp != NULL)
    {
      drawSettings(cur_sv, butDigit, false); //just redraw the currently edited button
      digSwPressed = false;
     // Serial.print("Z. ");
      updateVal_Scrn = false;
    }	

		// whole screen redraw
		if ((updateVal_Scrn || (highButton != oldButton)) && !HOLDSCREEN) 
		{   		 
		 	//Serial.printf("TSE:Redraw uV_S %i, Hi %i, old %i\n", updateVal_Scrn, highButton, oldButton); 
      //Serial.printf("CSV %i,  BD %i, HB %i, CM %i\n", cur_sv, butDigit, highButton, currentMenu);	
      redrawScreen();		  
		  oldButton = highButton; 
		}
		 
		 // less radical redraw
    if (updateVal_Vals && !HOLDSCREEN)
    {  			
  			//drawButtons(highButton);			  
  			//drawIndicators(currentMenu);	
  			drawSettings(cur_sv, butDigit, (oldButton != highButton)? true : false);
  			//Serial.print("Q. ");
        //updateVal_Vals = false;	
		}
	} // CALL_IDLE
  return wasTouched;
}

int blockTouched(TS_Point p)
{
	uint16_t xWid, yHt, widVal;
    // which button has been pressed in the current menu?
    for (int i = 0; i< NUMBUTS; i++)
    { 
      widVal = (but[i].siz & 0xf0) >> 4; // mirrors drawButtons() code
      if(widVal == 0) // width
        xWid = BUTSIZ/2;
      else
        xWid = widVal * BUTSIZ;

      if(but[i].siz & 0xf == 0) // height
        yHt = BUTSIZ/2;
       else
        yHt = (but[i].siz & 0xf) * BUTSIZ;

      if (but[i].menu == currentMenu 
            && (p.x >= but[i].xpos) && (p.x <= (but[i].xpos + xWid))
            && (p.y >= but[i].ypos) && (p.y <= (but[i].ypos + yHt))
          )
      {
        if(but[i].enableIf == NULL || *(but[i].enableIf)) // conditional button [displayIf used by drawButtons()]
        {		  /*     
          Serial.printf("CM = %i, Touched (%i,%i) [%i, %i]",currentMenu, p.x, p.y, p.x,p.y);
          Serial.print(but[i].text);  
          Serial.print(", Highlight ");
          Serial.println(highButton);
          */
          // change menu if required
          if (currentMenu != but[i].nextMenu)
          {
            highButton = 999;  // no highlight in new menu
            updateVal_Scrn = true;
            //Serial.print(" BT DS ");
          }
          currentMenu = but[i].nextMenu; // could be the same as this one        

          return i; // no need to try the rest of the blocks
        } 
      }
    }
    return -1;
}
// update the setting value using the (touch button) index to the settings array and the (L&R buttons) digit to change
// encoder position is global
// for simplicity, all arithmetic is done in float: convert *vp -> float -> process -> save as native format
// alters butdigit if it's outside the range of the input.
void updateVal(short btn)
{
  float vTemp, lTemp;
  char fTemp;
  uint8_t svx;
 
 // float minVal, maxVal;     
  
  if (btn > NUMBUTS  || btn < 0) // no button highlighted
    return;

  svx = but[btn].sv;
  if(svx == NOSETVAL)
    return;
   bool asFloat = setVals[svx].valIsFloat;
  //tftDebugChar('Z');//delay(1);

  fTemp = setVals[svx].fmt;	
  if (fTemp == 'I' && butDigit < 0) // integers can't alter 0.xx values. Move to 1's digit
	  butDigit = 0;

 // minVal = setVals[svx].minVal;
  //maxVal = setVals[svx].maxVal;

  //Serial.printf("updateVal: btn %i, digit %i, setting %i, Type %c",btn, butDigit,svx, fTemp);

  switch (fTemp) // ensure digit being highlighted/edited is within pre-post range
	{
	  case 'I': //  8 bit unsigned Integer 
	  case 'U': //  16 bit unsigned Integer 
    case 'L': //  32 bit unsigned Integer     
	  case 'B': // bracketed integer
	  case 'F': // float
    case 'R': // float flag char
    case 'V': // float flag token
		if (butDigit >= setVals[svx].pre)    butDigit = setVals[svx].pre -1;
		if (butDigit < (-setVals[svx].post)) butDigit = -setVals[svx].post;
		break;
	  // do nothing for text and toggle
	}
  switch (fTemp) 
  {
     case 'T': // Toggle
       if(asFloat)
         vTemp = round(*(float*)setVals[svx].vp);
       else
         vTemp = *(uint8_t*)setVals[svx].vp;
       butDigit = 0; // true/false always +/- 1
       break;    
     case 'I': //  8 bit unsigned Integer 
       if(asFloat)
         vTemp = round(*(float*)setVals[svx].vp);
       else
        vTemp = *(uint8_t*)setVals[svx].vp;
       break;
	   case 'U': //  16 bit unsigned Integer 
       if(asFloat)
         vTemp = round(*(float*)setVals[svx].vp);
       else
         vTemp = *(uint16_t*)setVals[svx].vp;
       break;
     case 'L': //  32 bit unsigned Integer 
       if(asFloat)
         vTemp = round(*(float*)setVals[svx].vp);
       else
         vTemp = *(uint32_t*)setVals[svx].vp;
       break;
     case 'B': // bracketed integer
       if(asFloat)
        vTemp = round(*(float*)setVals[svx].vp);
       else
        vTemp = *(uint8_t*)setVals[svx].vp ;
       break;
     case 'R': // float flag char
     case 'V': // float flag token
       butDigit = 0; // float, single digits only
     case 'F': // float
       lTemp = vTemp = *(float*)setVals[svx].vp ;
       break;    
  }
  //Serial.printf("val %6.4f,",vTemp);
  // all calcs in float
  if (newPosition > oldPosition) // increment
  {
   //Serial.print("+");
    //Serial.print(setVals[svx].maxVal);
    //  maximum set value 
    if ((vTemp + pow(10, butDigit) ) <=  setVals[svx].maxVal)
        vTemp += pow(10, butDigit);
    else  
        vTemp = setVals[svx].maxVal;
  }
  else // decrement
  {
   // Serial.print("-");
    //  minimum set value      
    if((vTemp - pow(10, butDigit) ) >=  setVals[svx].minVal)
      vTemp -= pow(10, butDigit);
    else  
      vTemp = setVals[svx].minVal;
  }
  //Serial.printf("-> %6.4f\n",vTemp);
  //Serial.println(vTemp);
  switch (fTemp) {
    case 'T': // Toggle
      if(asFloat)
        *(float*)setVals[svx].vp = vTemp;
      else
        *(uint8_t*)setVals[svx].vp = (uint8_t)vTemp;
      break;    
    case 'I': // 8 bit Integer 
      if(asFloat)
        *(float*)setVals[svx].vp = vTemp;
      else
        *(uint8_t*)setVals[svx].vp = (uint8_t)vTemp;
      break;
	  case 'U': // 16 bit unsigned Integer 
      if(asFloat)
        *(float*)setVals[svx].vp = vTemp;
      else
        *(uint16_t*)setVals[svx].vp = (uint16_t)vTemp;
      break;
    case 'L': // 32 bit unsigned Integer
      if(asFloat)
        *(float*)setVals[svx].vp = vTemp;
      else 
        *(uint32_t*)setVals[svx].vp = (uint32_t)vTemp;
      break;
    case 'B': // bracketed integer
      if(asFloat)
        *(float*)setVals[svx].vp = vTemp;
      else
       *(uint8_t*)setVals[svx].vp = (uint8_t)vTemp;
      break;
    case 'F': // float
    case 'R': // float flag char
    case 'V': // float flag token
      *(float*)setVals[svx].vp = vTemp;
      break;    
  }
  //valChange(VAL_CHGD_LOCAL);
  //changedLocal = true;
  
  // Sweep limits vary for different functions
  if (currentMenu == SWEEP_MENU && svx == 0)
  {
    //Serial.println("Sweep FAD has changed");
    setSweepTypeInfo((int) vTemp, (int) lTemp);
  }
}

/*
void printButs(int firstFew) // debug
{
  int widVal, xWid;
  if (firstFew < 0)
    firstFew = NUMBUTS;
  Serial.printf("First %i buttons\n", firstFew);
  for(int i = 0; i < firstFew; i++)
  {
    widVal = (but[i].siz & 0xf0) >> 4; // mirrors drawButtons() code
      if(widVal == 0) // width
        xWid = BUTSIZ/2;
      else
        xWid = widVal * BUTSIZ;
    Serial.printf("0x%03x: val %i, pix %i\n ", but[i].siz, widVal, xWid);
  }
}
*/

#endif
