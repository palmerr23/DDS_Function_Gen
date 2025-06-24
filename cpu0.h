/* CPU 0
  Screen
  WiFi
  Housekeeping
*/
#define ASYNC_WS

repeating_timer_t timer;

#include "screenDraw.h"
#include "switches.h"
#include <ArduinoJson.h>
#include "myWiFi.h"
#include "HTML_literal.h"
#include "HTML_processor.h"
#ifdef ASYNC_WS
  #include "HTML_literal.h"
  #include "HTML_processor.h"
  #include "asyncWebserver.h"
#else
  #include "myWebserver.h"
#endif
#include "touchProcess.h"
#include "udp_scpi.h"
long  lTimer, mTimer, fTimer, vlTimer, timeReport = 0;
int vfCounter;
#include "profile.h"
void setScrMode(uint8_t chan, DDSwaves funcMode, int);
int constrainButDig(int butDig, int hBut);
void printButs(int firstFew);
void printFifo(uint32_t f);
bool netBegin(void);

long tim1;

// Wifi must run on CPU0
void setup()
{   
  Serial.begin(115200);
  while(!Serial && millis() < SERIAL_TIMEOUT)
    delay(10);
  Serial.println("\n\n--- DDS Function Generator ---");
  Serial.printf("Board version %i\n", BOARDVER);
  Serial.printf("EEPROM (major) version %i\n", EE_VERSION);
  Serial.printf("Software (minor) version %i\n", SOFT_VERSION);   
 // Serial.printf("sRate %i, %i bits\n", sampleRate, SAMPWIDTH);
 // Serial.printf("Phase bits = %i + %i, masks: Ph %08X, Int %08X,\n",DDS_LOOKUP_BITS, DDS_INTER_BITS, DDS_PHASE_MASK, DDS_INTER_MASK);
 // Serial.printf("Volts to counts +1 %i, -1 %i\n",voltsToCounts_Pulse(0,1.0), voltsToCounts_Pulse(1,-1.0));
  //int32_t sysClk_Hz =  clock_get_hz(clk_sys);
  //Serial.printf("Clock %i\n",sysClk_Hz);


  while(!cpu0ready) // wait for CPU1 to finish critical setup tasks before proceeding
    delay(10);

  // negative timeout means exact delay (rather than delay between callbacks)
  // see struct bl for initial settings
  if (!add_repeating_timer_ms(-100, timer_callback, NULL, &timer)) 
  {
      Serial.printf("Failed to add timer\n");        
  }

#ifdef TFT_ON
  Serial.println("Starting TFT");
    highButton = -1;
    currentMenu = 0;
    screenBegin(); 
    //tftDebugChar('0'); //delay(500);
    //updateVal_Scrn = true;    
    #ifdef SPLASH
      splashScreen();
      delay(5000); // blocking
    #endif
#else
  Serial.println("Not starting TFT");
#endif

  int EEstat = eeBegin();
  switch (eeBegin())
  {
    case -1 :      
      screenError("No EEPROM found", ERR_BG_ERR ,5, false);  // don't hold - TFT may not be installed
      comms.enabled = true; // no EEPROM - probably programming offline
      delay(5000);  // make the error call block processing 
      break;
    case -2 :         
      screenError("Factory reset failed", ERR_BG_ERR ,5, false);  // don't hold - TFT may not be installed
      delay(5000);  // make the error call block processing 
      break;
    default :
      Serial.println("EEPROM found");  
  }

#ifdef TFT_ON
   setOSKtouchCal(); // values were updated from EE
   setScrMode(iSet.selChan, iSet.waveForm[iSet.selChan], MAIN_MENU); 
#endif
  //rp2040.fifo.push_nb(FUNC_CHANGE_A | FUNC_CHANGE_B | STOP_A | STOP_B); // set up the two channels as read from EEPROM

  bool fsFound = FILESYS.begin(); // auto format
  if(!fsFound) 
  {    
    screenError("Missing file system", ERR_BG_ERR ,5, false); 
    delay(5000); // make the error call block processing 
  } 
  else
    if(!FILESYS.exists("/favicon.ico")) // a file that won't possibly be gzipped
    {      
      screenError("Missing web files", ERR_BG_ERR ,5, false); 
      delay(5000); 
    }        

#ifdef WIFI_ON
   if(comms.enabled)
   {      
    //Serial.println("Starting WiFi");
      netBegin(); // WiFi, Server, UDP
   }   
#else
   Serial.println("Not starting WiFi");
#endif
  SCPIstart(); // SCPI is also serial
  startEncBut();
  delay(10);
  swPressed = false;
  updateVal_DDS = false;
  holdWait = false;
 // Serial.printf("vCal %1.4f, %1.4f\n", vCal[0], vCal[1]);
  fifoCmd = STOP_A | STOP_B | FUNC_CHANGE_A | FUNC_CHANGE_B;
//sineScaleTest(5000, 70000, 40);
  /*
 // Serial.printf("Lum BBXHIGH 0X%04x %i, HIGHCOL 0X%04x %i, BBMHIGH 0X%04x %i, BRK %i\n", BBXHIGH, lum(BBXHIGH),  BHIGHCOL, lum(BHIGHCOL),  BBMHIGH, lum(BBMHIGH), CLRBRK);
Serial.printf("LumV BBXHIGH 0X%04x %i, HIGHCOL 0X%04x %i, BBMHIGH 0X%04x %i, BRK %i\n", BBXHIGH, lumVis(BBXHIGH),  BHIGHCOL, lumVis(BHIGHCOL),  BBMHIGH, lumVis(BBMHIGH), CLRBRKV);
Serial.printf("LumV WHITE 0X%04x %i, BLACK 0X%04x %i, BBXHIGH 0X%04x %i, BRK %i\n", ILI9488_WHITE, lumVis(ILI9488_WHITE),  ILI9488_BLACK, lumVis(ILI9488_BLACK),  BBXHIGH, lumVis(BBXHIGH), CLRBRKV);
  */
  updateVal_Scrn = true; // REDRAW AFTER ANY MESSAGES EXPIRE
  if(strlen(postMsg) == 0)
  {
    strcpy(postMsg, "POST: No errors");
    Serial.println(postMsg);
  }
  Serial.println("--- Startup complete ---");
}

