/* SCPI Commands */
/* UDP and Serial SCPI */
#ifndef _SCPICMD_H
#define _SCPICMD_H
#include <string.h>
void writeEE(void);
void printStoredWiFi(void);
bool netBegin(void);
void errMessage(const char * message, int16_t onTime);
#define SCPIDIGITS 3
#define ERRMSGLEN 256
char SCPIerrMsgBuf[ERRMSGLEN];
bool checkVal(float val, int wave, int indx)
{
  bool OK = true;
  char errMsg[ERRMSGLEN] = "";
  
  // min/max limits 
  if (val < modeDefs[wave].lowLim[indx] || val > modeDefs[wave].highLim[indx])
  {
    strcat(errMsg, "Value outside limits. ");
    OK = false;
  }
  // check for V2 <= V1 and (v +/- DCoff) within MAXDC limits
  switch (wave)
  {
    case WA_SINE: // DC offset + half amplitude too large
      if(indx == SN_AM)
        if((modeVal[iSet.selChan][wave][SN_DC] + val/2) > MAXDC || (modeVal[iSet.selChan][wave][SN_DC] - val/2) < MINDC)
        {
          strcat(errMsg, "Amp + Offset too big. ");
          OK = false;
        }
      if(indx == SN_DC)
        if((modeVal[iSet.selChan][wave][SN_AM]/2 + val) > MAXDC || (modeVal[iSet.selChan][wave][SN_AM]/2 - val) < MINDC)
        {
          strcat(errMsg, "Amp + Offset too big. ");
          OK = false;
        }
      break;
    case WA_WHITE:
      if(indx == SN_AM)
        if((modeVal[iSet.selChan][wave][WH_DC] + val/2) > MAXDC || (modeVal[iSet.selChan][wave][WH_DC] - val/2 < MINDC))
        {
          strcat(errMsg, "Amp + Offset too big. ");
          OK = false;
        }
      if(indx == SN_DC)
        if((modeVal[iSet.selChan][wave][WH_AM]/2 + val) >  MAXDC || (modeVal[iSet.selChan][wave][WH_AM]/2 - val) < MINDC)
        {
          strcat(errMsg, "Amp + Offset too big. ");
          OK = false;
        }
      break;

    case WA_SQR:
    case WA_TRI: // sqr and TRI have same index values and limits
    /*
      if(indx == SQ_V1)
        if((modeVal[iSet.selChan][wave][SQ_V2] + val) > MAXDC || (modeVal[iSet.selChan][wave][SQ_V2] - val) < MINDC)
        {
          strcat(errMsg, "V1 >= V2. ");
          OK = false;
        }
      if(indx == SQ_V2)
        if((modeVal[iSet.selChan][wave][SQ_V1] + val) >  MAXDC || (modeVal[iSet.selChan][wave][SQ_V1] - val) < MINDC)
        {
          strcat(errMsg, "V1 >= V2. ");
          OK = false;
        }
      break;
*/
    case WA_PULSE:
    /*
      if(indx == PU_V1)
        if((modeVal[iSet.selChan][wave][PU_V2] + val) > MAXDC || (modeVal[iSet.selChan][wave][PU_V2] - val) < MINDC)
        {
          strcat(errMsg, "V1 >= V2. ");
          OK = false;
        }
      if(indx == PU_V2)
        if((modeVal[iSet.selChan][wave][PU_V1] + val) >  MAXDC || (modeVal[iSet.selChan][wave][PU_V1] - val) < MINDC)
        {
          strcat(errMsg, "V1 >= V2. ");
          OK = false;
        }
*/
    case WA_STEP:
    // Step V1 does not need to be < V2
      break;
    case WA_IMD:
    if(indx == IM_A1)
      if((val + modeVal[iSet.selChan][wave][IM_A2]) > MAXDC)
      {
        strcat(errMsg, "A1 + A2 too big. ");
        OK = false;
      }
     if(indx == IM_A2)
      if((val + modeVal[iSet.selChan][wave][IM_A1]) > MAXDC)
      {
        strcat(errMsg, "A1 + A2 too big. ");
        OK = false;
      }
      break;  
  } // switch
 
  // Chan B value set in BURST + BALTA
  if(BURST_MODE && BALTA && iSet.selChan == CHAN_B && wave < WA_SWEEP)
  {
    strcat(errMsg, "B settings disabled in BURST + BaltA. ");
    OK = false;
  }
 // Chan B value set in CONT + COUPLE
  if(BCOUPLE && iSet.selChan == CHAN_B && wave < WA_SWEEP)
  {
    strcat(errMsg, "B settings disabled in COUPLED mode. ");
    OK = false;
  }
  // Chan B value set A = IMD
  if(iSet.waveForm[CHAN_A] == WA_IMD && iSet.selChan == CHAN_B && wave < WA_SWEEP)
  {
    strcat(errMsg, "B settings disabled when A is IMD. ");
    OK = false;
  }

  // sweep Pulse: effective V2 < V1
  // put into AC_SWEEP_NEXT code
  if(!OK)
    Serial.printf("CheckVal: [%s]\n",errMsg);
  if(strlen(errMsg))
      errMessage(errMsg, 5);
  return OK;
}

int isTF(char* valStr)
{
  String first = String(valStr);
  first.toUpperCase();
  if (first == "T" || first == "1" || first == "+" || first == "ON") 
  {
      return 1;
  }
  if (first == "F" || first == "0"|| first == "-" || first == "OFF") 
  {
      return 0;
  }
  return -1;
}

