/*
  I2S DDS - Pico W
  Jan 2024
  Overclock to 240 MHZ for 2 sine DDS (e.g. IMD)
  Compile with -Ofast??
  CPU0
    WiFi, TFT, etc
    No direct manipulation of DDS *run* variables or states - sends change flags via interprocessor queue
   
  CPU1 
    DDS only
    Avoid print or other calls in CPU1 code that will compile with 'thread safe' code (delays will corrupt sample generation)

  INT32 SAMPLE SCALING (takes 1/20 of float multiply when shifted right after int mul)
  20 bit sample, 12 bit level

 *** NO CALLS TO PRINT IN THIS CPU1 CODE (causes breaks in generation) ***

*/
#define MODEL "DDS01"
#define SOFT_VERSION 1
#define HARD_VERSION 5  // version >=3 board. Different pins
#define EE_VERSION   1

#define DDS_ON  // debug: enable DDS
#define WIFI_ON // debug: initialise wifi 
#define TFT_ON  // debug: initialise TFT 
#define PS_PWM 1  // Pico 3.3V SMPS noise may be less in PWM mode
#define REALNET // test SSID/Pass
//#define SWAP_LED_TW // test W led without hardware
//#define PROD
#ifdef PROD
  #define SPLASH
  #define WEB_GZIP  // send gzipped css and js files (PROD)
#else
   //#define SPLASH 
   //#define NO_SCREEN_ERRORS
  //#define TRIGTEST  // serial input instead of GPIO - breaks serial SCPI
  //#define DEBUG_SWEEP // trig changes at end of sweep, rather than per step

#endif
//#define SAMPWIDTH 16
#define SAMPWIDTH 24
// Overclock 200MHz for 192K samplerate
//int sampleRate = 96000;
int sampleRate = 192000;

#include <hardware/pio.h>
#include <pico/stdlib.h>
#include "I2S.h"  
#include "hardware/timer.h"
#include "hardware/irq.h"

#include "defs.h"
#include "helpers.h"
#include "colours.h" // requires GFXfont definition
#include "DDSdefs.h"
void setSine(int chan, dds_actions action, bool restart = true);
void setSqr(int chan, dds_actions action, bool restart = true);
void setTri(int chan, dds_actions action, bool restart = true);
void setPulse(int chan, dds_actions action, bool restart = true);
void setStep(int chan, dds_actions action, bool restart = true);
void setFunc(int chan, dds_actions action, bool restart = true);
void setOnOff(int8_t channel, bool status);

bool trigOutOn = true; // send trigger pulses?
bool bPhaseA = false;
int phOffset = 1024 * DDS_INTER_MUL;
int burstCyclesDone = 0;
bool burstable = false, sweepable = false;
int multiMode[CHANS] = {MULTI_OFF, MULTI_OFF};  // STEP, IMD and WHITE will disable MULTI.

// tone burst or sweep
//const bool burstMode = false;
//const bool sweepMode = false;
bool ZCflag = true; // Chan A upward crossing detected. Prime for burst mode
int crossingLevel[CHANS] = {0, 0}; // differs by MODE
int idleVal[CHANS];

#include "DDS.h"
#include "DDSset.h"

I2S i2s(OUTPUT);

//int count = 0;
//#define STIME 5000  // mS
//#define EXIT_TIME 20000
//#define REPORT 2000
//bool scan = false;
//bool running = false;
//float dev = 1.0;

#define SERIAL_TIMEOUT 10000
//int inc, incA, incB; //40 ~1kHz @ 96k sample
bool I2Srunning = false;
// CPU1: DDS generation ONLY
void setup1() 
{
//  rp2040.enableDoubleResetBootloader(); // reset to USB bootload on switch double tap

  pinMode(PSPIN, OUTPUT); // power save OFF to improve ripple and noise
  digitalWrite(PSPIN, PS_PWM); 
   
  pinMode(TRIG_OUTPIN, OUTPUT);
  digitalWrite(TRIG_OUTPIN, !TRIG_OUT_POL);

  pinMode(LEDA, OUTPUT);
  digitalWrite(LEDA, 0);

  pinMode(LEDB, OUTPUT);
  digitalWrite(LEDB, 0);

  pinMode(LEDW, OUTPUT);

#ifdef DDS_ON
  initDDSsine();
  i2s.setBuffers(DMABUFS, DMABUFLEN, offVal); // short buffering to reduce trigger lag. non-zero silence value to defeat PCM5102 auto-mute 
  i2s.setBCLK(pBCLK); 
  i2s.setDATA(pDOUT);
  i2s.setBitsPerSample(SAMPWIDTH);

  cpu0ready = true;  // can finish CPU0 setup() now

  if (!i2s.begin(sampleRate)) 
  {
    I2Srunning = false;
  }
  else
    I2Srunning = true;
#endif

  setFunc(0, AC_NORMAL, true); // set values and move to idle value, but don't start
  setFunc(1, AC_NORMAL, true);
  dds[0].run = dds[1].run = false; // startup is channels off.
  dds[0].state = dds[1].state = PU_IDLE;
  swCont.run = false;
  burst.run = false;
  setOnOff(-1, false);
 // running = true;
}

