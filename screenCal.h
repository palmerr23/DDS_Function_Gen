/* Screen management
  Touch Screen Calibrator

  TFT/WiFi side *mode* management 
  updates modeVal, sweepVal, iSet, burst, etc - doesn't touch DDS.x swCont.x or burst.x
  called from CPU0
 
Need to define HMAX/VMAX = screen width/height in pixels
These are the touch screen readings corresponding to pixels (0,0) & (HMAX,VMAX)
	int thmin = ;
	int tvmin = ;
	int thmax = ;
	int tvmax = ;
*/
#ifndef MYSCREENCAL_H
#define MYSCREENCAL_H

bool getESPviReadings(void);
inline float readESP_V(void);
void currentZeroCal(void);
//void setOnOffxx(int8_t channel, bool status);
bool daughterSense(void);
void errorKill(bool shutOff, bool DACzero, float reading, const char errMsg[], bool nonBlocking);
void setScrMode(uint8_t chan, DDSwaves mode, int menu);
void setMode(DDSmodes mode);
void cancelSweepBurst(void);
void redrawScreen(void);
void setSweepTypeInfo(int newVal, int lastVal);
void setOSKtouchCal();

bool calOn = false;
uint8_t calStage = 0, hitcount = 0;
int scx1 = 0, scy1 = 0, scx2 = 0, scy2 = 0;
float slopex, slopey;
#define SC_HITCOUNT  5
#define SC_DIST	20		// distance of '+' from corners
#define SC_LINELEN 20	// length of '+' bars
#define SC_BG RGB666(0,20,15)
#define SC_BG2 RGB666(31,10,0)
#define SC_HIGH  0xffff

void calStop(void){
	calOn = false;
	updateVal_Scrn = true;
	//callStatus == CALL_IDLE;
}

void printScreenCal()
{
  Serial.println("Screen [touch] cal:");
  for (int i = 0; i < 6; i++)
    Serial.printf(" %i: %i [%i]\n", i, screenCal[i], screenCalOSK[i]);
}

int setScreenCal(int dummy) 
{
  tft.fillScreen(ILI9488_NAVY);
  tft.setFreeFont(myGFXfonts[1]);
  tft.setTextSize(1);	
  tft.drawString("Screen Calibration", 40, 160, GFXFF);
  tft.drawString("Touch arrows at", 40, 210, GFXFF);
  tft.drawString("all four corners", 40, 240, GFXFF);
  tft.calibrateTouch(screenCal, YELLOW_VL,ILI9488_NAVY,20);
  setOSKtouchCal();
  printScreenCal();
  return 1;
}

void setOSKtouchCal()
{
  memcpy(screenCalOSK, screenCal, sizeof(screenCal));
  screenCalOSK[4] = 1;  // some manipulation may be required!
  screenCalOSK[5] += iSet.touchRot;
}


int tsRotate(int x){ // rotate touch screen 180 degrees
	sc.tsRot = (sc.tsRot + 2 ) % 4;
  //Serial.println("tsRotate deprecated");
	valChange(VAL_CHGD);
	//ts.setRotation(screenCal[5]);
  updateVal_Scrn = true;
	return CALL_EX;
}
int scrRotate(int x){ // rotate display screen 180 degrees
  sc.scrRot = (sc.scrRot + 2 ) % 4;
  valChange(VAL_CHGD);
  tft.setRotation(screenCal[5]);
  updateVal_Scrn = true;
  //Serial.printf("Changed ROT to %i\n", sc.scrRot);
  return CALL_EX;
}


// ***************** DAC calibration ********************
// called by button press
/*
int calEntry(int dummy)
{
 // Serial.println("Cal entry");
	return 0;
}
*/
int calSave (int dummy)
{
  vCal[0] = vCalTmp[0] * vCal[0] / CALSET; // referenced to current cal setting
  vCal[1] = vCalTmp[1] * vCal[1] / CALSET;
	return 0;
}

int exitCal(int x)
{
// Nothing to do
//Serial.println("Cal exit");
	return 0;	
}

