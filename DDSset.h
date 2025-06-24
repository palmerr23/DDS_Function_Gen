#ifndef DDSSET_H_
#define DDSSET_H_
/*
 DDS-side mode management: takes valued from modeVals, sweepVal, burst, iSet, etc and moves them into DDS.x swCont.x or burst.x and actions result
 Only called from CPU1
 Actual DDS in DDS.h
 see also DDSdefs.h

 *** NO CALLS TO PRINT IN THIS FILE (causes breaks in generation) ***
*/
//uint32_t samples = 0;
//#define EVERY 150000

float logMult(void);
void setMode(DDSmodes mode);
void errMessage(const char * message, int16_t onTime);

// for Pulse modes not used by Sine.
int voltsToCounts_Pulse(int chan, float volts)
{
  int tmp = maxPulseVal * (volts / vCal[chan]);
 //Serial.printf("V2C_F %i V %2.3f {%1.4f} -> c %i\n", chan, volts, vCal[chan], tmp);
  return maxPulseVal * (volts / vCal[chan]); // overflow possible for volts/vCal > 1
}


// channel A only
int setSweep(int dummy)
{
  setMode(MO_SWEEP);  
  int wave = iSet.waveForm[CHAN_A];
  int mode = (sweepMode_t) ((int)sweepVal[iSet.waveForm[CHAN_A]].val[0][0]);
  digitalWrite(TRIG_OUTPIN, TRIG_OUT_POL); // turned off after first cycle
  sweep.mode = (sweepMode_t) ((int)sweepVal[iSet.waveForm[CHAN_A]].val[0][0]);
  // copy in from sweepVal settings
  //{SW_TYPE, SW_INI, SW_FIN, SW_TIM, SW_STE, SW_LOG, SW_REP}
  sweep.start  = sweepVal[wave].val[mode][SW_INI];
  sweep.end    = sweepVal[wave].val[mode][SW_FIN];
  sweep.steps  = sweepVal[wave].val[mode][SW_STE];
  sweep.time   = sweepVal[wave].val[mode][SW_TIM];
  sweep.log = sweepVal[wave].val[mode][SW_LOG];
  sweep.repeat = sweepVal[wave].val[mode][SW_REP];

  swCont.incEvery = sampleRate * sweep.time / (sweep.steps - 1);
  swCont.incCount = 0;
  swCont.stepCount = 0;
  //Serial.printf("SET SWEEP: type %i, from %2.2f to %2.2f, time %2.2f, steps %i, IncEvery %i, repeat %i\n", sweep.mode, sweep.start, sweep.time, sweep.end, sweep.steps, swCont.incEvery, sweep.repeat );
  // don't start the sweep
  swCont.done = false; 
  return 0;
}

int startSweep(int dummy)
{
  setSweep(99);
  swCont.done = false; 
  swCont.run = true;
  setFunc(0, AC_SWEEP_START, true);
  dds[0].run = true;
  if(BCOUPLE)
  {
   // setFunc(1, AC_SWEEP_START, true);
    dds[1].run = true;
  }
  return 0;
}
int stopSweep(int dummy)
{
  swCont.done = true; 
  swCont.run = false;
  dds[0].run = false;
  trigInActive = false;
  // iSet.run[0] = false;
  return 0;
}

int setBurst(int dummy)
{
 // dds[0].run = false;
  setMode(MO_BURST);  
  //burst.run = true;
  setFunc(0, AC_NORMAL, true);
  //trigOutOn = false; // disable per-cycle trig out pulse
  //trigOutActive = TRIG_DISABLE;
  if(BALTA)
  {
    iSet.waveForm[1] = iSet.waveForm[0];
    setFunc(1, AC_NORMAL, true);
  }
  burst.cyclesOn = modeVal[CHAN_A][WA_BURST][BU_ON];
  burst.cyclesOff = modeVal[CHAN_A][WA_BURST][BU_OFF];  
  //burst.bAltA = false; //modeVal[CHAN_A][WA_BURST][BU_OFF]; // unused
  burst.repeat = modeVal[CHAN_A][WA_BURST][BU_REP]; 
//// iSet.run[0];
  return 0;
}
int startBurst(int dummy)
{
  setBurst(99);
  dds[0].run = true;
  if(BALTA)
    dds[1].run = true;
  burst.run = true;  
  digitalWrite(TRIG_OUTPIN, TRIG_OUT_POL);  // extTrig() sets ZC flag
  burstTrigState = true;
  return 0;
}

int stopBurst(int dummy)
{
    burst.run = false;
    dds[0].run = false;
    if(BALTA)
      dds[1].run = false;
    burst.state = B_IDLE;
    trigInActive = false;
    updateVal_web = true; // update the RUN/STOP indicator
    digitalWrite(TRIG_OUTPIN, !TRIG_OUT_POL);
    return 0;
}

int toggleBurst(int) // callback from ENC switch or screen button
{
  //Serial.println("T Burst");
  if(burst.run)
    stopBurst(99);
  else 
    startBurst(99);
    valChange(VAL_CHGD_LOCAL);
  return CALL_NOEX; 
}

void setMode(DDSmodes mode)
{
  iSet.modeA = mode;
  switch (mode)
  {
    case MO_NOR :
      burst.run = false;
      swCont.run = false;
      //waveOn = true; 
      break;

    case MO_SWEEP :
      burst.run = false;
      //swCont.run = true;
      //waveOn = false; 
      break;

    case MO_BURST :
      //burst.run = true;
      swCont.run = false;
      
      //waveOn = false; 
      break;
  }  
}


// scale factor for freq due to filter roll off
float sinScaleF(float freq)
{
  float factor = 1.0;
  if(!_sineCorrect)
    return factor;
  for(int i = 0; i < SINCORLEN-1; i++)
    if(freq >= sineCor[i].freq && freq < sineCor[i+1].freq)
    {
        factor = mapf(freq, sineCor[i].freq, sineCor[i+1].freq, sineCor[i].factor, sineCor[i+1].factor);
        //Serial.printf("SineCor i %i, freq %3.2f, fact %3.2f\n", i, freq, factor);
        return factor;
    }
    return 1.0;
}

void sineScaleTest(float start, float end, int steps)
{
  int i;
  float freq, res, fStep;
  fStep = (end - start)/(steps-1);
  freq = start;
  Serial.printf("Sine Scale Test: F0 %3.1f, F1 %3.1f, Steps %i\n", start, end, steps);
  while (freq <= end)
  {
    Serial.printf("%3.1f, %1.6f\n", freq, sinScaleF(freq));
    freq += fStep;
  }
  Serial.println("Break test");
  for(i = 0; i < SINCORLEN; i++)
    Serial.printf("F0 %5.1f, %1.5f %1.5f %1.5f\n", sineCor[i].freq , sinScaleF(sineCor[i].freq -5.0), sinScaleF(sineCor[i].freq),sinScaleF(sineCor[i].freq +5.0));
}

// setXXX routines translate modeVal[] into working values for DDS routines

