/* Encoder and panel switches
*/
#ifndef MYSWITCHES_H
#define MYSWITCHES_H
void setOnOff(int8_t channel, bool status);
// ** Tac switches and encoder **//

#include "quadrature.h"
Quadrature_encoder<ENC_B, ENC_A> enc = Quadrature_encoder<ENC_B, ENC_A>();

#include "Button2.h"
Button2 butSW, butR,  butL, butA, butB;

//uint16_t oldHigh = -1;
short power = 0;
short currentDigit = 4;

long newPosition = 0, oldPosition = 0;    // no action first time around

inline long getEncPos(void)
{
	return enc.count();
}
#ifndef ALTENCODER
void rotate()
{
  newPosition = enc.count();
  //valChange(VAL_CHGD_LOCAL_NOUPD) ;
  Serial.print(" -ER- ");
}
#endif

// fixed this to use the setvals pre and post numbers
void clickedL(Button2& thisbut) { // up
  //Serial.println("Clicked L button");
//return;
  if (highButton < 0 || highButton >= NUMBUTS) // nothing selected
    return;
  int svx = but[highButton].sv;
  if (butDigit < (setVals[svx].pre - 1)) 
     butDigit++;
  else    // fix position if new setvalue is selected
    butDigit = setVals[svx].pre - 1;
  //Serial.printf("ButL = %i\n",butDigit);
  digSwPressed = true;
  butDir = 1;
  //valChange(VAL_CHGD_LOCAL);
}

// out of butDigit range also checked/fixed in displaySettings()
void clickedR(Button2& thisbut) {  //down  
  //Serial.println("Clicked R button");
//return;
  if (highButton < 0 || highButton >= NUMBUTS)
     return;
     
  int svx = but[highButton].sv; 
  if (butDigit > (-setVals[svx].post))
     butDigit--;
  else
     butDigit = -setVals[svx].post;
  //Serial.printf("ButR = %i\n",butDigit);
  digSwPressed = true;
  butDir = -1;
 // valChange(VAL_CHGD_LOCAL);
}

// start or stop sweep/burst
void clickedEnc(Button2& thisbut) 
{
 // Serial.println("Clicked ENC button");
  if(iSet.modeA == MO_BURST)
     if(burst.run)
      stopBurst(99);
     else 
      startBurst(99);

  if(iSet.modeA == MO_SWEEP)
    if(swCont.run)
      stopSweep(99);
    else       
      startSweep(99);  
  valChange(VAL_CHGD_LOCAL);
} 
// toggle 
void clickedA(Button2& thisbut) 
{   
  swPressed = true;
  fifoCmd = fifoCmd | (dds[CHAN_A].run) ? STOP_A : START_A;
  //Serial.printf("Clicked A button is %i : cmd 0x%x\n", dds[CHAN_A].run,fifoCmd);
  valChange(VAL_CHGD_LOCAL);
}

// toggle 
void clickedB(Button2& thisbut) 
{ 
  Serial.println("Clicked B button");
  swPressed = true;
  fifoCmd = fifoCmd | (dds[CHAN_B].run) ? STOP_B : START_B;
  valChange(VAL_CHGD_LOCAL);
}

void startEncBut(void)
{ 
  //Serial.println("start Enc But");
#if HARD_VERSION > 4
  pinMode(ENC_A, INPUT); // external pullups on V5
  pinMode(ENC_B, INPUT);
#else
  pinMode(ENC_A, INPUT_PULLUP); 
  pinMode(ENC_B, INPUT_PULLUP);
#endif
  pinMode(ENC_SW, INPUT_PULLUP);
  pinMode(L_BUT, INPUT_PULLUP);
  pinMode(R_BUT, INPUT_PULLUP);
  pinMode(A_BUT, INPUT_PULLUP);
  pinMode(B_BUT, INPUT_PULLUP);

  pinMode(TRIG_INPIN, INPUT_PULLDOWN); // changed by CODE
  lastTrigInPol = 0;

//Serial.print(" A ");
   enc.begin(pull_direction::up, resolution::quarter);
//Serial.print(" B ");
// setClickHandler doesn't seem to work - debounce???
  butSW.begin(ENC_SW);
  butSW.setTapHandler(clickedEnc);

  butL.begin(L_BUT);
  butL.setTapHandler(clickedL);
  
  butR.begin(R_BUT);
  butR.setTapHandler(clickedR);
  
  butA.begin(A_BUT);
  butA.setTapHandler(clickedA);
  
  butB.begin(B_BUT);
  butB.setTapHandler(clickedB);
 
 // Serial.println("Switch Enc done");
}