// funcMode is waveform for this channel or BURST or SWEEP.
void setScrMode(uint8_t chan, DDSwaves funcMode, int menu)
{  
  DDSwaves mode_is;
  // use modeVals for other menus? Variables have different types.
  int start, end;
  //int butStart;
  switch (menu) // either a waveform, or BURST or SWEEP values to set
  {
    case MAIN_MENU :  
      mode_is = funcMode;
      //butStart = MAIN_BUTS;
      //iSet.waveForm[chan] = funcMode;
      break;

    case FUNC_MENU :   
      mode_is = funcMode;
      //Serial.print(" FUNC MENU ");
      //butStart = MAIN_BUTS;
      break;
      
    case SWEEP_MENU :     // only allowed for CHAN_A
      if(chan != CHAN_A)
        return;
      mode_is = WA_SWEEP;
      iSet.modeA = MO_SWEEP;
      //butStart = MAIN_BUTS;
      break;

    case BURST_MENU :   // only allowed for CHAN_A 
      if(chan != CHAN_A)
        return;
      mode_is = WA_BURST;
      iSet.modeA = MO_BURST;
      break;

    case CONT_MENU :  
      mode_is = WA_CONT;
      break;
  }
  
  //Serial.printf("setScr: menu %i, mode_is %i <=> funcMode %i\n", menu, mode_is, funcMode);
  strcpy(but[FUNC_BUTTON].text, modeDefs[mode_is].name);

 // values from modeVal[], except for sweep (patched later)
    for(int i = 0; i < MODESETVALS; i++) // push settings into first MODESETVALS elements in but[] & setVals[].
    {
      setVals[i].menu = menu;    // display seTval in this menu
      if(i < modeDefs[mode_is].nValues && menu != FUNC_MENU)
      {  
        // *** generic, patched later for SWEEP in setSweepTypeInfo()
        if(menu == SWEEP_MENU)//values from sweepVal[]
          setVals[i].vp = NULL;
        else
          setVals[i].vp = &modeVal[chan][mode_is][i];
        setVals[i].minVal = modeDefs[mode_is].lowLim[i]; 
        setVals[i].maxVal = modeDefs[mode_is].highLim[i];
        
        strcpy(setVals[i].legend, modeDefs[mode_is].text[i]);
        strcpy(setVals[i].units, modeDefs[mode_is].units[i]);
        setVals[i].fmt =  modeDefs[mode_is].fmt[i];
        //setVals[i].pre =
        setVals[i].post =  modeDefs[mode_is].post[i];
        setVals[i].clr  =  modeDefs[mode_is].clr[i];

        setVals[i].displayIf = &trueVal;
        but[i].enableIf = &trueVal; // reuse main screen buttons
        but[i].menu = but[i].nextMenu = menu;
        //Serial.printf("%i: EN [%s]\n",i, setVals[i].legend);
      }
      else // disabled buttons and setVals
      {
        setVals[i].displayIf = &falseVal;
        but[i].enableIf = &falseVal;
        //Serial.printf("%i: DIS [%s]\n",i, setVals[i].legend);
      }
  
      //but[i].menu = menu; // SEL_A BUT?
      //but[i].nextMenu = menu;
      currentMenu = menu;
    }
  if(chan < CHANS)
    iSet.selChan = chan;
  Asel = (iSet.selChan == CHAN_A);
  Bsel = !Asel;  
  setMode(iSet.modeA);
  valChange(VAL_CHGD_LOCAL);   //  Send message to CPU1 
}

// set the DDS mode for the current  channel
int setSine(int x)
{
  //Serial.println("WA_SINE");
  iSet.waveForm[iSet.selChan] = WA_SINE;
  setScrMode(iSet.selChan, WA_SINE, MAIN_MENU);
  cancelSweepBurst(); // reset active sweep/burst
  strcpy(but[FUNC_BUTTON].text, modeDefs[WA_SINE].name); 
  valChange(VAL_CHGD);
  return CALL_EX;
}

int setIMD(int x)
{
  //Serial.println("WA_IMD");
  iSet.waveForm[iSet.selChan] = WA_IMD;
  setScrMode(iSet.selChan, WA_IMD, MAIN_MENU);
  cancelSweepBurst(); // reset active sweep/burst
  strcpy(but[FUNC_BUTTON].text, modeDefs[WA_IMD].name);
  valChange(VAL_CHGD);
  return CALL_EX;
}

