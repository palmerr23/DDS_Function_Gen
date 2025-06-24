/* DDS code 
 setup done in DDset.h
 see also DDSdefs.h

  *** NO CALLS TO PRINT IN THIS FILE (causes breaks in generation) ***
*/
#ifndef DDS_H
#define DDS_H

#include "fastPRNG.h" // faster random
fastPRNG::fastXS32 fastS;
#define FASTRAND

inline float DDSsineLookup(float stepSize);
inline void trigOut(int val, int chan);
void setRate(uint32_t sRate);

// time in milliseconds
inline float msToSamples(float ms)
{
	return (float)sampleRate * ms / 1000.0;
}
inline float samplesToMs(float samples)
{
	return 1000.0 * samples / sampleRate ;
}
// time in seconds
inline int sToSamples(float secs)
{
	return sampleRate * secs;
}
inline float samplesToSecs(float samples)
{
	return samples / sampleRate ;
}

// fill the DDS reference table
// one full cycle, signed binary
// sin() is <double>
void initDDSsine(void)
{
    for(int i = 0; i < DDS_TAB_LEN; i++)
        DDStab_i[i] = maxVal * sin(i * 2.0 * PI / DDS_TAB_LEN); 
}

void stopDDS(int side)
{
    dds[side].run = false;
    // iSet.run[side] = false;
}
bool show = false;
// 32 bit accumulator
// DDS_LOOKUP_BITS in table
// rest are linearly interpolated
// for 4096 table, an increment of 1 << 20 will create a 96k / 4096 signal (i.e 24kHz)
// for 32 bit data dx * dy can get large, so limit dx to a smaller number of bits. 

int DDSsineLookup_i(uint32_t phase)
{
  // mask bucket before shifting if using DDS_PHASE_BITS < 32
  int bucket = (phase & DDS_PHASE_MASK) >> DDS_INTER_BITS;  // mask above the bucket + iterpolation bits and scale down to total number of buckets
  int dx = phase & DDS_INTER_MASK; // mask off bucket part to find the remainder
  //if(show)   Serial.printf("st %8i, b %4i.%4i, ", phase, bucket, dx);

  // more error checking required?
   bucket = bucket % DDS_TAB_LEN; // may not be required, as masked above
 
  if(dx == 0) //return bucket val, if no interp needed
    return DDStab_i[bucket];
    
  int dy; 
  if(bucket != (DDS_TAB_LEN - 1))
    dy = DDStab_i[bucket + 1] - DDStab_i[bucket];
  else 
    dy = DDStab_i[0] - DDStab_i[DDS_TAB_LEN - 1]; // last bucket wrap around 

  //int delta = ((dy * dx) / (1 << DDS_INTER_BITS));
  int delta = (dy * dx) >> DDS_INTER_BITS;
  int val = DDStab_i[bucket] + delta ; // normalise dy*dx then add
   //if (show) Serial.printf(" dy %9i, b[i] %12i +  delta %9i = %12i\n", dy, DDStab_i[bucket], delta, val); 
  return val;
}


inline int shiftIt(int val)
{
#if(SAMPWIDTH == 16)
  return val >> DDSRSHIFT; // shift down to appropriate output level: 16 bits
#else
  return val << DDSLSHIFT; // shift up to appropriate output level: 32 bits
#endif
}