void  SCPIerr(const char text[], String val)
{
  char txtbuf[SCPIERRLEN+20];
  sprintf(txtbuf, "%s: '%s'", text, val.c_str());
  // avoid buffer overruns
  if((strlen(txtbuf) + strlen(lastSCPIerr)) > MULTIERRORLEN)
    lastSCPIerr[0] = '\0';
  strcat(lastSCPIerr, txtbuf); //:SYSTEM:ERROR?
  errMessage(txtbuf,5); // screen and web
  Serial.printf("SCPI error [%i][%i]: %s\n", strlen(txtbuf), strlen(lastSCPIerr), txtbuf);
}

void getError(SCPI_C commands, SCPI_P parameters, Stream& interface) 
{
  interface.println(lastSCPIerr);
  lastSCPIerr[0] = '\0'; // clear out buffer
}

void Identify(SCPI_C commands, SCPI_P parameters, Stream& interface) 
{
  char buf[64];
  sprintf(buf, "Platypus,%s,S%i.%i_H%i",comms.instName,EE_VERSION,SOFT_VERSION,HARD_VERSION);
  interface.println(buf);
}

void getPost(SCPI_C commands, SCPI_P parameters, Stream& interface) 
{
  interface.println(postMsg);
}
void getIP(SCPI_C commands, SCPI_P parameters, Stream& interface) 
{
  interface.println(myIP);
}

void DoNothing(SCPI_C commands, SCPI_P parameters, Stream& interface) 
{
  String header = String(commands.Last());
  Serial.printf("Found unimplemented cmd: \n", header.c_str());
}

void DoSpecialSCPI(SCPI_C commands, Stream& interface) 
{
  Serial.printf("Found special command\n");
}

// SYSTEM functions

// locate existing SSID or a blank space
void setSSID(SCPI_C commands, SCPI_P parameters, Stream& interface) 
{ 
  int i;
  bool found = false;
  int blank = -1;
  //String newSSID  = String(parameters.First());
  //std::transform(newSSID.begin(), newSSID.end(), newSSID.begin(), ::toupper);
  //String savedSSID;

  for(i = 0; i < WIFI_STORED; i++)
  {
    if(strlen(comms.STA_ssid[i]) == 0)
      blank = i;
      
   // savedSSID = String(comms.STA_ssid[i]));
   // std::transform(savedSSID.begin(), savedSSID.end(), savedSSID.begin(), ::toupper);
    if(strcasecmp(comms.STA_ssid[i], parameters.First()) == 0)
    {
      found = true;
      break;
    }    
  }
  if(!found)
    if(blank >= 0)
      i = blank;
    else
      i = 0;
  lastAddedSSID = i;
  strcpy(comms.STA_ssid[lastAddedSSID], parameters.First());
  printStoredWiFi();
}

// Uses lastAddedSSID set by last setSSID() call
void setPass(SCPI_C commands, SCPI_P parameters, Stream& interface) 
{
  strcpy(comms.STA_pass[lastAddedSSID], parameters.First()); 
}

void setWiFi(SCPI_C commands, SCPI_P parameters, Stream& interface) 
{
  int val = isTF(parameters.First());
  if (val == 1) 
  {
    comms.enabled = true;
    Serial.printf("Start WiFI\n");
    netBegin();
    valChange(VAL_CHGD_REMOTE); 
    return;
  }
  if (val == 0) 
  {
    comms.enabled = false;
    Serial.printf("Stop WiFI\n");
    wifiEnd(false);
    valChange(VAL_CHGD_REMOTE); 
    return;
  }
  SCPIerr("Bad Sweep enable value", String(parameters.First()));
}
void setFactReset(SCPI_C commands, SCPI_P parameters, Stream& interface) 
{
   factReset(99);
   writeEE(); // immediate update
}

// channel params
void setChanSel(SCPI_C commands, SCPI_P parameters, Stream& interface) 
{
  String first = String(parameters.First());
  first.toUpperCase();
  if (first == "A" || first == "1") 
  {
      iSet.selChan = CHAN_A;
      lastRemoteChan = CHAN_A;
      Serial.println ("SEL A");
      valChange(VAL_CHGD_REMOTE); 
      return;
  }
  if (first == "B" || first == "2") 
  {
      iSet.selChan = CHAN_B;
      lastRemoteChan = CHAN_B;
      Serial.println ("SEL B");
      valChange(VAL_CHGD_REMOTE); 
      return;
  }
  SCPIerr("Bad value", first);
}
void getChanSel(SCPI_C commands, SCPI_P parameters, Stream& interface) 
{
  interface.println(iSet.selChan+1);   
}
// validate add Channel / Control / Burst combinations 
bool checkWave(int chan, int wave)
{
  if(chan == CHAN_A)
  { 
    switch (iSet.modeA )
    {
      case MO_BURST :
        return true;

      case MO_SWEEP :
        return true;
   
      case MO_NOR :
      default:
        return true;
    }
  }
  if (chan == CHAN_B)
  {
    
    if(BCOUPLE)
      return false;
    if(wave == WA_IMD || wave == WA_WHITE)
      return false;     
  }
  return true;
}