int setWhite(int x)
{
 // Serial.println("WA_WHITE");
  iSet.waveForm[iSet.selChan] = WA_WHITE;
  cancelSweepBurst(); // reset active sweep/burst
  setScrMode(iSet.selChan, WA_WHITE, MAIN_MENU);
  strcpy(but[FUNC_BUTTON].text, modeDefs[WA_WHITE].name);
  valChange(VAL_CHGD);
  return CALL_EX;
}
int setSqr(int x)
{
  iSet.waveForm[iSet.selChan] = WA_SQR;
  setScrMode(iSet.selChan, WA_SQR, MAIN_MENU);
  cancelSweepBurst(); // reset active sweep/burst
  strcpy(but[FUNC_BUTTON].text, modeDefs[WA_SQR].name);
  valChange(VAL_CHGD);
  return CALL_EX;
}
int setTri(int x)
{
  iSet.waveForm[iSet.selChan] = WA_TRI;
  cancelSweepBurst(); // reset active sweep/burst
  setScrMode(iSet.selChan, WA_TRI, MAIN_MENU);
  strcpy(but[FUNC_BUTTON].text, modeDefs[WA_TRI].name);
  valChange(VAL_CHGD);
  return CALL_EX;
}
int setStep(int x)
{
  iSet.waveForm[iSet.selChan] = WA_STEP;
  setScrMode(iSet.selChan, WA_STEP, MAIN_MENU);
  cancelSweepBurst(); // reset active sweep/burst
  strcpy(but[FUNC_BUTTON].text, modeDefs[WA_STEP].name);
  valChange(VAL_CHGD);
  return CALL_EX;
}
int setPulse(int x)
{
  iSet.waveForm[iSet.selChan] = WA_PULSE;
  setScrMode(iSet.selChan, WA_PULSE, MAIN_MENU);
  cancelSweepBurst(); // reset active sweep/burst
  strcpy(but[FUNC_BUTTON].text, modeDefs[WA_PULSE].name);
  valChange(VAL_CHGD);
  return CALL_EX;
}

int setIdle(int x)
{
  // iSet.run[iSet.selChan] = false;
  dds[iSet.selChan].run = false;
  cancelSweepBurst(); // reset active sweep/burst
  valChange(VAL_CHGD);
  return CALL_EX;
}
int selA(int)
{
  iSet.selChan = CHAN_A;
  strcpy(but[FUNC_BUTTON].text, modeDefs[iSet.waveForm[CHAN_A]].name);
  //Asel = true; // done in setScrMode()
  //Bsel = !Asel;
  updateVal_Scrn = true;
  setScrMode(CHAN_A, iSet.waveForm[CHAN_A], MAIN_MENU);
  return CALL_NOEX; 
}
int selB(int)
{
  iSet.selChan = CHAN_B;
  strcpy(but[FUNC_BUTTON].text, modeDefs[iSet.waveForm[CHAN_B]].name);
  //Asel = false;
  //Bsel = !Asel;
  updateVal_Scrn = true;
  setScrMode(CHAN_B, iSet.waveForm[CHAN_B], MAIN_MENU);
  return CALL_NOEX; 
}

int setSweepMenu(int)
{
   //{SW_TYPE, SW_FIN, SW_TIM, SW_LOG, SW_LOG, SW_HO, SW_REP};
   DDSwaves w = iSet.waveForm[CHAN_A]; 
   // sweep only allowed for some waveforms
   if(w != WA_SINE && w != WA_SQR && w != WA_TRI && w != WA_PULSE && w != WA_STEP)
   {
      currentMenu = MAIN_MENU;
      return CALL_NOEX;
   }

  burst.run = false;
  updateVal_Scrn = true;//redrawScreen();
  currentMenu = SWEEP_MENU;
  //iSet.modeA = MO_NOR; // reset active sweep
  highButton = -1;
  setScrMode(CHAN_A, iSet.waveForm[CHAN_A], SWEEP_MENU);
  //int type = modeVal[CHAN_A][WA_SWEEP][SW_TYPE];
  int type =  sweepVal[iSet.waveForm[CHAN_A]].val[0][0] ;
  //Serial.printf("Sweep menu: Type %i\n", type);
  setSweepTypeInfo(type, -1); // after changing the setVals pointers to Sweep 
  return CALL_NOEX; 
}