int i = 0, j = 0, k = 0;
long timerX = 0;
long counter = 0;
#define COUNTXX 1000000 // report every xx samples
int cycles = 0;
bool onOff = true;
float mul = 1.0;
uint32_t fifoVal = 0;
#define REPORT_EACH 6000
//bool sweepTrig = false;
long showCount = 0;

void loop1() 
{ 
  int ch;    
  #ifdef DDS_ON
  //if(show) if (showCount++ == 10) show = false;
  
  if(rp2040.fifo.available()) // CPU0 sends all function and mode changes via the inter-CPU fifo
  {
    doChanges();
  }

  for(ch = 0; ch < CHANS; ch++)
  {    
    if(ch == CHAN_A) // sweep & burst 
    {      
      // sweep start raises TRIG_OUTPIN
      if((iSet.modeA == MO_SWEEP) && swCont.run) //&& ZCflag
      {
        ZCflag = false;
        swCont.incCount++;     
        //if(swCont.incCount % 1000000 == 0)  Serial.printf("%i ", swCont.incCount);
        if(swCont.incCount >= swCont.incEvery) // next step
        { 
          //Serial.print(swCont.stepCount);
          swCont.incCount = 0;    
          swCont.stepCount++;  
          if(swCont.stepCount > 0)
            digitalWrite(TRIG_OUTPIN, !TRIG_OUT_POL);
                      // next sweep step
          if(!swCont.done)
          {
            //Serial.printf("SW-NXT %i ", swCont.stepCount);
            setFunc(0, AC_SWEEP_NEXT, false);
            //if(BCOUPLE) setFunc(1, AC_SWEEP_NEXT, false);              
          }
        
          if(swCont.stepCount >= sweep.steps || swCont.done) // done a complete sweep
          {
            //Serial.print("Swp comp ");
            swCont.stepCount = 0;
            if(sweep.repeat)
            {       
              //Serial.print("Swp-Restart ");
              setFunc(0, AC_SWEEP_START, false);
             // if(BCOUPLE) setFunc(1, AC_SWEEP_START, false);
              digitalWrite(TRIG_OUTPIN, TRIG_OUT_POL);
            }
            else
            {                  
              swCont.done = true;
              //Serial.print(" Swp done\n");            
            
             if(!sweep.repeat) // idle on completion?
              {
                swCont.run =  false;
                updateVal_web = true; // update the RUN/STOP indicator
                //Serial.print("-Stop-");
                setFunc(ch, AC_IDLE, false); 
                if(BCOUPLE)
                  setFunc(CHAN_B, AC_IDLE, false); 
                trigInActive = false;
              }  
            }
          }
        }        
      }
      
      // channel 0 only
      // ZCflag == finished a single waveform cycle
      if(iSet.modeA == MO_BURST && ZCflag && ch == CHAN_A && burst.run) // zero crossing occurred burst.run  
      {
        ZCflag = false;          
        burstCyclesDone++;
        if(burst.state == B_ACTIVE && burstCyclesDone >= burst.cyclesOn) // change state
        {
          burstCyclesDone = 0;
          burst.state = B_IDLE;          
          if(burstTrigState == true)
          {
            digitalWrite(TRIG_OUTPIN, TRIG_OUT_POL);
            burstTrigState = false;
          }          
        }
        else if(burst.state == B_IDLE && burstCyclesDone >= burst.cyclesOff)
        {
          burstCyclesDone = 0;
          burst.state = B_ACTIVE;          
          if(burstTrigState == false)
          {
            digitalWrite(TRIG_OUTPIN, !TRIG_OUT_POL);  // avoid overhead of using extTrig()
            burstTrigState = true;
          }          
          if(!burst.repeat)  
          {            
            stopBurst(99);
          }        
        }
      }           
    }

    if(ch == CHAN_B && dds[CHAN_A].waveForm == WA_IMD)
      DACval[CHAN_B] = 0;
    else
      switch (dds[ch].waveForm)
      {
        case WA_SINE :        
          DACval[ch] = DDSsine(ch);
          if(ch == 0 && BURST_MODE && burst.state == B_IDLE ) // keep the generator running in burst idle
            DACval[0] = idleVal[0];
          if(ch == 1 && BURST_MODE && BALTA && burst.state == B_ACTIVE) 
            DACval[1] = idleVal[1];
          break;
        case WA_SQR :
        case WA_PULSE :
        case WA_STEP :
        case WA_TRI :
          DACval[ch] = DDSpulse(ch); // keep the generator(s) running in burst idle
          if(ch == 0 && BURST_MODE && burst.state == B_IDLE) 
            DACval[0] = idleVal[0]; 
          if(ch == 1 && BURST_MODE && BALTA && burst.state == B_ACTIVE) 
            DACval[1] = idleVal[1];
          break;
        case WA_IMD :
          if(ch == 0)
          { 
            DACval[ch] = DDSintermod(); 
            if(burst.state == B_IDLE) // keep the generator running in burst idle
              DACval[ch] = idleVal[ch];
          }
          break;   
        case WA_WHITE :
          DACval[ch] = DDSwhite(ch);
          if(ch == 0 && burst.state == B_IDLE) // keep the generator running in burst idle
            DACval[ch] = idleVal[ch];
          if(ch == 1 && BURST_MODE && burst.state == B_ACTIVE)
             DACval[ch] = idleVal[ch];
          break;
        default : // silence
          DACval[ch] = offVal;
          break;
      }      
  } // for channels

  if((BCOUPLE && !BURST_MODE) && BPHASE == 0) // no drift phase alignment. Probably not needed
    DACval[1] = DACval[0];
 
  #if (SAMPWIDTH == 16)
    //DACval[0] = constrain(DACval[0], -maxDAC, maxDAC); // clip overshoots
    //DACval[1] = constrain(DACval[1], -maxDAC, maxDAC);
    i2s.write((int16_t)(DACval[0])); 
    i2s.write((int16_t)(DACval[1])); 
  #else
    // clipping pointless in 32 bit packed. Clip overshoots in lookup routines
    i2s.write((int32_t)(DACval[0])); 
    i2s.write((int32_t)(DACval[1]));
  #endif    
#else // DDS off
  delay(200);
#endif
} // end main