// set and get waveForm
void setWave(SCPI_C commands, SCPI_P parameters, Stream& interface)
{
  int i;
  bool found = false;
  String first = String(parameters.First());
  iSet.selChan = lastRemoteChan;
  for(i = 0; i < MODEITEMS; i++)
  {
    
    if (strcasecmp(modeDefs[i].name, first.c_str()) == 0)
    {
      found = true;
      Serial.print("*");
      break;
    }
    //Serial.printf("SetWave [%s]=[%s]?\n",first.c_str(), modeDefs[i].name);
  }
  // legitimate waveform for this channel?
  //Serial.printf("Set Chan %i to %i: %s, upd_scrn %i\n", iSet.selChan, i, modeDefs[i].name, updateVal_Scrn);    
  if (found &&  (modeEnabled[iSet.selChan] & (1 << i))) // && checkWave(iSet.selChan, i) 
  {
    iSet.waveForm[iSet.selChan] = (DDSwaves)i;
    setScrMode(iSet.selChan, (DDSwaves)i, MAIN_MENU);   
    fifoCmd = fifoCmd | (iSet.selChan == CHAN_A) ? FUNC_CHANGE_A : FUNC_CHANGE_B;  
    valChange(VAL_CHGD_REMOTE);  
    
  }
  else
    SCPIerr("Bad waveform", first);
} 

// generic functions for values to/from modeDefs
// not used as there's no benefit
void getWave(SCPI_C commands, SCPI_P parameters, Stream& interface) 
{
  int wav = iSet.waveForm[iSet.selChan];
  interface.printf("%s\n", modeDefs[wav].name);
}

// set value with simple ID and float value e.g. AMP 2.0
void setWaveParamF(char* valStr, int wave, int valIndx)
{
 
  float val = atof(valStr);
  bool ok = checkVal(val, wave, valIndx);
  iSet.selChan = lastRemoteChan;
  //Serial.printf("SWP %s %s", modeDefs[wave].name, modeDefs[wave].text[valIndx], valStr);
  if(ok)
  {
    modeVal[iSet.selChan][wave][valIndx] = val;
    fifoCmd = fifoCmd | (iSet.selChan) ? VAL_CHANGE_B : VAL_CHANGE_A;
    valChange(VAL_CHGD_REMOTE); 
  }
  else
  {
    sprintf(SCPIerrMsgBuf, "Bad %s %s", modeDefs[wave].name, modeDefs[wave].text[valIndx], valStr);
    SCPIerr(SCPIerrMsgBuf, String(valStr));
  }
}

// set value with numeric suffix and float value eg V2 2.0
void setWaveParamNumF(char *valStr, char *cmdStr, int wave, int baseIndx) 
{
  int suffix = 0;
  int found = 0;
  //char SCPIerrMsgBuf[256];
  float val = atof(valStr);
  String header = String(cmdStr);
  header.toUpperCase();
  found = sscanf(header.c_str(),"%*[V]%d", &suffix); // SQR, TRI, PULSE, STEP have V# (no others have :V arg)
  if(!found)
    found = (wave == WA_IMD && sscanf(header.c_str(),"%*[FREQ]%d", &suffix)); // IMD has FREQ# and AMP# (TRI, SQR, SINE have FREQ also)
  if(!found)
    found = (wave == WA_IMD && sscanf(header.c_str(),"%*[AMP]%d", &suffix)); // IMD has FREQ# and AMP# (SINE has FREQ also)
  if(wave == WA_IMD)
    iSet.selChan = CHAN_A;
  else
    iSet.selChan = lastRemoteChan;
  Serial.printf("Set %s [%s]#%i (%i) = %3.2f, ch %i, to val %i\n",  modeDefs[wave].name, header.c_str(), suffix, found, val, iSet.selChan, baseIndx);
  bool ok = checkVal(val, wave, baseIndx); // limits will be the same
  if(ok)
  {
    if(found)
      suffix --;
    else
      suffix = 0;
    modeVal[iSet.selChan][wave][baseIndx + suffix] = val;
    fifoCmd = fifoCmd | (iSet.selChan == 0) ? VAL_CHANGE_A : VAL_CHANGE_B;
    valChange(VAL_CHGD_REMOTE); 
  }
  else
  {
    sprintf(SCPIerrMsgBuf, "Bad %s %s %3.2f", modeDefs[wave].name, modeDefs[wave].text[baseIndx + suffix - 1]);
    SCPIerr(SCPIerrMsgBuf, String(valStr));
  }
 
}
// get float value with numeric suffix e.g. V2?
void getWaveParamNumF(SCPI_C commands, Stream& interface, int wave, int baseIndx) 
{
 int suffix;
 int found = 0;
 String header = String(commands.Last());
 iSet.selChan = lastRemoteChan;
 header.toUpperCase();
 found = sscanf(header.c_str(),"%*[V]%d", &suffix); // SQR, TRI, PULSE, STEP have V# (no others have :V arg)
 if(!found)
    found = (wave == WA_IMD && sscanf(header.c_str(),"%*[FREQ]%d", &suffix)); // IMD has FREQ# and AMP# (TRI, SQR, SINE have FREQ also)
 if(!found)
    found = (wave == WA_IMD && sscanf(header.c_str(),"%*[AMP]%d", &suffix)); // IMD has FREQ# and AMP# (SINE has FREQ also)
  if(found)
    suffix--;
 //sscanf(header.c_str(),"%*[V]%u", &suffix);
 Serial.printf("Get %s [%s] %i\n",  modeDefs[wave].name, header.c_str(), suffix);
 interface.println(modeVal[iSet.selChan][wave][baseIndx + suffix]); 
}