// Sweep on change
// Sweep vals are drrectly edited in sweepVal[]
// units and limits are updated from sweepLim[]
// called from updateVal()
// lastVal < 0 to block previous value save
void setSweepTypeInfo(int newVal, int lastVal) // SetVals[0] for Sweep.
{ 
  float low, high;
  int wave = iSet.waveForm[CHAN_A]; // sweep is always CHAN_A
  //int xxx = modeVal[0][WA_SWEEP][SW_TYPE]; // == newVal
  //Serial.printf("SWP menu - from %i to %i: ", lastVal, newVal);
  if (newVal >= sweepLim[wave].options)
  {
    Serial.printf("Sweep mode outside range %i\n", sweepLim[wave].options);
    newVal = sweepVal[iSet.waveForm[CHAN_A]].val[0][0] = sweepLim[wave].options - 1;
    //newVal = modeVal[CHAN_A][WA_SWEEP][SW_TYPE] = sweepLim[wave].options - 1;
    updateVal_Scrn = true;
  }
  // save the previous values - always float, always in 2nd and 3rd place
  /* // now direct save
  if(lastVal >= 0) // only save if previously edited
  {
    sweepVal[wave].low[lastVal]  = *(float*)setVals[1].vp;
    sweepVal[wave].high[lastVal] = *(float*)setVals[2].vp;
    Serial.printf("Saving: %3.1f, %3.1f, to",  sweepVal[wave].low[lastVal], sweepVal[wave].high[lastVal]);
  }
  */ 
  // update value pointers - direct edit of values in sweepVal[]
  setVals[0].vp = (void*)&sweepVal[iSet.waveForm[CHAN_A]].val[0][0]; // type
  setVals[1].vp = (void*)&sweepVal[wave].val[newVal][SW_INI]; 
  setVals[2].vp = (void*)&sweepVal[wave].val[newVal][SW_FIN];
  setVals[3].vp = (void*)&sweepVal[wave].val[newVal][SW_TIM]; 
  setVals[4].vp = (void*)&sweepVal[wave].val[newVal][SW_STE];
  setVals[5].vp = (void*)&sweepVal[wave].val[newVal][SW_LOG]; 
  setVals[6].vp = (void*)&sweepVal[wave].val[newVal][SW_REP];

  // set units and limits
  strcpy(setVals[1].units, sweepLim[wave].units[newVal]);
  strcpy(setVals[2].units, sweepLim[wave].units[newVal]);
  low  =  setVals[1].minVal = setVals[2].minVal  = sweepLim[wave].min[newVal]; 
  high =  setVals[1].maxVal = setVals[2].maxVal = sweepLim[wave].max[newVal];
  
  //Serial.printf(" new vals %3.1f [%3.1f], %3.1f  [%3.1f]; lims %3.1f, %3.1f\n", sweepVal[wave].val[newVal][SW_INI], *(float*)setVals[1].vp, sweepVal[wave].val[newVal][SW_FIN], *(float*)setVals[2].vp,low, high);

  updateVal_Scrn = true;
  //Serial.println("Changed limits and values");
  redrawScreen();
}

// Sweep vals are directly edited in sweepVal[]
// units and limits are updated from sweepLim[]
void setSweepVals(void)
{
  burst.run = false;
  int func = iSet.waveForm[CHAN_A];
  int type;
  //{SW_TYPE, SW_INI, SW_FIN, SW_TIM, SW_LOG, SW_LOG, SW_REP}; 
  type = sweep.mode = (sweepMode_t) ((int)sweepVal[iSet.waveForm[CHAN_A]].val[0][0]); //(sweepMode_t)sweepVal[func].type;   
  sweep.start   = sweepLim[func].min[type];
  sweep.end    = sweepLim[func].max[type];
  sweep.time   = sweepVal[func].val[type][SW_TIM];
  sweep.steps  = sweepVal[func].val[type][SW_STE];
 // sweep.hold   = modeVal[0][WA_SWEEP][SW_HO]; // deprecated
  sweep.log = sweepVal[func].val[type][SW_LOG];
  sweep.repeat = sweepVal[func].val[type][SW_REP];
  if(swCont.run)
  { 
    iSet.modeA = MO_SWEEP;    
    startSweep(99);
    //startSweep_DDS = true;
  }
  else
    stopSweep(99);
    //iSet.modeA = MO_NOR;    
}

int toggleSweep(int)
{
 // Serial.println("T Sweep");
  swCont.run = !swCont.run;
  setSweepVals();  
  valChange(VAL_CHGD);
  updateVal_Scrn = true;
  return CALL_NOEX; 
}