int DDSsine(int chan)
{
  int sinVal32;
/*
  if(bPhaseA && chan == 1)
    sinVal32 = DDSsineLookup_i(sinval[0].phAcc + phOffset); // always 32 bit maxVal
  else
    sinVal32 = DDSsineLookup_i(sinval[chan].phAcc);
    */

  int xChan = chan;
  // ch B only if not coupled or phase degrees is set
  if(1 || chan == 0 || !BCOUPLE || BPHASE > 0) 
  {
    sinVal32 = DDSsineLookup_i(sinval[chan].phAcc);
    sinval[chan].phAcc += sinval[chan].inc; // let it overflow
    sinval[chan].calcVal = sinVal32;  // ch 0: save for linked vals
  }
  else // redundant
  {
    sinVal32 = (BPHASE == 0) ? sinval[0].calcVal : -sinval[0].calcVal;
    if(BCOUPLE)
      xChan = 0; // apply offset and scaling from channel A
  }
  if(!dds[chan].run)
    return shiftIt(sinval[xChan].dcOff);

  // scale for amplitude and add DCoffset 
  sinVal32 = sinVal32 * sinval[xChan].scale; // scaled up value
  sinVal32 = (sinVal32 >> INTSCALE_BITS);
  sinVal32 += sinval[xChan].dcOff; // scaled up value
  if(chan == CHAN_A)
    trigOut(sinVal32, chan); // only trigger for correct channel
  //if(show)  Serial.printf("svf %10i, %10i\n", sinVal32, shiftIt(sinVal32));
  sinVal32 = constrain(sinVal32, -maxVal, maxVal);
  return shiftIt(sinVal32); // shift up/down to appropriate output (16 or 32 bits)
}