// can change values on the fly, but not functions or modes
// Sweep and burst have specific needs. Should have mode set first if compound change
// may be several settings OR'd in one command.
void doChanges(void)
{
    rp2040.fifo.pop_nb(&fifoVal);
    //Serial.printf("\nGot FIFO flag %03x\n", fifoVal);
    
    // stop first, start last
    if(fifoVal & STOP_A)
    { 
      setOnOff(0, false);
      if((BURST_MODE && BALTA) || (BCOUPLE && !BURST_MODE))
        setOnOff(1, false);
      //Serial.println("-DDS A stop-");
    }
    if(fifoVal & STOP_B)
    {      
      setOnOff(1, false);
      if((BURST_MODE && BALTA) || (BCOUPLE && !BURST_MODE))
        setOnOff(0, false);
      //Serial.println("-DDS B stop-");
    }

     if(fifoVal & STOP_BURST) // stop before start
    { 
      stopBurst(99); 
      //Serial.println("-DDS Burst stop-");
    }
    if(fifoVal & STOP_SWEEP)
    { 
      stopSweep(99); 
      //Serial.println("-DDS Sweep stop-");
    }

    if(fifoVal & FUNC_CHANGE_A) 
    { 
      setFunc(0, AC_NORMAL, true); // restart mode. 
      if((BURST_MODE && BALTA) || (!BURST_MODE && BCOUPLE)) // update channel B also
      {        
        iSet.waveForm[CHAN_B] = iSet.waveForm[CHAN_A];
        setFunc(1, AC_NORMAL, true); 
      }
      //Serial.println("-DDS func Change A-");
    }
    if(fifoVal & FUNC_CHANGE_B) 
    {      
      setFunc(1, AC_NORMAL, true); 
      //if(BALTA && BURST_MODE) // update channel B also
       // setFunc(1, AC_NORMAL, true); 
      //Serial.println("-DDS func Change B-");   
    }

    if(fifoVal & VAL_CHANGE_A)
    {
      //Serial.print("-VAL CHANGE_A- ");
      setFunc(0, AC_NORMAL, false); // keep going. AC_NORMAL not right for burst or sweeps????
    }
    if(fifoVal & VAL_CHANGE_B)
    {
      //Serial.print("-VAL CHANGE_B- ");
      setFunc(1, AC_NORMAL, false);
    }
    // start channels before sweep or burst
    if(fifoVal & START_A)
    {
      //setFunc(0, AC_NORMAL, true); // restart mode. 
      setOnOff(0, true);
      //Serial.println("-DDS A start-");
      if((BURST_MODE && BALTA) || (BCOUPLE && !BURST_MODE))
      {
        setOnOff(1, true);
        // Serial.println("-DDS A start B");
      }
    }
    if(fifoVal & START_B)
    {  
      //Serial.println("-DDS B start-");
      // (re)start A first
      if((BURST_MODE && BALTA) || (BCOUPLE && !BURST_MODE))
      {
        setFunc(1, AC_NORMAL, true); // Burst mode may reset values       
        setOnOff(0, true);
        //Serial.println("-DDS B start A");
      }       
      setOnOff(1, true);   
    }

    if(fifoVal & START_SWEEP)
    {
      //Serial.print("-SWEEP Start- ");
      
      startSweep(99);   
    }   
    if(fifoVal & START_BURST)
    {
      startBurst(99);
      //Serial.print("-BURST Start- ");       
    }   
    fifoVal = 0; 
}
void printState(void)
{
  //Serial.printf("State A: Run %i, state %i, wave %i\n",dds[0].run, dds[0].state, iSet.waveForm[0]);
}
#include "cpu0.h"