// sine
void setSinFreq(SCPI_C commands, SCPI_P parameters, Stream& interface)  {  setWaveParamF(parameters.First(), WA_SINE, SN_FR); }
void setSinAmp(SCPI_C commands, SCPI_P parameters, Stream& interface)   {  setWaveParamF(parameters.First(), WA_SINE, SN_AM); }
void setSinDC(SCPI_C commands, SCPI_P parameters, Stream& interface)    {  setWaveParamF(parameters.First(), WA_SINE, SN_DC); }
void getSinFreq(SCPI_C commands, SCPI_P parameters, Stream& interface)  {  interface.println(modeVal[iSet.selChan][WA_SINE][SN_FR], SCPIDIGITS); }
void getSinAmp(SCPI_C commands, SCPI_P parameters, Stream& interface)   {  interface.println(modeVal[iSet.selChan][WA_SINE][SN_AM], SCPIDIGITS); }
void getSinDC(SCPI_C commands, SCPI_P parameters, Stream& interface)    {  interface.println(modeVal[iSet.selChan][WA_SINE][SN_DC], SCPIDIGITS); }
void setCor(SCPI_C commands, SCPI_P parameters, Stream& interface) 
{
  bool val = isTF(parameters.First());
  iSet.selChan = lastRemoteChan;
  if (val >= 0)
  {
    _sineCorrect = val;
    //Serial.printf("Sine Cor %i\n",_sineCorrect);
    fifoCmd = fifoCmd | (iSet.selChan) ? VAL_CHANGE_B : VAL_CHANGE_A;
    valChange(VAL_CHGD_REMOTE); 
  }
  else
    SCPIerr("Bad Sine Correction value", String(parameters.First()));
}
// 0..x
void setPreshoot(SCPI_C commands, SCPI_P parameters, Stream& interface) 
{
  int val = atoi(parameters.First());
  iSet.selChan = lastRemoteChan;
  if (val >= 0)
  {
    _sqrPreshoot = val;
    Serial.printf("Square pre-compensation %i\n",_sqrPreshoot);
    fifoCmd = fifoCmd | (iSet.selChan) ? VAL_CHANGE_B : VAL_CHANGE_A;
    valChange(VAL_CHGD_REMOTE); 
  }
  else
    SCPIerr("Bad Sine Correction value", String(parameters.First()));
}
//square
void setSqrFreq(SCPI_C commands, SCPI_P parameters, Stream& interface)  {  setWaveParamNumF(parameters.First(), commands.Last(), WA_SQR, SQ_FR); }
void setSqrVolts(SCPI_C commands, SCPI_P parameters, Stream& interface) {  setWaveParamNumF(parameters.First(), commands.Last(), WA_SQR, SQ_V1); } // V2 will offset
void setSqrDuty(SCPI_C commands, SCPI_P parameters, Stream& interface)  {  setWaveParamF(parameters.First(), WA_SQR, SQ_DU); }
void getSqrFreq(SCPI_C commands, SCPI_P parameters, Stream& interface)  {  interface.println(modeVal[iSet.selChan][WA_SQR][SQ_FR], SCPIDIGITS); }
void getSqrVolts(SCPI_C commands, SCPI_P parameters, Stream& interface) {  getWaveParamNumF(commands, interface, WA_SQR, SQ_V1); }
void getSqrDuty(SCPI_C commands, SCPI_P parameters, Stream& interface)  {  interface.println(modeVal[iSet.selChan][WA_SQR][SQ_DU], SCPIDIGITS); }

// Tri
void setTriFreq(SCPI_C commands, SCPI_P parameters, Stream& interface)  {  setWaveParamF(parameters.First(), WA_TRI, TR_FR); }
void setTriVolts(SCPI_C commands, SCPI_P parameters, Stream& interface) {  setWaveParamNumF(parameters.First(), commands.Last(), WA_TRI, TR_V1); }
void setTriDuty(SCPI_C commands, SCPI_P parameters, Stream& interface)  {  setWaveParamF(parameters.First(), WA_TRI, TR_DU); }
void getTriFreq(SCPI_C commands, SCPI_P parameters, Stream& interface)  {  interface.println(modeVal[iSet.selChan][WA_TRI][TR_FR], SCPIDIGITS); }
void getTriVolts(SCPI_C commands, SCPI_P parameters, Stream& interface) {   getWaveParamNumF(commands, interface, WA_TRI, TR_V1);}
void getTriDuty(SCPI_C commands, SCPI_P parameters, Stream& interface)  {  interface.println(modeVal[iSet.selChan][WA_TRI][TR_DU], SCPIDIGITS);}

// IMD
void setImdFreq(SCPI_C commands, SCPI_P parameters, Stream& interface) {  setWaveParamNumF(parameters.First(), commands.Last(), WA_IMD, IM_F1); }
void setImdAmp(SCPI_C commands, SCPI_P parameters, Stream& interface)  {  setWaveParamNumF(parameters.First(), commands.Last(), WA_IMD, IM_A1); }
void getImdFreq(SCPI_C commands, SCPI_P parameters, Stream& interface) {  getWaveParamNumF(commands, interface, WA_IMD, IM_F1); }
void getImdAmp(SCPI_C commands, SCPI_P parameters, Stream& interface)  {  getWaveParamNumF(commands, interface, WA_IMD, IM_A1);}

//White 
void setWhiteAmp(SCPI_C commands, SCPI_P parameters, Stream& interface) {  setWaveParamNumF(parameters.First(), commands.Last(), WA_WHITE, WH_AM); }
void setWhiteDC(SCPI_C commands, SCPI_P parameters, Stream& interface)  {  setWaveParamNumF(parameters.First(), commands.Last(), WA_WHITE, WH_DC); }
void getWhiteAmp(SCPI_C commands, SCPI_P parameters, Stream& interface) {  getWaveParamNumF(commands, interface, WA_WHITE, WH_AM);  }
void getWhiteDC(SCPI_C commands, SCPI_P parameters, Stream& interface)  {  getWaveParamNumF(commands, interface, WA_WHITE, WH_DC);  }