int sweepEx(int)
{
  int type = modeVal[0][WA_SWEEP][SW_TYPE];
  setSweepVals();  
  setSweepTypeInfo(type, type); // save values
  currentMenu = MAIN_MENU;
  updateVal_Scrn = true;
  highButton = -1;
 // holdScreenUntil = 0;
  setScrMode(CHAN_A, iSet.waveForm[CHAN_A], MAIN_MENU);
  return CALL_NOEX; 
}
// generic submenu exit. Kills burst and sweep modes.
int subEx(int)
{
  currentMenu = MAIN_MENU;
  iSet.modeA = MO_NOR;
  sweep.run = false;
  burst.run = false;
  updateVal_Scrn = true;
  highButton = -1;
  //holdScreenUntil = 0;
  setScrMode(iSet.selChan, iSet.waveForm[iSet.selChan], MAIN_MENU);
  return CALL_NOEX; 
}

void cancelSweepBurst(void)
{
   swCont.run = false;
   burst.run = false;
   //burst.run = false;
   iSet.modeA = MO_NOR;
   //setScrMode(CHAN_A, iSet.waveForm[CHAN_A], MAIN_MENU);
}
int setBurstMenu(int)
{
  // all modes have a burst capability - time or cycles.
  //Serial.println("Burst menu");
  updateVal_Scrn = true;//redrawScreen();
  currentMenu = BURST_MENU;
  swCont.run = false;
  highButton = -1;
  setScrMode(CHAN_A, iSet.waveForm[CHAN_A], BURST_MENU);
  return CALL_NOEX; // cotinue touch processing
}

int setFuncMenu(int)
{
  //Serial.println("Func menu");  
  currentMenu = FUNC_MENU;
  highButton = -1;
  setScrMode(iSet.selChan, iSet.waveForm[iSet.selChan], FUNC_MENU);
  //updateVal_Scrn = true; //
  //redrawScreen();
  valChange(VAL_CHGD_LOCAL);
  return CALL_NOEX; // cotinue touch processing
}

// load burst values and exit
int burstEx(int)
{ 
  setBurst(1);
  currentMenu = MAIN_MENU;
  updateVal_Scrn = true;
 // holdScreenUntil = 0;
  setScrMode(CHAN_A, iSet.waveForm[CHAN_A], MAIN_MENU);
  //  if (iSet.modeA == MO_BURST)  setBurst(1);
  return CALL_NOEX; 
}

int setContMenu(int)
{
  //Serial.println("Control menu");
  setScrMode(CHAN_A, iSet.waveForm[CHAN_A], CONT_MENU);
  updateVal_Scrn = true;//redrawScreen();
  currentMenu = CONT_MENU;
  return CALL_EX;
}

int setSetMenu(int)
{
  vCalTmp[0] = CALSET;// reset screen Cal values
  vCalTmp[1] = CALSET;  
  return CALL_EX;
}

int setComMenu(int) // also called by cycleLAN() and updateVal()
{
  setVals[SET_COM+1].vp = (void*)&comms.STA_ssid[editLAN-1][0];
  setVals[SET_COM+2].vp = (void*)&comms.STA_pass[editLAN-1][0];
  //Serial.printf("Copy in [%s] [%s], editLAN %i\n", (char*)setVals[SET_COM+1].vp, (char*)setVals[SET_COM+2].vp, editLAN);
  //Serial.printf("AC %i, name [%s]\n",wAutoConn, comms.instName);
  //Serial.printf("IP [%s], host [%s]\n",IPstring, myHostName);
  return CALL_EX;
}
int setEx(int)
{
  //strcpy(comms.STA_ssid[editLAN-1], ssidTemp);
  //strcpy(comms.STA_pass[editLAN-1], passTemp);
  return CALL_EX;
}
int setNor(int)
{
  //Serial.println("set Nor");
  selA(1);
  setMode(MO_NOR);
  //setScrMode(CHAN_A, iSet.waveForm[CHAN_A], MAIN_MENU);
  //redrawScreen();
  currentMenu = MAIN_MENU;
  updateVal_Scrn = true;
  return CALL_EX;
}

/*
void printSV(int item)
{
  float val;
  Serial.printf("SetVal %i: ", item);
  switch(setVals[item].fmt)
  {
    case 'F':
    case 'R':
    case 'V':
      val = *(float*)setVals[item].vp;
      break;
      
    default: // ignore other formats
    val = 0;
  }
  Serial.printf(" val %3.2f, fmt %c, min %3.2f, max %3.2f\n", val, setVals[item].fmt, setVals[item].minVal, setVals[item].maxVal);
}
*/
#endif