//int count1;
// offset values so that servicing times don't always align
#define PROCESS_EVERY_VF    5     //  every X loops
#define PROCESS_EVERY_F     5     //   mS
#define PROCESS_EVERY_CTRL 11 //  10 mS 
#define PROCESS_EVERY_M    103    //  100mS -   screen touch and update
#define PROCESS_EVERY_L    753   // 750 mS - display and readings; fan
#define PROCESS_EVERY_VL   30003  // 30 S  - EE save
#define PROCESS_EVERY_T    3007         // for tesing/debug only
#define REPORTEVERY 10007 // debug

uint16_t x, y, z;
uint8_t rotation = 0;
bool debugXXX = true;

void loop()
{
    bool fifoOK;
    butSW.loop();
    butA.loop();
    butB.loop();
    butL.loop();
    butR.loop();        
    yield();
      
   if(++vfCounter > PROCESS_EVERY_VF)
   {
    vfCounter = 0;
    extTrig();    
   }
  // start/stop
  // sweeps (bursts and cycles in CPU0 code)
    if((millis() - fTimer) > PROCESS_EVERY_F)
    {   
      fTimer = millis();  
      SCPIexec();
   
    }
    if((millis() - mTimer) > PROCESS_EVERY_M)
    {  
      mTimer = millis(); 
      _scrTouched = processTouchSwEnc(false); 
      if(_scrTouched) // release screen hold on touch
      {
        holdScreenUntil = 0;
        holdWait = false;
      }
      newPosition = enc.count();
  
    }
    //yield();
    // lower priority  - screen redraw
    if(millis() > lTimer + PROCESS_EVERY_L)
    {
      lTimer = millis();  

      if(highButton >= 0 && highButton < NUMBUTS) // ensure highlighted set digit is always visible
      {
        //int sv =  but[highButton].sv;  
        butDigit = constrainButDig(butDigit, highButton); 
        //Serial.printf("BD[%i] = %i [%i..%i]\n", highButton, butDigit, (1 - setVals[sv].post),(setVals[sv].post -1) );
      } 

      if(updateVal_Scrn)
      {
        //Serial.printf("RDS: Wave %i, selected %i\n", iSet.waveForm[0], iSet.selChan);
        if(!HOLDSCREEN)
          redrawScreen();        
      }
      
      if(currentMenu == 0)
      {
        if(messageExpires > millis())
        {                  
          if(strlen(messageLine))
          {
            tft.setFreeFont(&FONT0);
            tft.setTextColor(MSGCOL, BGCOL); 
            if(strlen(messageLine) > MAXMSG) // long messages span 2 lines.
            {
              char msg[MAXMSG+1];
              strncpy(msg, messageLine, MAXMSG);
              msg[MAXMSG] = '\0';
              tft.drawString(msg, 2, MSGLINE-MSG_HT, GFXFF);
              // truncate extra long lines
              strncpy(msg, &messageLine[MAXMSG],MAXMSG);
              tft.drawString(msg, 2, MSGLINE, GFXFF);
            }
            else
              tft.drawString(messageLine, 2, MSGLINE, GFXFF);//tft.print(".");
          }
        }
        else
        {
          if(strlen(messageLine))
          {
            //Serial.println("Msg Expired");
            tft.fillRect(0, MSGLINE - 2 * MSG_HT, HMAX, 2 * MSG_HT, BGCOL);
            messageLine[0] = '\0';
          }
        }  
      }
    }

  #ifdef WIFI_ON
    #ifndef ASYNC_WS
      loopWiFi();
    #endif
  #endif 
  //yield();
  if(ext_trig && iSet.modeA != MO_NOR)
  {
    fifoCmd = fifoCmd | (iSet.modeA == MO_SWEEP) ? START_SWEEP : START_BURST;
    valChange(VAL_CHGD_LOCAL);
    ext_trig = false;
  }

  if(startSweep_DDS && iSet.modeA == MO_SWEEP)
  {
    fifoCmd = fifoCmd | START_SWEEP;
    valChange(VAL_CHGD_LOCAL);
    startSweep_DDS = false;
  }

  // catch B=A T->F transition and stop B (to ensure unwanted signal not generated)
  if (lastBCOUPLE && !BCOUPLE)
  {
    fifoCmd = fifoCmd | STOP_B;
    valChange(VAL_CHGD_LOCAL);
    //Serial.println("BC T->F");
  }
  lastBCOUPLE = BCOUPLE;

  if(fifoCmd)
  {    
     //fifoCmd = (iSet.selChan == CHAN_A) ? VAL_CHANGE_A : VAL_CHANGE_B;
     //Serial.printf("REP: min %i, max %i, cur %i\n", pulse[0].minVal, pulse[0].maxVal, dds[0].curVal );
     // if(fifoCmd) updateVal_Scrn = true;
     //mute(true); // this might take a while, interrupting synthesis
     fifoOK = rp2040.fifo.push_nb(fifoCmd); // flag other processor to reload params
     /*
     if(fifoCmd == FUNC_CHANGE_A || fifoCmd == FUNC_CHANGE_B)
     {
      Serial.println("FIFO func change");      
     }
     */
     if(!fifoOK)
      Serial.print("Can't Send FIFO flag \n");
     // set save to EEPROM flag...
     updateVal_DDS = false;
     //mute(false);
     //printFifo(fifoCmd);
     fifoCmd = 0;
  }

  if((millis() - vlTimer) > PROCESS_EVERY_VL) 
  {
    vlTimer = millis();
    if(updateVal_EE)
    {
      if(!HOLDSCREEN)
      { 
        if(WifiConnected)
        {
          bl.t[0] = 0;
          bl.t[1] = 10;
        }
        else
        {
          bl.t[0] = 10;
          bl.t[1] = 0;
        }
        bl.sCount = 30; // invert for 3 seconds
        bl.single = true;
        Serial.print(" ~EE~ ");
        writeEE();
        updateVal_EE = false;
      }
      if(factoryReset)
      { 
        screenError("Factory reset\ncomplete\nRestarting in\n5 seconds.", ERR_BG_MSG ,5, false);  // hold until screen touch.
        //.println("Factory reset\ncomplete. Restarting\n");
        delay(5000); // wait for EEPROM write to complete
        rp2040.reboot();
      }
    }
  }

  if((millis() - timeReport) > REPORTEVERY) // debug
  {
   // Serial.printf("Free heap: %i\n",rp2040.getFreeHeap());
   //   Serial.printf("vCal %1.4f, %1.4f\n", vCal[0], vCal[1]);
   // Serial.print("Last SCPI error: "); getLastSCPIerror(Serial);
    //Serial.printf("***Idle %i : %i, maxVal %i, maxDAC %i, maxPulse %i\n",idleVal[0], sinval[0].dcOff, maxVal,maxDAC,maxPulseVal);
    //Serial.printf ("DDSMode 0: %i, 1: %i\n",iSet.waveForm[0],iSet.waveForm[1]);// iSet.run// iSet.run
   // Serial.printf ("DDSMode 0:%i[%i], 1:%i[%i], modeA %i, ", iSet.waveForm[0], iSet.run[0], iSet.waveForm[1], iSet.run[1], iSet.modeA);
   // Serial.printf ("SelCh %i, SWon %i, BUon %i, Run %i | %i, coupled %i\n", iSet.selChan, swCont.run, burst.run, dds[0].run, dds[1].run, BCOUPLE);
    //Serial.printf ("Enc %i\n", enc.count());
    //Serial.printf ("TR step %i, del %i, steps %i\n", stepSizQR, stepDelQR, sampsQR);
    //Serial.printf ("TF step %i, del %i, steps %i\n", stepSizQF, stepDelQF, sampsQF);
//printPulses();
//Serial.printf("Last-BCOUPLE %i, B=A %1.3f, BCPL %i\n", lastBCOUPLE, modeVal[0][WA_CONT][CP_BA], BCOUPLE);
    //printSV(1);
    //Serial.printf("BU: BaltA %3.2f [%i], state %i\n", modeVal[CHAN_A][WA_BURST][BU_BALTA], BALTA, burst.state);
   Serial.printf("Blink %i %i\n", bl.t[0], bl.t[1]);
    //printProfile();
    timeReport = millis();
  }

  // yield();
#ifndef ASYNC_WS
  server.handleClient();
#endif
 MDNS.update();
 //yield();
}