// DDS routines avoid float values wherever possible.
//////////////////////// SINE ///////////////////////
void setSine(int chan, dds_actions action, bool restart)
{
  float amp, freq, DCoff;

  dds[chan].waveForm = WA_SINE; 
  crossingLevel[chan] = sinval[chan].dcOff; 
  int sChan = chan;
  // chan O needs to be set before the chan 1 call
  if((chan == CHAN_B) && (BCOUPLE || (BURST_MODE && BALTA))) 
  {  
    int phaseOff = 0;  
    sChan = CHAN_A;  
    sinval[1].inc   = sinval[0].inc;
    sinval[1].scale = sinval[0].scale;
    sinval[1].dcOff = sinval[0].dcOff;
    if(!BURST_MODE)
      phaseOff =  DDS_INTER_MUL * (DDS_TAB_LEN * BPHASE / 360); // watch for overflow
    sinval[1].phAcc = sinval[0].phAcc - phaseOff; // lagging phase
    // idle = first lookup  value 
    if(BCOUPLE && !(BURST_MODE && BALTA))
    {
      int sinVal32 = DDSsineLookup_i(sinval[1].phAcc); 
      sinVal32 = - sinVal32 * sinval[0].scale; 
      sinVal32 = (sinVal32 >> INTSCALE_BITS);
      sinVal32 += sinval[0].dcOff; // scaled up value
      // shift up/down to appropriate output (16 or 32 bits)
      idleVal[chan] = shiftIt(sinVal32); 
    }
    if(BURST_MODE && BALTA)
      idleVal[chan] = (modeVal[chan][WA_SINE][SN_DC]/(2*vCal[chan])) * maxDAC; // phase offset!!!!
    dds[1].run = dds[0].run;
   // Serial.printf("SINE-CPL[%i]:  ph offset %i\n", chan, phaseOff >> DDS_INTER_BITS);
    if(BURST_MODE)
      burst.state = B_ACTIVE; 
    return;
  }

  // stop clipping by reducing amplitude
  if(modeVal[chan][WA_SINE][SN_AM] + 2 * abs(modeVal[chan][WA_SINE][SN_DC]) > MAXAC)
  {
    valChange(VAL_CHGD_LOCAL);  
    modeVal[chan][WA_SINE][SN_AM] = MAXAC - 2 * abs(modeVal[chan][WA_SINE][SN_DC]) - 0.01; // slightly lower value stops hunting
    if(modeVal[chan][WA_SINE][SN_AM] < 0)
      modeVal[chan][WA_SINE][SN_AM] = 0;
  }
  
  if(action == AC_IDLE)
  {
    dds[chan].run = false; 
    //maxV = modeVal[chan][WA_TRI][TR_V2]; // used by coupling
    return;
  }

  if(action == AC_NORMAL) // or burst
  {  
    sinval[chan].inc   = modeVal[chan][WA_SINE][SN_FR] * DDS_INTER_MUL * DDS_TAB_LEN / sampleRate;
    sinval[chan].scale = sinScaleF(modeVal[chan][WA_SINE][SN_FR]) * (modeVal[chan][WA_SINE][SN_AM]/vCal[chan]) * INTSCALE_MUL / (2 * AMP_MUL); // * 0.5 for P-P -> ampl
    sinval[chan].dcOff = (modeVal[chan][WA_SINE][SN_DC]/vCal[chan]) * maxVal / AMP_MUL; // shifted and scaled    
 
    idleVal[chan] = (modeVal[chan][WA_SINE][SN_DC]/(2*vCal[chan])) * maxDAC;
  //Serial.printf("SINE-NOR[%i]: Fr %3.1f, amp %3.1f, offset %3.1f, Inc %i scale %i offset %i\n", 
      //chan, modeVal[chan][WA_SINE][SN_FR], modeVal[chan][WA_SINE][SN_AM], modeVal[chan][WA_SINE][SN_DC], 
   // sinval[chan].inc, sinval[chan].scale, sinval[chan].dcOff);
    burst.state = B_ACTIVE;
    return;
  }

  if(action == AC_SWEEP_START) // SINE
  {   
    if(sweep.log)
    {
      if(sweep.start <= SMALL_VAL) // trap illegal log() values
      {
        sweep.start = SMALL_VAL;
         errMessage("Log sweep start <= 0. ", 5);
      }
      if(sweep.end <= SMALL_VAL)
      {
        sweep.end = SMALL_VAL;
        errMessage("Log sweep end <= 0. ", 5);
      }
    }   
    DCoff = sinval[chan].dcOff = (modeVal[chan][WA_SINE][SN_DC]/vCal[chan]) * maxVal/ AMP_MUL; // shifted and scaled  
    idleVal[chan] = sinval[chan].dcOff;
    // default values from sine mode, sweep values may overwrite these
    freq = swCont.currFreq = modeVal[chan][WA_SINE][SN_FR];
    amp  = swCont.currAmp  = modeVal[chan][WA_SINE][SN_AM];
    switch ((sweepMode_t) ((int)sweepVal[iSet.waveForm[CHAN_A]].val[0][0]))
    { 
      case SWM_FR :
        freq = swCont.currFreq = sweep.start;
        break;
      case SWM_AM :
        amp  = swCont.currAmp = sweep.start;
        break;
      case SWM_DC :
      default :
        errMessage("ERROR: Duty Cycle sweep not allowed for Sine.", 5);
        //Serial.println();
        sweep.start = SMALL_VAL;
    } 
    
    if(sweep.log)
      swCont.incVal = logMult(); // multiply   
    else
      swCont.incVal = (sweep.end - sweep.start) / (sweep.steps -1); // add
 
    swCont.incCount  = 0;
    swCont.stepCount = 0;
    swCont.done = false;

    sinval[chan].inc = swCont.currFreq * DDS_INTER_MUL * DDS_TAB_LEN / sampleRate;

    sinval[chan].scale = sinScaleF(swCont.currFreq) * (swCont.currAmp/vCal[chan]) * INTSCALE_MUL / (2 *AMP_MUL); // 0.5 * amplitude
    //if(!sweep.hold)
    sinval[chan].phAcc = 0; 
    idleVal[chan] = (modeVal[chan][WA_SINE][SN_DC]/vCal[chan]) * maxDAC;
    //Serial.printf("SW-SINE-ST: chan %i, mode %i, F %3.1f, inc %3.1f (%3.1f -> %3.1f) steps %i, time %3.1f scale %i\n", chan, sweep.mode, swCont.currFreq, swCont.incVal, sweep.start, sweep.end, sweep.steps, sweep.time, sinval[chan].scale );
    return; //sweep start
  }

  if(action == AC_SWEEP_NEXT) // SINE
  {     
    sweepMode_t mode = (sweepMode_t) ((int)sweepVal[iSet.waveForm[CHAN_A]].val[0][0]);
    bool fwdSweep = sweep.end > sweep.start;
    if(mode == SWM_FR)
    {
      //Serial.printf("SW_NX: fr %3.2f + inc %3.2f, %i, ",swCont.currFreq, swCont.incVal, sweep.log);
      if(sweep.log)
        swCont.currFreq *= swCont.incVal;
      else
        swCont.currFreq += swCont.incVal;      
      if (fwdSweep && swCont.currFreq > sweep.end || !fwdSweep && swCont.currFreq < sweep.end)
        swCont.done = true;
      sinval[chan].inc = swCont.currFreq * DDS_INTER_MUL * DDS_TAB_LEN / sampleRate;
    }
    if(mode == SWM_AM)
    {
      if(sweep.log)
        swCont.currAmp *= swCont.incVal;  
      else
        swCont.currAmp += swCont.incVal;  
      if (fwdSweep && swCont.currAmp > sweep.end || !fwdSweep && swCont.currAmp < sweep.end)
        swCont.done = true;
      sinval[chan].scale = sinScaleF(swCont.currFreq) * (swCont.currAmp/vCal[chan])* INTSCALE_MUL / (2 * AMP_MUL); 
    }
    //Serial.printf("SW-SINE-NXT: chan %i,Fr %3.1f, Amp %1.3f, phase inc %i\n",chan, swCont.currFreq, swCont.currAmp, sinval[chan].inc);
    restart = (BCOUPLE && mode == SWM_FR) ? true : restart; // force to retain phase relationship
    /*
    if(!BCOUPLE)
      restart = false;
    else
      restart = true; 
      */ 
  }

  if(restart)
      sinval[chan].phAcc = 0; 
} // end SINE