int pulseCnt = 0;
int pulseState;
int sampsQR = 0, stepSizQR = 0, stepDelQR = 0, sampsQF = 0, stepSizQF = 0, stepDelQF = 0;  // debug only
// arithmetic is mostly integer in counts, not volts or seconds, for speed.
// for all sample counts: value of 0 is immediate change
// TRI, SQR and STEP also use PULSE DDS
int DDSpulse(int chan)
{
  if(!dds[chan].run)
    return pulse[chan].minVal;
  int val, temp;
  if(chan == CHAN_B && (BCOUPLE && !BURST_MODE)) // B = A
  {
    // pulse and step invert
    if((WAVE_A == WA_PULSE || WAVE_A == WA_STEP) && BPHASE >= 0.01)
    {
      dds[1].curVal = pulse[0].maxVal + pulse[0].minVal - dds[0].curVal;
      return dds[1].curVal;
    }
    // pulse and step, or all waves and no pahse shift
    if(WAVE_A == WA_PULSE || WAVE_A == WA_STEP || BPHASE < 0.01)
    {
      dds[1].curVal = dds[0].curVal;
      return dds[1].curVal;
    }
    

  }
  //normal operation  
  switch (dds[chan].state)
    {
      case PU_IDLE : // will never leave this state unless state externally changed 
        dds[chan].curVal = pulse[chan].minVal;
        break;

      case PU_LOW : // externally initiated transition to this state
        dds[chan].curVal = pulse[chan].minVal; // need set this on entry				
        if (dds[chan].samplesToGo <= 0) // samplesToGo set by FALL or initiator
        {
          if(pulse[chan].riseSamps > 0) // next stage has steps
          {
            dds[chan].state = PU_RISE;
            // rise can be ratiometric for slow triangles / pulses
            sampsQR = dds[chan].samplesToGo = pulse[chan].riseSamps;
            stepSizQR = dds[chan].stepSize =  pulse[chan].riseInc;	
            stepDelQR = dds[chan].delaySteps =  pulse[chan].riseSampsStep;
            //digitalWrite(TRIG_OUTPIN, HIGH);
            trigOut(TRIG_ACTIVE, chan);    
          }
          else // straight to HIGH
          {
              dds[chan].state = PU_HIGH;
              dds[chan].samplesToGo = pulse[chan].maxSamps;		
              if(chan == CHAN_A && iSet.waveForm[chan] == WA_SQR  && _sqrPreshoot)
                  dds[chan].curVal = pulse[chan].minVal + pulse[chan].riseInc >> _sqrPreshoot;  // 12% pre-step to stop ringing
              trigOut(TRIG_ACTIVE, chan);  
          }   
        }
        else
        {
          dds[chan].samplesToGo--;
          dds[chan].delaySteps--; 
        }
        break;

      case PU_RISE:
          if(dds[chan].delaySteps <= 0) // slow ramps only increment occasionally
          {
            dds[chan].curVal += dds[chan].stepSize;	
            dds[chan].delaySteps = pulse[chan].riseSampsStep;
          }			  
          if (dds[chan].samplesToGo <= 0) // samples done - better timing
          //if (dds[chan].curVal >= pulse[chan].maxVal) // reached value - nicer ramp
          {   
            if(pulse[chan].maxSamps > 0) 	
            {					
              dds[chan].state = PU_HIGH;
              dds[chan].samplesToGo = pulse[chan].maxSamps;						
              
            }
            else // straight to FALL
            {
              dds[chan].state = PU_FALL;
              // fall can be ratiometric for slow triangles / pulses
              if(iSet.waveForm[chan] == WA_TRI)
                dds[chan].curVal = pulse[chan].maxVal;  // skipped stage, so re-establish baseline                
              sampsQF = dds[chan].samplesToGo  = pulse[chan].fallSamps;
              stepSizQF = dds[chan].stepSize   = pulse[chan].fallInc; 
              stepDelQF = dds[chan].delaySteps = pulse[chan].fallSampsStep; 
              //digitalWrite(TRIG_OUTPIN, LOW);
              trigOut(TRIG_IDLE, chan);  
            }           
          }
            else
          {
            dds[chan].samplesToGo--;
            dds[chan].delaySteps--; 
          }
          break;

      case PU_HIGH:
        dds[chan].curVal = pulse[chan].maxVal;  
        if (dds[chan].samplesToGo <= 0) // samplesToGo set by FALL or initiator
        {        
          if(pulse[chan].fallSamps > 0)
          {
            dds[chan].state = PU_FALL;
            // fall can be ratiometric for slow triangles / pulses
            sampsQF = dds[chan].samplesToGo = pulse[chan].fallSamps;
            stepSizQF = dds[chan].stepSize =  pulse[chan].fallInc; 
            stepDelQF = dds[chan].delaySteps = pulse[chan].fallSampsStep; 
            //digitalWrite(TRIG_OUTPIN, LOW);
            trigOut(TRIG_IDLE, chan);  
          }
          else // straight to LOW
          {
            dds[chan].state = PU_LOW;
            //dds[chan].curVal = pulse[chan].minVal;       
            if(chan == CHAN_A && iSet.waveForm[chan] == WA_SQR && _sqrPreshoot)
                dds[chan].curVal = pulse[chan].maxVal - pulse[chan].fallInc >> _sqrPreshoot;  // 12% pre-step to stop ringing               
            dds[chan].samplesToGo = pulse[chan].minSamps;
            dds[chan].cyclesToGo--;   // completed a cycle 
            trigOut(TRIG_IDLE, chan); 
          }
        }
        else
        {
          dds[chan].samplesToGo--;
          dds[chan].delaySteps--; 
        }
        break;

      case PU_FALL:
        if(dds[chan].delaySteps <= 0)
        {
          dds[chan].curVal -= dds[chan].stepSize;	
          dds[chan].delaySteps = pulse[chan].fallSampsStep;
        }          
        if (dds[chan].samplesToGo <= 0)
        //if (dds[chan].curVal <= pulse[chan].minVal)
        {
          if(pulse[chan].minSamps > 0)	
          {					
            dds[chan].state = PU_LOW;
            dds[chan].curVal = pulse[chan].minVal;                      
            dds[chan].samplesToGo = pulse[chan].minSamps;
            dds[chan].cyclesToGo--;   // completed a cycle    
          }
          else // straight to RISE
          {
            dds[chan].state = PU_RISE;
            
            if(iSet.waveForm[chan] == WA_TRI)
              dds[chan].curVal = pulse[chan].minVal; // skipped stage, so re-establish baseline for triangle                
            // rise can be ratiometric for slow triangles / pulses
            sampsQR = dds[chan].samplesToGo = pulse[chan].riseSamps;
            stepSizQR = dds[chan].stepSize =  pulse[chan].riseInc;	
            stepDelQR = dds[chan].delaySteps =  pulse[chan].riseSampsStep;
            //digitalWrite(TRIG_OUTPIN, HIGH);
            trigOut(TRIG_ACTIVE, chan); 
          }           
        }
        else
        {
          dds[chan].samplesToGo--;
          dds[chan].delaySteps--; 
        }
        break;
  }// switch  
  return constrain(dds[chan].curVal, -maxDAC, maxDAC);
}