void printSets(int num)
{

}
// keep the digit being changed for the selected setting within bounds.
int constrainButDig(int butDig, int hBut)
{
  int svx = but[hBut].sv; 
  int post = setVals[svx].post;
  int pre = setVals[svx].pre;
  // assume  post and pre are not *both* zero
  if(post == 0) // integer
    butDig = constrain(butDig, 0, pre - 1);
  else
    butDig = constrain(butDig, -post, pre - 1);
  if(pre == 0) // no XXX. part
    butDig = constrain(butDig, -post, -1);
  return butDig;
}
void printFifo(uint32_t f)
{
  String q;
  if (f & VAL_CHANGE_A)
   q += "VAL_CHANGE_A ";
  if (f & VAL_CHANGE_B)
   q += "VAL_CHANGE_B ";

  if (f & FUNC_CHANGE_A)
   q += "FUNC_CHANGE_A ";
  if (f & FUNC_CHANGE_B)
   q += "FUNC_CHANGE_B ";  

  if (f & START_A)
   q += "START_A ";   
  if (f & START_B)
   q += "START_B ";   

  if (f & STOP_A)
   q += "STOP_A ";   
  if (f & STOP_B)
   q += "STOP_B ";     

  if (f & START_SWEEP)
   q += "START_SWEEP ";   
  if (f & STOP_SWEEP)
   q += "STOP_SWEEP ";   
  if (f & CHANGE_SWEEP)
   q += "CHANGE_SWEEP ";  

  if (f & START_BURST)
   q += "START_BURST ";   
  if (f & STOP_BURST)
   q += "STOP_BURST ";  

  if (f & CHANGE_BURST)
   q += "CHANGE_BURST ";   
  if (f & CHANGE_CONTROL)
   q += "CHANGE_CONTROL ";  

  Serial.println(q);
}