// Channel A only - uses both sinVal[] entries
/////////////////////////// INTERMOD //////////////////////
void setIntermod()
{
  dds[0].waveForm = WA_IMD; 
  // stop clipping by reducing amplitude
  if((modeVal[0][WA_IMD][IM_A1] + modeVal[0][WA_IMD][IM_A2]) > MAXAC)
  {
    valChange(VAL_CHGD_LOCAL);  
    modeVal[0][WA_IMD][IM_A2] = MAXAC - modeVal[0][WA_IMD][IM_A1] - 0.01; // slightly lower value stops hunting
    if(modeVal[0][WA_IMD][IM_A2] < 0)
      modeVal[0][WA_IMD][IM_A2] = 0;
  }
  //dds[chan].freq = modeVal[chan][WA_SINE][SN_FR];
  sinval[0].inc   = modeVal[0][WA_IMD][IM_F1] * DDS_INTER_MUL * DDS_TAB_LEN / sampleRate;
  sinval[0].scale = sinScaleF(modeVal[0][WA_IMD][IM_F1]) * (modeVal[0][WA_IMD][IM_A1]/vCal[0]) * INTSCALE_MUL / (2 * AMP_MUL); // * 0.5 for P-P -> ampl
  // second IMD frequency saved in chan 1 values, output on chan 0
  sinval[1].inc   = modeVal[0][WA_IMD][IM_F2] * DDS_INTER_MUL * DDS_TAB_LEN / sampleRate;
  sinval[1].scale = sinScaleF(modeVal[0][WA_IMD][IM_F2]) * (modeVal[0][WA_IMD][IM_A2]/vCal[0]) * INTSCALE_MUL / (2 * AMP_MUL); // still on channel 0
  idleVal[0] = 0;
  sinval[1].dcOff = sinval[0].dcOff = 0; 
  sinval[0].phAcc = sinval[1].phAcc = 0;
  crossingLevel[0] = 0; 
  //Serial.printf("IMD: Fr %3.1f [%3.1f], Inc %i [%i] scale %1.3f [%1.3f]\n", modeVal[0][WA_IMD][IM_F1], modeVal[0][WA_IMD][IM_F2], sinval[0].inc, sinval[1].inc, sinval[0].scale, sinval[1].scale);
} // end INTERMOD