//Pulse
void setPulVolts(SCPI_C commands, SCPI_P parameters, Stream& interface) {  setWaveParamNumF(parameters.First(), commands.Last(), WA_PULSE, PU_V1); }
void setPulTlo(SCPI_C commands, SCPI_P parameters, Stream& interface)   {  setWaveParamF(parameters.First(), WA_PULSE, PU_TL); }
void setPulTri(SCPI_C commands, SCPI_P parameters, Stream& interface)   {  setWaveParamF(parameters.First(), WA_PULSE, PU_TR); }
void setPulThi(SCPI_C commands, SCPI_P parameters, Stream& interface)   {  setWaveParamF(parameters.First(), WA_PULSE, PU_TH); }
void setPulTfa(SCPI_C commands, SCPI_P parameters, Stream& interface)   {  setWaveParamF(parameters.First(), WA_PULSE, PU_TF); }
void getPulVolts(SCPI_C commands, SCPI_P parameters, Stream& interface) {  getWaveParamNumF(commands, interface, WA_PULSE, PU_V1);  }
void getPulTlo(SCPI_C commands, SCPI_P parameters, Stream& interface)   {  interface.println(modeVal[iSet.selChan][WA_PULSE][PU_TL], SCPIDIGITS);  }
void getPulTri(SCPI_C commands, SCPI_P parameters, Stream& interface)   {  interface.println(modeVal[iSet.selChan][WA_PULSE][PU_TR], SCPIDIGITS);  }
void getPulThi(SCPI_C commands, SCPI_P parameters, Stream& interface)   {  interface.println(modeVal[iSet.selChan][WA_PULSE][PU_TH], SCPIDIGITS);  }
void getPulTfa(SCPI_C commands, SCPI_P parameters, Stream& interface)   {  interface.println(modeVal[iSet.selChan][WA_PULSE][PU_TF], SCPIDIGITS);  }

// Step
void setStepVolts(SCPI_C commands, SCPI_P parameters, Stream& interface) {  setWaveParamNumF(parameters.First(), commands.Last(), WA_STEP, ST_V1); }
void setStepSup(SCPI_C commands, SCPI_P parameters, Stream& interface)   {  setWaveParamF(parameters.First(), WA_STEP, ST_SU); }
void setStepSdn(SCPI_C commands, SCPI_P parameters, Stream& interface)   {  setWaveParamF(parameters.First(), WA_STEP, ST_SD); }
void setStepTup(SCPI_C commands, SCPI_P parameters, Stream& interface)   {  setWaveParamF(parameters.First(), WA_STEP, ST_TU); }
void setStepTdn(SCPI_C commands, SCPI_P parameters, Stream& interface)   {  setWaveParamF(parameters.First(), WA_STEP, ST_TD); }
void setStepRepeat(SCPI_C commands, SCPI_P parameters, Stream& interface) 
{
  bool val = isTF(parameters.First());
  iSet.selChan = lastRemoteChan;
  if (val >= 0)
  {
    modeVal[iSet.selChan][iSet.waveForm[iSet.selChan]][ST_RE] = val;
    fifoCmd = fifoCmd | (iSet.selChan) ? VAL_CHANGE_B : VAL_CHANGE_A;
    valChange(VAL_CHGD_REMOTE); 
  }
  else
    SCPIerr("Bad Step Repeat value", String(parameters.First()));
}
void getStepVolts(SCPI_C commands, SCPI_P parameters, Stream& interface)    {  getWaveParamNumF(commands, interface, WA_STEP, ST_V1); }
void getStepSup(SCPI_C commands, SCPI_P parameters, Stream& interface)      {  interface.println(modeVal[iSet.selChan][WA_STEP][ST_SU], SCPIDIGITS);  }
void getStepSdn(SCPI_C commands, SCPI_P parameters, Stream& interface)      {  interface.println(modeVal[iSet.selChan][WA_STEP][ST_SD], SCPIDIGITS);  }
void getStepTup(SCPI_C commands, SCPI_P parameters, Stream& interface)      {  interface.println(modeVal[iSet.selChan][WA_STEP][ST_TU], SCPIDIGITS);  }
void getStepTdn(SCPI_C commands, SCPI_P parameters, Stream& interface)      {  interface.println(modeVal[iSet.selChan][WA_STEP][ST_TD], SCPIDIGITS);  }
void getStepRepeat(SCPI_C commands, SCPI_P parameters, Stream& interface)   {  interface.println((modeVal[iSet.selChan][WA_STEP][ST_RE])?'T':'F'); }


// STOP START
void setChanStart(SCPI_C commands, SCPI_P parameters, Stream& interface) 
{
  iSet.selChan = lastRemoteChan;
  fifoCmd = fifoCmd | (iSet.selChan) ? START_B : START_A;
  Serial.printf("Start %i\n", iSet.selChan);
  valChange(VAL_CHGD_REMOTE); 
}
void setChanStop(SCPI_C commands, SCPI_P parameters, Stream& interface) 
{
  iSet.selChan = lastRemoteChan;
  fifoCmd = fifoCmd | (iSet.selChan) ? STOP_B : STOP_A;
  Serial.printf("Stop %i\n", iSet.selChan);
  valChange(VAL_CHGD_REMOTE); 
}
// start or stop
void setChanEnable(SCPI_C commands, SCPI_P parameters, Stream& interface) 
{
  int val = isTF(parameters.First());
  iSet.selChan = lastRemoteChan;
  if (val == 1) 
  {
      fifoCmd = fifoCmd | (iSet.selChan) ? START_B : START_A;
      Serial.printf("Start %i\n", iSet.selChan);
      valChange(VAL_CHGD_REMOTE); 
      return;
  }
  if (val == 0) 
  {
      fifoCmd = fifoCmd | (iSet.selChan) ? STOP_B : STOP_A;
      Serial.printf("Stop %i\n", iSet.selChan);
      valChange(VAL_CHGD_REMOTE); 
      return;
  }
  SCPIerr("Bad value", String(parameters.First()));
}