int whiteCount = 0;
int DDSwhite(int ch)
{   
  int val; 
  if(!dds[ch].run)
    return  white[ch].offset;
#ifndef FASTRAND
  val =  random(pulse[ch].minVal, pulse[ch].maxVal); // do not use - too slow
#else
  int32_t res = (int32_t)fastS.xoroshiro64x();  
  val =  (((res >> 16) * white[ch].range)) + white[ch].offset;
#endif  

  if(ch == 0) 
    if(whiteCount++ > (sampleRate >> 10)) // ~10mS ZC flag for burst
    {
      whiteCount = 0;
      ZCflag = true;      
    }
  return val;
}

// chan A only
int imdCount = 0;
#define NPRINT 5
int DDSintermod()
{
  int val  = DDSsineLookup_i(sinval[0].phAcc) * sinval[0].scale;
  int val2 = DDSsineLookup_i(sinval[1].phAcc) * sinval[1].scale;
  val = val >> INTSCALE_BITS; // ensure within safe int32 range before adding
  val2 = val2 >> INTSCALE_BITS;
  show = false;
  sinval[0].phAcc += sinval[0].inc; // let it overflow
  sinval[1].phAcc += sinval[1].inc;

  if(imdCount++ == sampleRate/1000) // 1mS indicator for burst
  {
    imdCount = 0;
    ZCflag = true;      
  }
  if(!dds[0].run)
    return shiftIt(idleVal[0]);

  int tot = val + val2;
  tot = constrain(tot, -maxVal, maxVal);
  return shiftIt(tot); 
}

// trig only for Chan A
int lastTrigVal;  // for zero crossingLevels

// inline function to avoid context shift delay.
// val is the calculated value for sine, or the trigger state for pulse waveforms
// waveform version. Burst and Sweep are done in mainline code.
inline void trigOut(int val, int chan)
{
  trigOutActive = TRIG_DISABLE;
  if(chan != 0 || !trigOutOn)
    return;
    
  switch (iSet.waveForm[chan])
  {
    case WA_SINE :     
      if((lastTrigVal <= crossingLevel[chan]) && (val > crossingLevel[chan])) // upward
      { 
        trigOutActive = TRIG_ACTIVE;
      }
      if((lastTrigVal > crossingLevel[chan]) && (val <= crossingLevel[chan]))  // downward
        trigOutActive = TRIG_IDLE;
        break;
    //case WA_TRI : // uses PULSE
    default : // PULSE
      trigOutActive = val;
  }

  if(trigOutActive == TRIG_ACTIVE)
  {
    if(iSet.modeA == MO_NOR) // SWEEP & BURST trig output code is in main loop
      digitalWrite(TRIG_OUTPIN, LOW ^ (int)TRIG_OUT_POL);
    if(iSet.waveForm[chan] != WA_SINE) 
      ZCflag = true;
  }
    
  if(trigOutActive == TRIG_IDLE)
  {
    if(iSet.modeA == MO_NOR)
      digitalWrite(TRIG_OUTPIN, HIGH ^ (int)TRIG_OUT_POL);
    if(iSet.waveForm[chan] == WA_SINE) 
      ZCflag = true;
  }
  lastTrigVal = val;
}
#endif