// riseSampsStep close to 1 produces substantial granularity in gradients, so use a longer wait between steps and a bigger step increment.
#define RAT_TRIG 50  // use multi sample steps below this value - balance between slope granularity and step height
#define RAT_MUL 50   // not necessarily the same value as RAT_TRIG
////////////////////// PULSE //////////////////////////
void setPulse(int chan, dds_actions action, bool restart)
{
  dds[chan].waveForm = WA_PULSE;
  //int tfc, v2c, ssc;
  int sChan = chan;
  if(chan == CHAN_B && BALTA && BURST_MODE)
  {
    sChan = CHAN_A; // set up pulse[B] from modeVal[A]
    iSet.waveForm[CHAN_B] = WA_PULSE;
    //Serial.println("PUL BaltA");
  }
  if(chan == CHAN_B && (BCOUPLE && !BURST_MODE))
  {
    sChan = CHAN_A; // set up pulse[B] from modeVal[A]
    iSet.waveForm[CHAN_B] = WA_PULSE;
    //Serial.println("PUL B=A");
    dds[1].run = dds[0].run;
  }
  
  // only amplitude sweeps allowed
  if(action == AC_NORMAL || action == AC_SWEEP_START) // sweep start is modified below
  {   
    pulse[chan].minVal    = voltsToCounts_Pulse(chan, modeVal[sChan][WA_PULSE][PU_V1]/AMP_MUL);
    pulse[chan].maxVal    = voltsToCounts_Pulse(chan, modeVal[sChan][WA_PULSE][PU_V2]/AMP_MUL);
    pulse[chan].maxSamps  = msToSamples(modeVal[sChan][WA_PULSE][PU_TH]);        
    pulse[chan].minSamps  = msToSamples(modeVal[sChan][WA_PULSE][PU_TL]);
    pulse[chan].riseSamps = msToSamples(modeVal[sChan][WA_PULSE][PU_TR]);
    pulse[chan].fallSamps = msToSamples(modeVal[sChan][WA_PULSE][PU_TF]);   
    dds[chan].cyclesToGo = 0; // infinite (debug) 
  }

  idleVal[chan] = pulse[sChan].minVal;

  sweepMode_t sMode = (sweepMode_t) ((int)sweepVal[iSet.waveForm[CHAN_A]].val[0][0]); // always ampl for pulse
  if(action == AC_SWEEP_START && sMode == SWM_AM) // PULSE
  {
    swCont.currAmp = sweep.start; 
    pulse[chan].maxVal  = voltsToCounts_Pulse(chan, swCont.currAmp/AMP_MUL);
    //pulse[chan].riseInc = gapS / pulse[sChan].riseSamps;
    if(sweep.log)
      swCont.incVal = logMult(); // multiply   
    else
      swCont.incVal = (sweep.end - sweep.start) / (sweep.steps -1); 
     // Serial.printf("-PUL_SW_ST\n");
  }

  if(action == AC_SWEEP_NEXT && sMode == SWM_AM && !swCont.done) // PULSE
  {     
    bool fwdSweep = sweep.end > sweep.start;
    if(sweep.log)
      swCont.currAmp *= swCont.incVal;  
    else
      swCont.currAmp += swCont.incVal;  
    //if (fwdSweep && swCont.currAmp > sweep.end || !fwdSweep && swCont.currAmp < sweep.end)
      //swCont.done = true;
    pulse[chan].maxVal  = voltsToCounts_Pulse(chan, swCont.currAmp/AMP_MUL);
    //pulse[chan].riseInc = gapS / pulse[sChan].riseSamps;
    //Serial.printf("-PUL_SW_NXT rest %i\n", restart);
  }

  int64_t gap  = (int64_t)pulse[chan].maxVal  - (int64_t)pulse[chan].minVal;  
  //int64_t gapS = (int64_t)pulse[sChan].maxVal - (int64_t)pulse[sChan].minVal;
  pulse[chan].riseInc   = gap / pulse[chan].riseSamps;
  pulse[chan].riseSampsStep  = 1; // best for steep ramps 

  pulse[chan].fallInc   = gap / pulse[sChan].fallSamps; // subtracted in code
  pulse[chan].fallSampsStep  = 1;    
  /*if(chan == 1 && BCOUPLE && BALTA)
    idleVal[chan] = pulse[0].minVal;  // coupled, out of phase
  else
  */
 
  if(abs(pulse[chan].riseInc) < RAT_TRIG)   // a step every few samples for moderately slow ramps
  { // ratiometric - increase delay between steps and increment
    if (pulse[chan].riseSamps < 1) // avoid div0 below
      pulse[chan].riseSamps = 1;
    
    pulse[chan].riseSampsStep = RAT_MUL * pulse[chan].riseSamps / gap;
    pulse[chan].riseInc = gap * pulse[chan].riseSampsStep / pulse[chan].riseSamps;
    if(pulse[chan].riseInc < 1) // at least one count per step
      pulse[chan].riseInc = 1;
    //Serial.print(" RAT_R");
  }

  if(abs(pulse[chan].fallInc) < RAT_TRIG) 
  {    
    if (pulse[chan].fallSamps < 1)
      pulse[chan].fallSamps = 1;

    pulse[chan].fallSampsStep = RAT_MUL * pulse[chan].fallSamps / gap;
    pulse[chan].fallInc = gap * pulse[chan].fallSampsStep / pulse[chan].fallSamps;
    if(pulse[chan].fallInc < 1)
      pulse[chan].fallInc = 1;
   // Serial.print(" RAT_F");
  }
  //int totSamps = pulse[chan].maxSamps + pulse[chan].minSamps + pulse[chan].riseSamps + pulse[chan].fallSamps;
 // Serial.printf("WA_PULSE[%i, %i], Min %i [%3.3f], ", chan, sChan, pulse[chan].minVal, modeVal[chan][WA_PULSE][PU_V1]);
 // Serial.printf("Max %i [%3.3f]\n", pulse[chan].maxVal, modeVal[chan][WA_PULSE][PU_V2]);
  //Serial.printf("TL %i [%3.3f]\n", pulse[chan].minSamps, modeVal[chan][WA_PULSE][PU_TL]);
  //Serial.printf("TH %i [%3.3f]\n", pulse[chan].maxSamps, modeVal[chan][WA_PULSE][PU_TH]);

 // longlong x;
 // x.ll = gap;
 // Serial.printf(" TR %lli [%3.3f], samps %i, steps %i, ", pulse[chan].riseInc, modeVal[chan][WA_PULSE][PU_TR], pulse[chan].riseSamps, pulse[chan].riseSteps);
 // Serial.printf(" gap %lli ", gap);  Serial.printf("[0x%08x %08x]\n", x.l[1], x.l[0]);
  //Serial.printf(" gapS %lli\n", gapS);
 // Serial.printf("TF %i [%3.3f], samps %i, steps %i. Tot samps %i\n", pulse[chan].fallInc,  modeVal[chan][WA_PULSE][PU_TF], pulse[chan].fallSamps, pulse[chan].fallSteps, totSamps);
  // Go!
  if(restart) // PULSE
  {
    dds[chan].state = PU_LOW;
    dds[chan].samplesToGo = 0;
  }
} // end PULSE

void printPulses()
{
  Serial.printf("Pulses: HS %i [%i], FS %i [%i], LS %i [%i], RS %i [%i]\n", pulse[0].maxSamps, pulse[1].maxSamps, pulse[0].fallSamps, pulse[1].fallSamps, 
        pulse[0].minSamps,  pulse[1].minSamps, pulse[0].riseSamps, pulse[1].riseSamps);
  Serial.printf("        RI %lli [%lli], FI %lli [%lli] ]\n", pulse[0].riseInc, pulse[1].riseInc, pulse[0].fallInc, pulse[1].fallInc); // 64 bit values
  Serial.printf("        Max %i, Comp FI>>%i %lli, min %i\n", pulse[0].maxVal, _sqrPreshoot, pulse[0].fallInc >> _sqrPreshoot, pulse[0].minVal);
  Serial.printf("        RSS %i [%i], FSS %i [%i]\n", pulse[0].riseSampsStep, pulse[1].riseSampsStep, pulse[0].fallSampsStep, pulse[1].fallSampsStep);
}