void getChanEnable(SCPI_C commands, SCPI_P parameters, Stream& interface) 
{
  iSet.selChan = lastRemoteChan;
  interface.println((dds[iSet.selChan].run)?'T':'F');
}



// Sweep
void setSweepStart(SCPI_C commands, SCPI_P parameters, Stream& interface) 
{  
  fifoCmd = fifoCmd |  START_SWEEP ;
  Serial.printf("Start SWEEP\n");
  valChange(VAL_CHGD_REMOTE); 
}
void setSweepStop(SCPI_C commands, SCPI_P parameters, Stream& interface) 
{
 fifoCmd = fifoCmd |  STOP_SWEEP ;
 Serial.printf("Stop SWEEP\n");
 valChange(VAL_CHGD_REMOTE); 
}
//start or stop Sweep
void setSweepEnable(SCPI_C commands, SCPI_P parameters, Stream& interface) 
{
  int val = isTF(parameters.First());
  if (val == 1) 
  {
    iSet.modeA = MO_SWEEP;
    Serial.printf("Set sweep\n");
    valChange(VAL_CHGD_REMOTE); 
    return;
  }
  if (val == 0) 
  {
    iSet.modeA = MO_NOR;
    Serial.printf("Cancel sweep mode\n");
    valChange(VAL_CHGD_REMOTE); 
    return;
  }
  SCPIerr("Bad Sweep enable value", String(parameters.First()));
}
void getSweepEnable(SCPI_C commands, SCPI_P parameters, Stream& interface) {  interface.println(((iSet.modeA == MO_SWEEP)) ? "T" : "F");} // && swCont.run

// log sweeps are legal for all legal sweeps 
bool sweepLegal(int wave, int type)
{
  sweepMode_t tst = (sweepMode_t)type; //  == 255 for illegal value
  if(tst >= SW_MODES)
    return false;
  switch(wave)
  {
    case WA_SINE :
      return (type != SWM_DC);
    case WA_SQR :
    case WA_TRI :
      return true; // all legal modes accepted
    case WA_PULSE :
      return (type == SWM_AM);
    case WA_IMD : // no sweeps allowed
    case WA_WHITE :
    case WA_STEP :
    default:
      return false;
  }
  return false;
}
void setSweepType(SCPI_C commands, SCPI_P parameters, Stream& interface) 
{
  //char SCPIerrMsgBuf[256];
  int wave = iSet.waveForm[CHAN_A]; 
  String typeS = String(parameters.First());
  typeS.toUpperCase();
  int type = 99; // illegal
  if(typeS == "F") type = SWM_FR;
  if(typeS == "V") type = SWM_AM;
  if(typeS == "D") type = SWM_DC;

  if (sweepLegal(iSet.waveForm[CHAN_A], type))
  {
    sweepVal[wave].val[0][SW_TYPE] = type;
    fifoCmd = fifoCmd | CHANGE_SWEEP;
    valChange(VAL_CHGD_REMOTE); 
  }
  else 
  {
    sprintf(SCPIerrMsgBuf, "Bad %s sweep type", modeDefs[wave].name);
    SCPIerr(SCPIerrMsgBuf, String(parameters.First()));
  }
}

// Sweep values are stored in sweepVals
void setSweepParamF(char *valStr, int indx) 
{
  int wave = iSet.waveForm[CHAN_A];
  int type = sweepVal[wave].val[0][SW_TYPE];
  //char SCPIerrMsgBuf[256];
  float val = atof(valStr);
  bool ok;
  Serial.printf("Sweep [%s] %3.2f, indx %i, wave %i\n", valStr, val, indx, wave);

  if(indx == SW_INI || indx == SW_FIN)
  {
    ok = (val >= sweepLim[wave].min[type] && val <= sweepLim[wave].max[type]);
    Serial.printf(" Low %1.2f, Hi %1.2f\n", sweepLim[wave].min[type], sweepLim[wave].max[type]);
  }
  else
  {
    ok = (val >= modeDefs[WA_SWEEP].lowLim[indx] && val <= modeDefs[WA_SWEEP].highLim[indx]);
    Serial.printf(" Low %1.2f, Hi %1.2f\n", modeDefs[WA_SWEEP].lowLim[indx], modeDefs[WA_SWEEP].highLim[indx]);
  }

  //checkVal(val, wave, indx); // limits will be from chan A waveform x FAD type
  if(ok)
  {
    sweepVal[wave].val[type][indx] = val;
    fifoCmd = fifoCmd | CHANGE_SWEEP | VAL_CHANGE_A ;
    valChange(VAL_CHGD_REMOTE); 
  }
  else
  {
    sprintf(SCPIerrMsgBuf, "Bad %s sweep val %s", modeDefs[wave].name, modeDefs[wave].text[indx]);
    SCPIerr(SCPIerrMsgBuf, String(val));
  } 
}