#define DEBOUNCE_OO  5
uint8_t onSwitch, offSwitch;
int oo_samestate = 0;
// channel  < 0 == both
void setOnOff(int8_t channel, bool status)
{
  //Serial.printf("SetOnOff ch %i, st %i\n", channel, status);
  if(channel <= 0)  // chan A
  {
    if(status) 
      setFunc(0, AC_NORMAL, true);
    dds[0].run = status; 
    digitalWrite(LEDA, status);
  }

  if(channel < 0 || channel == CHAN_B) // CHAN_B
  {
    //Serial.printf("OnOff B %i\n", status);
    if(status) 
      setFunc(1, AC_NORMAL, true);
    dds[1].run = status; 
    digitalWrite(LEDB, status);
  }
}

//bool lastTrigState = false; // may need to vary this to stop initial false triggers when CP_ET is enabled.
// trigInActive reset by burst or sweep in main() code
void extTrig(void)
{
  // ext trigger only for Sweep and Burst 
  if(iSet.modeA == MO_NOR || !TRIG_EN || trigInActive)
  {
    //lastTrigEnabled = false;
    return;
  }
  int pol = modeVal[CHAN_A][WA_CONT][CP_INPOL];
  if(lastTrigInPol != pol) // pull pin in direction to stop false triggers
  {
    //Serial.printf("Ext Trig change: pol %i\n", pol);
    pinMode(TRIG_INPIN, (pol) ? INPUT_PULLDOWN: INPUT_PULLUP); // pull down for pos trigger
    lastTrigInPol = pol;
  }

#ifdef TRIGTEST
  int cs = getSerialChar();
  if (cs == -1) // no char avail
    return;
#else
  bool cs = digitalRead(TRIG_INPIN);
#endif

  //lastTrigState = cs;  
  if(cs != TRIG_IN_POL)
    return;
  trigInActive = true; // prevent re-triggering
  //Serial.printf("TRig Y %i\n", cs);
  if(iSet.modeA == MO_BURST)
    fifoCmd = fifoCmd | START_BURST;
  if(iSet.modeA == MO_SWEEP)
    fifoCmd = fifoCmd | START_SWEEP;
}

volatile bool LEDstate = false;
bool inSingle = false;
// see defs.h for struct blink 
bool timer_callback(repeating_timer_t *rt) 
{
  if(bl.single || bl.sCount > 0) // single mode sequence
  {
    //strcpy(xxbuf, " S0 ");
    if(bl.single && bl.sCount > 0) // both single shot parameters have been loaded. First interation only
    {
      //strcpy(xxbuf, " S2 ");
      LEDstate = !LEDstate; // invert current LED mode
      digitalWrite(LEDW, LEDstate);
      bl.single = false;    
    }
    else  // subsequent iterations
    {
      if(--bl.sCount == 0)
      {
       // strcpy(xxbuf, " S2 ");;
        LEDstate = !LEDstate; // restore LED mode
      }
    }
  }
  else // standard mode
    if(--bl.counter <= 0)
    {  
      bl.counter = bl.t[!LEDstate];
      if(bl.counter == 0) // zero time for this state
        bl.counter = bl.t[LEDstate];
      else   
        LEDstate = !LEDstate;
      digitalWrite(LEDW, LEDstate);
    }
  return true; // restart
}
#endif