////////////////////// SQUARE ///////////////////////
void setSqr(int chan, dds_actions action, bool restart)
{
  int samps, inc;
  float maxv, freq, duty;
  int sChan = chan; // source channel
  dds[chan].waveForm = WA_SQR; 
  
  // coupled
  if(chan == CHAN_B && BCOUPLE && !BURST_MODE)
  {
    sChan = CHAN_A;  // source most settings from channel 0

    dds[1].run = dds[0].run;
  }
  if(chan == CHAN_B && BURST_MODE && BALTA) // SQR
  {
    sChan = CHAN_A;
    dds[1].run = dds[0].run;
  }
  
  // minVal does not change with sweeps;  
  idleVal[chan] = voltsToCounts_Pulse(chan, modeVal[sChan][WA_SQR][SQ_V2]/AMP_MUL);
  float minv = modeVal[sChan][WA_SQR][SQ_V1]/AMP_MUL;
  pulse[chan].minVal  = voltsToCounts_Pulse(chan, minv);   

  if(action == AC_NORMAL || action == AC_IDLE) // idle also resets to standard values
  {  
    freq =  modeVal[sChan][WA_SQR][SQ_FR];   
    pulse[chan].maxVal  = voltsToCounts_Pulse(chan, modeVal[sChan][WA_SQR][SQ_V2]/AMP_MUL);
    pulse[chan].riseInc = ((int64_t)pulse[sChan].maxVal - (int64_t)pulse[sChan].minVal)/AMP_MUL;
    pulse[chan].fallInc = pulse[sChan].riseInc;
    duty = modeVal[sChan][WA_SQR][SQ_DU];
    dds[chan].state = PU_LOW; // always restart
    if(BURST_MODE)
      burst.state = B_ACTIVE;
    //Serial.printf("-ST_NO- ");    
  }

  if(action == AC_IDLE) // SQR
  {
    dds[chan].state = PU_IDLE;
    //pulse[chan].maxVal  = voltsToCounts_Pulse(chan, modeVal[chan][WA_SQR][SQ_V2]); // used by coupling - done above
    restart = false; // override
  }
  // AC_SWEEP_NEXT will only change the values for the appropriate "type"
  // some duplicated code to avoid additional integer divisions and float arithmetic
  if(action == AC_SWEEP_START) // always CHAN_A
  {   
      if(sweep.log)
      {
        if(sweep.start <= SMALL_VAL) // trap illegal log() values
        {
          sweep.start = SMALL_VAL;
          errMessage("Log sweep start <= 0. ", 5);
        }
        if(sweep.end <= SMALL_VAL)
        {
          sweep.end = SMALL_VAL;
          errMessage("Log sweep end <= 0. ", 5);
        }
      } 
      sweepMode_t mode = (sweepMode_t) ((int)sweepVal[iSet.waveForm[0]].val[0][0]);
      freq = swCont.currFreq = modeVal[sChan][WA_SQR][SQ_FR];
      maxv = swCont.currAmp  = modeVal[sChan][WA_SQR][SQ_V2]/AMP_MUL;
      duty = swCont.currDuty = modeVal[sChan][WA_SQR][SQ_DU];
      switch (mode)
      { 
        case SWM_FR :
          freq = swCont.currFreq = sweep.start;
          break;
        case SWM_AM :
          maxv = swCont.currAmp = sweep.start/AMP_MUL;
          break;
        case SWM_DC :
          duty = swCont.currDuty = sweep.start;
          if(duty < SMALL_VAL && sweep.log)
            duty = SMALL_VAL; // avoid undefined log values
          break;     
      }   
    
    if(sweep.log)
      swCont.incVal = logMult(); // multiply      
    else      
      swCont.incVal = (sweep.end - sweep.start) / (sweep.steps -1); // add

    swCont.incCount = 0; // SQR
    swCont.stepCount = 0; 
    swCont.currAmp = maxv; 
    swCont.currDuty = duty;
    swCont.currFreq = freq;

    pulse[chan].maxVal  = voltsToCounts_Pulse(chan, maxv);
    // minVal does not change
    pulse[chan].riseInc = (int64_t)pulse[chan].maxVal - (int64_t)pulse[chan].minVal; // square, all in one go
    pulse[chan].fallInc = pulse[chan].riseInc;
    //swCont.done = false; 
    //Serial.printf("-SQ_ST- ch %i, inc %3.1f, mode %i, FR %3.1f , AM %3.1f, DU %3.1f: ", chan, swCont.incVal, sweep.mode, swCont.currFreq, swCont.currAmp, swCont.currDuty, duty );
    swCont.run = true;
    restart = true; // override    
  }
  
  if(action == AC_SWEEP_NEXT && !swCont.done) // SQR
  {
    if((sweep.mode == SWM_FR) && BCOUPLE)
      restart = true; // both channels to retain phase
    if(sweep.mode == SWM_FR && chan == 0)
    {
      if(sweep.log)
        swCont.currFreq *= swCont.incVal;
        
      else
        swCont.currFreq += swCont.incVal;
      //Serial.printf("-SQ_NE- ch %i, FR %3.1f inc %3.1f, ", chan, swCont.currFreq, swCont.incVal);
    }
    freq = swCont.currFreq; 

    if(sweep.mode == SWM_AM && chan == 0) // recalculate amplitude 
    {
      if(sweep.log)
        swCont.currAmp *= swCont.incVal; 
      else        
        swCont.currAmp += swCont.incVal; 

      pulse[chan].maxVal  = voltsToCounts_Pulse(chan, swCont.currAmp);
      pulse[chan].riseInc = (int64_t)pulse[chan].maxVal - (int64_t)pulse[chan].minVal;
      pulse[chan].fallInc = pulse[chan].riseInc;   
     // Serial.printf("-SQ_NE- ch %i, Amp %3.1f inc %3.1f: ", chan, swCont.currAmp, swCont.incVal);
    }
    // maxv not used later on

    if(sweep.mode == SWM_DC && chan == 0) // SQR
    {
      if(sweep.log)
        swCont.currDuty *= swCont.incVal;       
      else 
        swCont.currDuty += swCont.incVal;
   
    // Serial.printf("-SQ_NE- ch %i, DC %3.1f inc %3.1f: ", chan, swCont.currDuty, swCont.incVal);
    }
    duty = swCont.currDuty;

    if(duty < SMALL_VAL && sweep.log)
        duty = SMALL_VAL; // avoid multiply by zero

    //restart = false;  // override
  } // swp_next
  samps = sampleRate / freq;
  pulse[chan].maxSamps  = (samps * duty / 100) -1;   // Rise & Fall take 2 sample each     
  pulse[chan].minSamps  = samps - pulse[sChan].maxSamps -2;
  pulse[chan].riseSamps = 0;
  pulse[chan].fallSamps = 0;
  pulse[chan].riseSteps = 0; 
  pulse[chan].fallSteps = 0;
  // some sweep and burst options must be synchronised to retain phase
  if(restart || (BCOUPLE && !BURST_MODE) || (BURST_MODE && BALTA))
  {
    dds[chan].state = PU_LOW;
    dds[chan].samplesToGo = 0;
  }

  // set phase relationship for BCOUPLE
  if((BCOUPLE && !BURST_MODE) && (BPHASE >= SMALL_VAL)) // SQR (chan == CHAN_B) && 
  {      
    bool firstPart = (BPHASE/360.0 < duty/100.00);
    float prop = (BPHASE/360)*(100/duty);
    float samps = (BPHASE/360)*(pulse[sChan].maxSamps + pulse[sChan].minSamps);
    // lagging phase
    if(samps < pulse[CHAN_A].minSamps)
    {
      dds[CHAN_B].state = PU_LOW;
      dds[CHAN_B].samplesToGo = samps;
    }
    else // second part
    {
      dds[CHAN_B].state = PU_HIGH;     
      dds[CHAN_B].samplesToGo = samps - pulse[CHAN_A].minSamps;
    }
    //Serial.printf("BU SQ du %3.2f, first? %i, prop %3.2f, stg %i, maxS %i, minS %i\n", du, firstPart, prop, dds[chan].samplesToGo, pulse[chan].maxSamps, pulse[chan].minSamps);
  }
  //Serial.println();
// Serial.printf("SQR ch %i, tot samps %i\n", chan, pulse[sChan].maxSamps + pulse[sChan].minSamps);
 //Serial.printf("WA_SQR[%i]: Min %i, Max %i, duty %3.1f, H/L samps  %i / %i, R inc %i\n", 
       //      chan, pulse[chan].minVal, pulse[chan].maxVal, duty, pulse[chan].maxSamps, pulse[chan].minSamps, pulse[chan].riseInc);
} // end SQUARE