void setSweepInit(SCPI_C commands, SCPI_P parameters, Stream& interface)  { setSweepParamF(parameters.First(), SW_INI); }
void setSweepFinal(SCPI_C commands, SCPI_P parameters, Stream& interface) { setSweepParamF(parameters.First(), SW_FIN); }
void setSweepSteps(SCPI_C commands, SCPI_P parameters, Stream& interface) { setSweepParamF(parameters.First(), SW_STE); }
void setSweepTime(SCPI_C commands, SCPI_P parameters, Stream& interface)  { setSweepParamF(parameters.First(), SW_TIM); }

float getSweepVal(int wave, int indx)
{
  int type = sweepVal[wave].val[0][SW_TYPE];
  return sweepVal[wave].val[type][indx];
}
char iToVFD(int swpType)
{
  switch(swpType)
  {
    case 0:
      return 'V';
    case 1:
      return 'F';
    case 2: 
      return 'D';
  }
  return 'X';
}


void getSweepType(SCPI_C commands, SCPI_P parameters, Stream& interface)  { interface.println(iToVFD(sweepVal[iSet.waveForm[CHAN_A]].val[0][SW_TYPE])); }
void getSweepInit(SCPI_C commands, SCPI_P parameters, Stream& interface)  { interface.println(getSweepVal(iSet.waveForm[CHAN_A], SW_INI), SCPIDIGITS); }
void getSweepFinal(SCPI_C commands, SCPI_P parameters, Stream& interface) { interface.println(getSweepVal(iSet.waveForm[CHAN_A], SW_FIN), SCPIDIGITS); }
void getSweepSteps(SCPI_C commands, SCPI_P parameters, Stream& interface) { interface.println(getSweepVal(iSet.waveForm[CHAN_A], SW_STE), SCPIDIGITS); }
void getSweepTime(SCPI_C commands, SCPI_P parameters, Stream& interface) { interface.println(getSweepVal(iSet.waveForm[CHAN_A], SW_TIM), SCPIDIGITS); }

void setSweepLog(SCPI_C commands, SCPI_P parameters, Stream& interface) 
{
  int wave = iSet.waveForm[CHAN_A];
  int type = sweepVal[wave].val[0][SW_TYPE];
  bool isLegal = sweepLegal(wave, type); // all legal waveform/mode combos allow log sweeps??
  int val = isTF(parameters.First());
  //Serial.printf("SweepLog [%s] legal %i, val %i\n",  parameters.First(), isLegal, val);
  if (val >= 0 && isLegal) 
  {
    sweepVal[wave].val[type][SW_LOG] = val;
    fifoCmd = fifoCmd | CHANGE_SWEEP ;
    //Serial.printf("sweep log change\n");
    valChange(VAL_CHGD_REMOTE); 
    return;
  }  
  SCPIerr("Bad Sweep Log value", String(parameters.First()));
}
void getSweepLog(SCPI_C commands, SCPI_P parameters, Stream& interface) { interface.println((getSweepVal(iSet.waveForm[CHAN_A], SW_LOG))?"T":"F");  }
void setSweepRep(SCPI_C commands, SCPI_P parameters, Stream& interface)
{
  //char SCPIerrMsgBuf[3];
  int rep = isTF(parameters.First());
  int wave = iSet.waveForm[CHAN_A];
  int type = sweepVal[wave].val[0][SW_TYPE];
  if (rep >= 0) 
  {
    iSet.selChan = lastRemoteChan;
    //sprintf(SCPIerrMsgBuf,"%i",rep);
    sweepVal[wave].val[type][SW_REP] = rep;  
    valChange(VAL_CHGD_REMOTE); 
    return;
  }
   SCPIerr("Bad Sweep Repeat value", String(parameters.First()));
}

void getSweepRep(SCPI_C commands, SCPI_P parameters, Stream& interface) { interface.println(getSweepVal(iSet.waveForm[CHAN_A], SW_REP) ? "T" : "F");  }



// Burst
void setBurstStart(SCPI_C commands, SCPI_P parameters, Stream& interface) 
{
  fifoCmd = fifoCmd |  START_BURST ;
  valChange(VAL_CHGD_REMOTE); 
  Serial.printf("Start Burst\n");
}
void setBurstStop(SCPI_C commands, SCPI_P parameters, Stream& interface) 
{
 fifoCmd = fifoCmd |  STOP_BURST ;
 valChange(VAL_CHGD_REMOTE); 
 Serial.printf("Stop Burst\n");
}
//start or stop Sweep
void setBurstEnable(SCPI_C commands, SCPI_P parameters, Stream& interface) 
{
  int tfp = isTF(parameters.First());
  if (tfp == 1) 
  {
    iSet.modeA = MO_BURST;
    Serial.printf("Set Burst mode\n");
    valChange(VAL_CHGD_REMOTE); 
    return;
  }
  if (tfp == 0) 
  {
    iSet.modeA = MO_NOR;
    Serial.printf("Cancel Burst mode\n");
    valChange(VAL_CHGD_REMOTE); 
    return;
  }
  SCPIerr("Bad Burst enable value", String(parameters.First()));
}
void getBurstEnable(SCPI_C commands, SCPI_P parameters, Stream& interface) 
{
  interface.println((iSet.modeA == MO_BURST)? "T" : "F");
}