///////////////////  TRIANGLE ///////////////////
void setTri(int chan, dds_actions action, bool restart)
{
  int samps;
  dds[chan].waveForm = WA_TRI;
  //int tfc, v2c,ssc;
  float maxV, minV; // for V sweep: V High changes between these values 
  float freq, m0, duty;//, tf, jumpV;
  sweepMode_t mode;

  int sChan = chan;
 // minV = modeVal[chan][WA_TRI][TR_V1]/AMP_MUL;
 // pulse[chan].minVal = voltsToCounts_Pulse(chan, minV); 
 // idleVal[chan] = pulse[chan].minVal;

  // coupling
  if(chan == 1 && BURST_MODE && BALTA)
  {
    sChan = 0;    
    //Serial.printf("TRI[1] BU_BALTA: idle %i\n", idleVal[chan]);
  }

  if(chan == 1 && BCOUPLE && !BURST_MODE) // TRI
  {
     sChan = 0;
     dds[1].run = dds[0].run;    
   //Serial.printf("TRI[1] CPL: BPHASE %3.2f\n, ", BPHASE);
    //return; // no further processing
  }  
  
  minV = modeVal[sChan][WA_TRI][TR_V1]/AMP_MUL;
  pulse[chan].minVal = voltsToCounts_Pulse(chan, minV); 
  idleVal[chan] = pulse[chan].minVal;

  if(action == AC_NORMAL) // normal mode
  {      
    swCont.currFreq = modeVal[sChan][WA_TRI][TR_FR]; // swCont.x used as temp for AC_NORMAL mode
    swCont.currAmp  = modeVal[sChan][WA_TRI][TR_V2]/AMP_MUL;
    //if(maxV <  minV) // V2 < V1 forbidden
    //    maxV =  minV;
    swCont.currDuty = duty = modeVal[sChan][WA_TRI][TR_DU]; 
   
    if(BURST_MODE)
      burst.state = B_ACTIVE;
    else
      burst.state = B_IDLE;
    if(BCOUPLE && !BURST_MODE) // phase calc needs to be reset
      restart = true;
   // Serial.printf("-TR_NOR: %i\n", sChan);
  }

  if(action == AC_IDLE)  // TRI
  {
    dds[chan].state = PU_IDLE;
    swCont.currAmp = modeVal[sChan][WA_TRI][TR_V2]/AMP_MUL; // used by coupling
    restart = false; // override
    //Serial.printf("-TR_IDL- %i\n", sChan);
  }

  if(action == AC_SWEEP_START) // TRI
  {  
    // generic  values for sweep 
    sweepMode_t mode = (sweepMode_t) ((int)sweepVal[iSet.waveForm[CHAN_A]].val[0][0]);
    minV  = modeVal[sChan][WA_TRI][TR_V1]/AMP_MUL; 
    swCont.currAmp  = modeVal[sChan][WA_TRI][TR_V2]/AMP_MUL; 
    //duty = swCont.currDuty = modeVal[sChan][WA_TRI][TR_DU]; 
//if(maxV <  minV) // V2 < V1 forbidden
    //    maxV =  minV;
   // maxV = swCont.currAmp;  
    //freq = swCont.currFreq;
    //duty = swCont.currDuty; 

    if(chan == CHAN_A) // set values. Chan B just copies (above)
    {
      swCont.currFreq = modeVal[sChan][WA_TRI][TR_FR];
      duty = swCont.currDuty = modeVal[sChan][WA_TRI][TR_DU];  
      //freq = swCont.currFreq = modeVal[sChan][WA_TRI][TR_FR]; // defaults, before applying sweep type values   
      //maxV = swCont.currAmp  = modeVal[sChan][WA_TRI][TR_V2]/AMP_MUL; 
      //duty = swCont.currDuty = modeVal[sChan][WA_TRI][TR_DU]; 
      //if(swCont.currAmp <  minV) // V2 < V1 forbidden
       // swCont.currAmp = minV +1;
      if(sweep.log)
      {
        if(sweep.start <= SMALL_VAL) // trap illegal log() values
        {
          sweep.start = SMALL_VAL;
          errMessage("Log sweep start <= 0. ", 5);
        }
        if(sweep.end <= SMALL_VAL)
        {
          sweep.end = SMALL_VAL;
          errMessage("Log sweep end <= 0. ", 5);
        }
      }         
      if(mode == SWM_FR)
      {   
        swCont.currFreq = sweep.start; 
      }
      
      if(mode == SWM_AM)
      {   
        swCont.currAmp = sweep.start/AMP_MUL;
        //maxV = sweep.end/AMP_MUL;
      }
      
      if(mode == SWM_DC)
      {   
        swCont.currDuty = sweep.start;
        if(swCont.currDuty < SMALL_VAL && sweep.log)
          swCont.currDuty = SMALL_VAL; // avoid undefined log values      
      }

      // set up the values for controlling pulse()    
      if(sweep.log)
        swCont.incVal = logMult(); // multiply   
      else
        swCont.incVal = (sweep.end - sweep.start) / (sweep.steps -1); // add
      //swCont.currAmp = maxV; 
      //swCont.currDuty = duty;
      //swCont.currFreq = freq;
    }
    duty = swCont.currDuty;
    swCont.incCount = 0;
    swCont.stepCount =  0;     
    restart = true;
    dds[chan].state = PU_LOW;   
    dds[chan].cyclesToGo = 0; 
    swCont.run = true;
    restart = true;
    //Serial.printf("-TR_ST- Inc %3.1f, ", swCont.incVal);
   //Serial.printf("-TR_ST- ch %i, type %i, C Hi %3.1f, V Low %3.1f, du %3.1f, fr %3.1f\n", chan, mode, swCont.currAmp, minV, swCont.currDuty, swCont.currFreq);
  }

  if(action == AC_SWEEP_NEXT) // TRI   
  {
    sweepMode_t mode = (sweepMode_t) ((int)sweepVal[iSet.waveForm[CHAN_A]].val[0][0]);    
    //if(swCont.done) return;
    if(chan == CHAN_A)
    {      
      if(mode == SWM_FR) 
      {
        if(sweep.log)
          swCont.currFreq *= swCont.incVal;
        else
          swCont.currFreq += swCont.incVal;
      }
      if(mode == SWM_AM) 
      {
        if(sweep.log)
          swCont.currAmp *= swCont.incVal;     
        else
          swCont.currAmp += swCont.incVal;   
        if(swCont.currAmp <  minV) 
          swCont.currAmp =  minV;
      }
      if(mode == SWM_DC)  
      {   
        if(sweep.log)
          swCont.currDuty *= swCont.incVal;
        else
          swCont.currDuty += swCont.incVal;
      }
    }
    //minV = swCont.currAmp;
    //maxV = swCont.currAmp;
    //freq = swCont.currFreq;
    duty = swCont.currDuty;
   
    if((duty < SMALL_VAL) && sweep.log)
        duty = SMALL_VAL; // avoid divide by 

    //Serial.printf(" -TR_SW_NE- ch %i, type %i, C Hi %3.1f, V Lo %3.2f, du %3.1f, fr %3.1f\n", chan, mode, swCont.currAmp, minV, duty, swCont.currFreq);
    if(BCOUPLE && !BURST_MODE) // phase calc needs to be reset
      restart = true;    
  }

  if (restart)
  {
    dds[chan].state = PU_RISE;
    dds[chan].samplesToGo = pulse[chan].riseSamps; 
    dds[chan].cyclesToGo = 0; 
  }
  // all modes: minVal does not change 
  pulse[chan].maxVal = voltsToCounts_Pulse(chan, swCont.currAmp);

  samps = sampleRate / swCont.currFreq;
  pulse[chan].minSamps  = pulse[chan].maxSamps = 0; 
  pulse[chan].riseSamps = (samps * duty / 100) -1;   // Max and min take a single sample time
  pulse[chan].fallSamps = samps - pulse[chan].riseSamps -2;

  pulse[chan].riseSampsStep  = pulse[chan].riseSamps / samps;
  pulse[chan].fallSampsStep  = pulse[chan].fallSamps / samps;

  int64_t diff = (int64_t)pulse[chan].maxVal - (int64_t)pulse[chan].minVal;
  if(pulse[chan].fallSamps > 0)
    pulse[chan].fallInc =  diff / pulse[chan].fallSamps;
  else 
    pulse[chan].fallInc =  diff; 

  if(pulse[chan].riseSamps > 0)
    pulse[chan].riseInc  = diff / pulse[chan].riseSamps;  
  else
    pulse[chan].riseInc  = diff;  

 #define RAT_TRIG 50  // use multi sample steps below this value - balance between slope granularity and step height
 #define RAT_MUL 50   // not necessarily the same value as RAT_TRIG 
  // riseSampsStep close to 1 produces substantial granularity in gradients
  if(abs(pulse[chan].riseInc) < RAT_TRIG)   // a step every few samples for moderately slow ramps
  { // ratiometric - increase delay between steps and increment
  //Serial.print(" -** RAT-R **- ");
    pulse[chan].riseSampsStep = RAT_MUL * pulse[chan].riseSamps / diff;
    pulse[chan].riseInc = diff * pulse[chan].riseSampsStep / pulse[chan].riseSamps;
    if(pulse[chan].riseInc < 1) // at least one count per step
    {
      pulse[chan].riseInc = 1;
    }
  }
  
  if(abs(pulse[chan].fallInc) < RAT_TRIG)   
  { 
//Serial.print(" -** RAT-F **- ");
    pulse[chan].fallSampsStep = RAT_MUL * pulse[chan].fallSamps / diff;
    pulse[chan].fallInc = diff * pulse[chan].riseSampsStep / pulse[chan].fallSamps;
    if(pulse[chan].fallInc < 1) 
    {
      pulse[chan].fallInc = 1;
    }
  }
  int totSamps = pulse[chan].fallSamps + pulse[chan].riseSamps + 2;// max and min take 1 each
  float effF = (float)sampleRate / totSamps;

    //Serial.printf("WA_TRI[%i]: effF [%3.2f], DU %3.1f\n", chan,  effF, duty);

   // Serial.printf("  -TRI_POST_RAT chan %i, inc %i[%i], F %i[%i], tot steps %i[%i]\n", chan, pulse[chan].riseInc, pulse[chan].riseSamps, pulse[chan].fallInc, pulse[chan].fallSamps, pulse[chan].riseSamps+ pulse[chan].fallSamps, pulse[chan].maxSamps + pulse[chan].minSamps);
    // Go!
    if(restart)
    {
      dds[chan].state = PU_LOW;
      dds[chan].samplesToGo = 0;
    }

  if((chan == 1) && (BCOUPLE && !BURST_MODE) && (BPHASE >= SMALL_VAL) )
  {     
    // phase depends on what channel A is currently up to (samps will be the same for both channels)    
    // for PU_LOW samps to go always == 0
    int aSamps  = (dds[0].state == PU_RISE || dds[0].state == PU_LOW) ?  totSamps - pulse[0].fallSamps - dds[0].samplesToGo : totSamps  - dds[0].samplesToGo;
    float aProp = ((float)aSamps)/ totSamps; // current A phase as 0..1
    float bProp = BPHASE/360.0;
    float dProp  = duty/100.0;
    int samps   = (BPHASE/360.00)*(totSamps);
    float xProp = aProp + bProp; // required b phase
    if (xProp > 1) 
      xProp = xProp -1;
   // Serial.printf(" -PHAS [ST %i] Aprop %1.3f aS  %3i (tS %3i [R %3i, F %3i]: DstG %3i) bP %1.3f, xP %1.3f, bS %3i, res %i", dds[0].state, aProp, aSamps, totSamps, pulse[0].riseSamps, pulse[0].fallSamps, dds[0].samplesToGo, bProp, xProp, samps, restart);
    // Serial.printf(" R0 %3i, H0 %3i, F0 %3i, L0 %3i [Rs %i]", pulse[0].riseSamps, pulse[0].maxSamps, pulse[0].fallSamps, pulse[0].minSamps, restart);
    if(xProp < dProp) // start on the leading egde of a cycle
    {
      dds[1].state = PU_RISE;     
      dds[1].samplesToGo =  pulse[0].riseSamps * xProp/dProp;
     // Serial.printf(" RISE %3i [%1.3f]\n", dds[1].samplesToGo, xProp);
    }
    else // falling 
    {
      dds[1].state = PU_FALL;
      dds[1].samplesToGo = pulse[0].fallSamps * (xProp - dProp)/(1-dProp);
      //Serial.printf(" FALL %3i [%1.3f]\n", dds[1].samplesToGo, xProp - dProp); 
    }      
    //Serial.printf("BU TRI du %3.2f, prop %3.2f, stg %i, riseS %i, fallS %i\n", du, prop, dds[chan].samplesToGo, pulse[chan].riseSamps, pulse[chan].fallSamps);
  }
  //Serial.printf(" -TRI tot samps %i: %i\n",chan, pulse[chan].fallSamps + pulse[chan].riseSamps);
} // end TRIANGLE