void setBurstCycon(SCPI_C commands, SCPI_P parameters, Stream& interface)   
{ 
  int ch = iSet.selChan; 
  iSet.selChan = CHAN_A; 
  setWaveParamF(parameters.First(), WA_BURST, BU_ON);  
  iSet.selChan = ch; 
  valChange(VAL_CHGD_REMOTE); 
}
void setBurstCycoff(SCPI_C commands, SCPI_P parameters, Stream& interface)  
{ 
  int ch = iSet.selChan; 
  iSet.selChan = CHAN_A; 
  setWaveParamF(parameters.First(), WA_BURST, BU_OFF); 
  iSet.selChan = ch;
}
void getBurstCycsOff(SCPI_C commands, SCPI_P parameters, Stream& interface)  {  interface.println(modeVal[CHAN_A][WA_BURST][BU_OFF],SCPIDIGITS);  }
void getBurstCycsOn(SCPI_C commands, SCPI_P parameters, Stream& interface)   {  interface.println(modeVal[CHAN_A][WA_BURST][BU_ON],SCPIDIGITS);  }
void setBurstRepeat(SCPI_C commands, SCPI_P parameters, Stream& interface)
{
  int tfp = isTF(parameters.First());
  if (tfp >= 0)
  {    
      modeVal[CHAN_A][WA_BURST][BU_REP] = tfp;
      fifoCmd = fifoCmd | CHANGE_BURST;    
      valChange(VAL_CHGD_REMOTE); 
  }
  else
      SCPIerr("Bad Burst Repeat value", String(parameters.First())); 
}
void getBurstRepeat(SCPI_C commands, SCPI_P parameters, Stream& interface)  {  interface.println((modeVal[CHAN_A][WA_BURST][BU_REP])?'T':'F');  }



// Control
void setContCpl(SCPI_C commands, SCPI_P parameters, Stream& interface) 
{
  int tfp = isTF(parameters.First());
  Serial.printf("SCPI COnt CPl %i\n", tfp);
  if (tfp >= 0) 
  {
    modeVal[CHAN_A][WA_CONT][CP_BA] = tfp;
    fifoCmd = fifoCmd | CHANGE_CONTROL;
    Serial.printf("Control Change B=A\n");
    valChange(VAL_CHGD_REMOTE); 
    return;
  }  
  SCPIerr("Bad Control B=A value", String(parameters.First()));
}
void getContCpl(SCPI_C commands, SCPI_P parameters, Stream& interface) {  interface.println((BCOUPLE)?'T':'F');  }
void setContPh(SCPI_C commands, SCPI_P parameters, Stream& interface)  {  setWaveParamF(parameters.First(), WA_CONT, CP_PH); }
void getContPh(SCPI_C commands, SCPI_P parameters, Stream& interface)  {  interface.println(modeVal[CHAN_A][WA_CONT][CP_PH],SCPIDIGITS);  }

void setBurstBaltA(SCPI_C commands, SCPI_P parameters, Stream& interface)
{
  int tfp = isTF(parameters.First());
  if (tfp >= 0)
  {    
      modeVal[CHAN_A][WA_BURST][BU_BALTA] = tfp;
      fifoCmd = fifoCmd | CHANGE_BURST;
      valChange(VAL_CHGD_REMOTE);     
  }
  else
      SCPIerr("Bad Burst BaltA value", String(parameters.First())); 
}
void getBurstBaltA(SCPI_C commands, SCPI_P parameters, Stream& interface)  {  interface.println((modeVal[CHAN_A][WA_BURST][BU_BALTA])? 'T':'F');  }

void setContOutPol(SCPI_C commands, SCPI_P parameters, Stream& interface) 
{
  int tfp = isTF(parameters.First());
  if (tfp >= 0) 
  {
    modeVal[CHAN_A][WA_CONT][CP_OUTPOL] = tfp;
    fifoCmd = fifoCmd | CHANGE_CONTROL;
    Serial.printf("Control Change OutPol %i\n", tfp);
    valChange(VAL_CHGD_REMOTE);
    return;
  }  
  SCPIerr("Bad Control OutPol value", String(parameters.First()));
}
void getContOutPol(SCPI_C commands, SCPI_P parameters, Stream& interface)  {  interface.println((modeVal[CHAN_A][WA_CONT][CP_OUTPOL])?'+':'-');  }

void setContInPol(SCPI_C commands, SCPI_P parameters, Stream& interface) 
{
  int tfp = isTF(parameters.First());
  if (tfp >= 0) 
  {
    modeVal[CHAN_A][WA_CONT][CP_INPOL] = tfp;
    fifoCmd = fifoCmd | CHANGE_CONTROL;
    Serial.printf("Control Change InPol %i\n", tfp);
    valChange(VAL_CHGD_REMOTE); 
    return;
  }  
  SCPIerr("Bad Control OutPol value", String(parameters.First()));
}
void getContInPol(SCPI_C commands, SCPI_P parameters, Stream& interface)  {  interface.println((modeVal[CHAN_A][WA_CONT][CP_INPOL])?'+':'-');  }

void setContExtTrig(SCPI_C commands, SCPI_P parameters, Stream& interface) 
{
  int tfp = isTF(parameters.First());  
  if (tfp >= 0) 
  {
    modeVal[0][WA_CONT][CP_ET] = tfp;
    fifoCmd = fifoCmd | CHANGE_CONTROL;
    Serial.printf("Control Change B=A\n");
    valChange(VAL_CHGD_REMOTE); 
    return;
  }  
  SCPIerr("Bad Control B=A value", String(parameters.First()));
}
void getContExtTrig(SCPI_C commands, SCPI_P parameters, Stream& interface)  {  interface.println((modeVal[CHAN_A][WA_CONT][CP_ET]) ?"T":"F");  }
#endif