/////////////////////// STEP //////////////////////////
void setStep(int chan, dds_actions action, bool restart)
{  
  //int tfc, v2c,ssc;
  //float tf, jumpV;

  dds[chan].waveForm = WA_STEP; 
  dds[chan].cyclesToGo = 0; // infinite (debug) 

  int sCh = chan;
  if(chan == CHAN_B && ((BCOUPLE && !BURST_MODE) || (BURST_MODE && BALTA)))
    sCh = CHAN_A;
  if(chan == CHAN_B && BCOUPLE && !BURST_MODE)
    dds[1].run = dds[0].run;

  pulse[chan].minVal    = voltsToCounts_Pulse(sCh, modeVal[sCh][WA_STEP][ST_V1]/AMP_MUL);
  pulse[chan].maxVal    = voltsToCounts_Pulse(sCh, modeVal[sCh][WA_STEP][ST_V2]/AMP_MUL);
  idleVal[chan]         = pulse[sCh].minVal;
  pulse[chan].maxSamps  = 0;        
  pulse[chan].minSamps  = 0;
  pulse[chan].riseSamps = msToSamples(modeVal[sCh][WA_STEP][ST_TU]);
  pulse[chan].fallSamps = msToSamples(modeVal[sCh][WA_STEP][ST_TD]);
  pulse[chan].riseSteps = modeVal[sCh][WA_STEP][ST_SU] +0.5; // round float val
  pulse[chan].fallSteps = modeVal[sCh][WA_STEP][ST_SD] +0.5;
  pulse[chan].riseSampsStep = pulse[chan].riseSamps / (pulse[chan].riseSteps); // diff can exceed 32 int max
  pulse[chan].fallSampsStep = pulse[chan].fallSamps / (pulse[chan].fallSteps);

  int64_t gap = (int64_t)pulse[chan].maxVal - (int64_t)pulse[chan].minVal;
  pulse[chan].riseInc = gap / (pulse[chan].riseSteps);
  pulse[chan].fallInc = gap / (pulse[chan].fallSteps);

 // Serial.printf("WA_STEP[%i], Min %i [%3.3f], ", chan, pulse[chan].minVal, modeVal[chan][WA_STEP][ST_V1]);
 // Serial.printf("Max %i [%3.3f], gap %i [%lli] (%lli)", pulse[chan].maxVal, modeVal[chan][WA_STEP][ST_V2],pulse[chan].maxVal - pulse[chan].minVal, (int64_t)pulse[chan].maxVal - (int64_t)pulse[chan].minVal, gap);
 //  Serial.printf("RAT %1.3f\n", ((float)pulse[chan].maxVal)/maxDAC);
 // Serial.printf(" TR_Inc %lli [%3.3f], steps %i, r samps %i[s/s %i] \n", pulse[chan].riseInc,  modeVal[chan][WA_STEP][ST_TU], pulse[chan].riseSteps, pulse[chan].riseSamps, pulse[chan].riseSampsStep);
 // Serial.printf(" TF_Inc %lli [%3.3f], steps %i, f samps %i[s/s %i] \n", pulse[chan].fallInc,  modeVal[chan][WA_STEP][ST_TD], pulse[chan].fallSteps, pulse[chan].fallSamps, pulse[chan].fallSampsStep);
  
  burst.state = B_ACTIVE;
  // Go!
  if(restart)
  {
    dds[chan].state = PU_LOW;
    dds[chan].samplesToGo = 0;
  }
   dds[chan].state = PU_LOW; // remove???
} // end STEP

// White noise conversion
int voltsToCounts_White(int chan, float volts)
{
  return maxWhiteVal * (volts / vCal[chan]); // overflow possible for volts/vCal > 1
}

void setWhiteFunc(int ch)
{
  int sCh = ch;
  dds[ch].waveForm = WA_WHITE;

  if(BCOUPLE && ch == CHAN_B)
    sCh = CHAN_A;
  // WHITE_MUL: digital filter overshoots on white noise
  // White random is always 16 bit
  if(modeVal[sCh][WA_WHITE][WH_AM] + abs(modeVal[sCh][WA_WHITE][WH_DC]) > MAXAC)
  {
    modeVal[sCh][WA_WHITE][WH_AM] = MAXAC - abs(modeVal[sCh][WA_WHITE][WH_DC]);
    valChange(VAL_CHGD_LOCAL);  
  }
  white[ch].range = voltsToCounts_White(ch, modeVal[sCh][WA_WHITE][WH_AM] * WHITE_MUL / AMP_MUL);
  // offset is 16 or 32 bit
  idleVal[ch] = white[ch].offset = voltsToCounts_White(ch, modeVal[sCh][WA_WHITE][WH_DC] / AMP_MUL) << DDSWHITESHIFT;
#if SAMPWIDTH == 16
  white[ch].shift = 16; 
#else
  white[ch].shift = 16; // calcs always in 16 bit
#endif
  //Serial.printf("WHITE ra %i [off %i] sh %i\n", white[ch].range, white[ch].offset, white[ch].shift);
}

// set new parameters, but don't change state machine / DDS operation 
// sets basic mode OR sweep/burst etc.
// call twice for burst OR sweep - waveForm forst, then sweep.
void setFunc(int chan, dds_actions action, bool restart)
{  
  DDSwaves func = iSet.waveForm[chan];

  if(action == AC_NORMAL || action == AC_IDLE)
    digitalWrite(TRIG_OUTPIN, !TRIG_OUT_POL);
  switch (func) // set up wave generation and availability of sweep/burst/triggers
  {
    case WA_SINE :
      setSine(chan, action, restart);
      if(((BCOUPLE && !BURST_MODE) || (BALTA && BURST_MODE)) && chan == 0) 
        setSine(1, action, restart);
      burstable = true;
      sweepable = true;
      trigOutOn = true;
      break;

     case WA_SQR :   
      setSqr(chan, action, restart);
      if(((BCOUPLE && !BURST_MODE) || (BALTA && BURST_MODE)) && chan == 0)  
        setSqr(1, action, restart);
      burstable = true;
      sweepable = true;
      trigOutOn = true;
      break;

     case WA_TRI :  
      setTri(chan, action, restart);
      if(((BCOUPLE && !BURST_MODE) || (BALTA && BURST_MODE)) && chan == 0)  
        setTri(1, action, restart);
      burstable = true;
      sweepable = true;
      trigOutOn = true;
      break;

    case WA_STEP :
      setStep(chan, action, restart);
      burstable = true;
      sweepable = true;
      trigOutOn = true;
      break;

     case WA_PULSE :
      setPulse(chan, action, restart);
      burstable = true;
      sweepable = false;
      break;

    case WA_IMD :
      if(chan == 0) 
        setIntermod(); // A only
      burstable = false;
      sweepable = false;
      trigOutOn = false;
      trigOut(TRIG_IDLE, chan);  
      break;

    case WA_WHITE :
      setWhiteFunc(chan); 
      burstable = false;
      sweepable = false;
      trigOutOn = false;
      trigOut(TRIG_IDLE, chan);  
      break;

    case WA_BURST : // do nothing      
      break;

    default :
      dds[chan].waveForm = WA_SINE; 
      break;     
  }
}
/*
void dumpBuffer_i()
{
    for(int i = 0; i < 20; i++)
      Serial.printf("%03i: %06i\n",i, DDStab_i[i]);
      Serial.printf("Peak %06i\n", DDStab_i[DDS_TAB_LEN/4]);
   for(int i = DDS_TAB_LEN - 20; i < DDS_TAB_LEN; i++)
      Serial.printf("%03i: %06i\n", i, DDStab_i[i]);
}

void mute(bool muted)
{
  if(muted)
    digitalWrite(MUTEPIN, LOW);
  else
    digitalWrite(MUTEPIN, HIGH);
}
*/
// calculate multiplier for log sweeps
// initial value depends on waveform; final value & steps are in sweep[].
float logMult(void)
{
  // avoid calculating log(0)
  //if(sweep.start == sweep.end) return 1;

  float m0 = sweep.start;
  if (m0 < SMALL_VAL)
    m0 = SMALL_VAL;
  float logMin = log10(m0);

  float m1 = sweep.end;
  if (m1 < SMALL_VAL)
    m1 = SMALL_VAL;
  float logMax = log10(m1);

  float logDiff = logMax - logMin;
  float mul = 1 + (pow(10, logDiff / (sweep.steps - 1) + logMin)- m0) / m0; 
  //Serial.printf("LogMul %1.3f\n", mul);
  return mul;
}
#endif