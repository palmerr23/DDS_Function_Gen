/* UDP and Serial SCPI */
#ifndef _UDPCSPI_H
#define _UDPCSPI_H

void regCmds();

#define SCPI_MAX_TOKENS 60 //Default value = 15
#define SCPI_ARRAY_SYZE 6 //Default value = 6
#define SCPI_MAX_COMMANDS 110 //Default value = 20
#define SCPI_MAX_SPECIAL_COMMANDS 0 //Default value = 0
#define SCPI_BUFFER_LENGTH 128 //Default value = 64
#define SCPI_HASH_TYPE uint16_t //Default value = uint8_t
#include "Vrekrer_scpi_parser.h"

SCPI_Parser ddSCPI;
#include "SCPIcmds.h"

void SCPIstart()
{
  ddSCPI.hash_magic_number = 137; //Default value = 37
  ddSCPI.hash_magic_offset = 23;  //Default value = 7
  ddSCPI.timeout = 10; //value in miliseconds. Default value = 10
  regCmds();
  //ddSCPI.PrintDebugInfo(Serial);
}

void regCmds()
{
    ddSCPI.SetCommandTreeBase(F(""));
      ddSCPI.RegisterCommand(F("*IDN?"), &Identify);
      ddSCPI.RegisterCommand(F("*TST?"), &getPost);
    ddSCPI.SetCommandTreeBase(F(":SYSTEM"));
      ddSCPI.RegisterCommand(F(":ERROR?"), &getError); 
      ddSCPI.RegisterCommand(F(":SSID"), &setSSID); 
      ddSCPI.RegisterCommand(F(":PASS"), &setPass); 
      ddSCPI.RegisterCommand(F(":WIFI"), &setWiFi); 
      ddSCPI.RegisterCommand(F(":IP?"),  &getIP);
      ddSCPI.RegisterCommand(F(":FACTORY_RESET"), &setFactReset); 

    ddSCPI.SetCommandTreeBase(F(":SWEEP"));
      ddSCPI.RegisterCommand(F(":ENABLE"),   &setSweepEnable); // START/STOP
      ddSCPI.RegisterCommand(F(":START"),    &setSweepStart);
      ddSCPI.RegisterCommand(F(":STOP"),     &setSweepStop);
      ddSCPI.RegisterCommand(F(":TYPE"),     &setSweepType);
      ddSCPI.RegisterCommand(F(":INITIAL"),  &setSweepInit);
      ddSCPI.RegisterCommand(F(":FINAL"),    &setSweepFinal);
      ddSCPI.RegisterCommand(F(":STEPS"),    &setSweepSteps);
      ddSCPI.RegisterCommand(F(":TIME"),     &setSweepTime);
      ddSCPI.RegisterCommand(F(":LOG"),      &setSweepLog);
      ddSCPI.RegisterCommand(F(":REPEAT"),   &setSweepRep);

      ddSCPI.RegisterCommand(F(":ENABLE?"),  &getSweepEnable);
      ddSCPI.RegisterCommand(F(":TYPE?"),    &getSweepType);
      ddSCPI.RegisterCommand(F(":INITIAL?"), &getSweepInit);
      ddSCPI.RegisterCommand(F(":FINAL?"),   &getSweepFinal);
      ddSCPI.RegisterCommand(F(":STEPS?"),   &getSweepSteps);
      ddSCPI.RegisterCommand(F(":TIME?"),    &getSweepTime);
      ddSCPI.RegisterCommand(F(":LOG?"),     &getSweepLog);
      ddSCPI.RegisterCommand(F(":REPEAT?"),  &getSweepRep);

ddSCPI.SetCommandTreeBase(F(":BURST"));
      ddSCPI.RegisterCommand(F(":ENABLE"),  &setBurstEnable);
      ddSCPI.RegisterCommand(F(":START"),   &setBurstStart);
      ddSCPI.RegisterCommand(F(":STOP"),    &setBurstStop);
      ddSCPI.RegisterCommand(F(":CYCSON"),   &setBurstCycon);
      ddSCPI.RegisterCommand(F(":CYCSOFF"),  &setBurstCycoff); 
      ddSCPI.RegisterCommand(F(":BALTA"),    &setBurstBaltA);      
      ddSCPI.RegisterCommand(F(":REPEAT"),   &setBurstRepeat);

      ddSCPI.RegisterCommand(F(":ENABLE?"), &getBurstEnable);
      ddSCPI.RegisterCommand(F(":CYCSON?"),  &getBurstCycsOn);
      ddSCPI.RegisterCommand(F(":CYCSOFF?"), &getBurstCycsOff);      
      ddSCPI.RegisterCommand(F(":BALTA?"),   &getBurstBaltA);
      ddSCPI.RegisterCommand(F(":REPEAT?"),  &getBurstRepeat);

ddSCPI.SetCommandTreeBase(F(":CONT"));
      ddSCPI.RegisterCommand(F(":BCOUPLE"), &setContCpl);
      ddSCPI.RegisterCommand(F(":BPHASE"),  &setContPh);
      ddSCPI.RegisterCommand(F(":EXT"),     &setContExtTrig);
      ddSCPI.RegisterCommand(F(":INPOL"),   &setContInPol);
      ddSCPI.RegisterCommand(F(":OUTPOL"),  &setContOutPol);

      ddSCPI.RegisterCommand(F(":BCOUPLE?"), &getContCpl);
      ddSCPI.RegisterCommand(F(":BPHASE?"),  &getContPh);
      ddSCPI.RegisterCommand(F(":EXT?"),     &getContExtTrig);
      ddSCPI.RegisterCommand(F(":INPOL?"),   &getContInPol);
      ddSCPI.RegisterCommand(F(":OUTPOL?"),  &getContOutPol);

ddSCPI.SetCommandTreeBase(F(":SOURCE"));
      ddSCPI.RegisterCommand(F(":SELECT"),  &setChanSel);

      ddSCPI.RegisterCommand(F(":START"),   &setChanStart);
      ddSCPI.RegisterCommand(F(":STOP"),    &setChanStop);
      //ddSCPI.RegisterCommand(F(":ENABLE"),  &setChanEnable);

      ddSCPI.RegisterCommand(F(":WAVE"),    &setWave);
      //ddSCPI.RegisterCommand(F(":ENABLE?"), &getChanEnable);
      ddSCPI.RegisterCommand(F(":SELECT?"), &getChanSel);
      ddSCPI.RegisterCommand(F(":WAVE?"),   &getWave);
      
ddSCPI.SetCommandTreeBase(F(":SINE"));
      ddSCPI.RegisterCommand(F(":FREQ"),   &setSinFreq);
      ddSCPI.RegisterCommand(F(":AMP"),    &setSinAmp);
      ddSCPI.RegisterCommand(F(":DCOFF"),  &setSinDC);
      ddSCPI.RegisterCommand(F(":CORR"),  &setCor);   // not published

      ddSCPI.RegisterCommand(F(":AMP?"),   &getSinAmp);
      ddSCPI.RegisterCommand(F(":FREQ?"),  &getSinFreq);
      ddSCPI.RegisterCommand(F(":DCOFF?"), &getSinDC);
ddSCPI.SetCommandTreeBase(F(":SQR"));
      ddSCPI.RegisterCommand(F(":FREQ"),  &setSqrFreq);
      ddSCPI.RegisterCommand(F(":V#"),    &setSqrVolts);
      ddSCPI.RegisterCommand(F(":DUTY"),  &setSqrDuty);
      ddSCPI.RegisterCommand(F(":COMP"),  &setPreshoot); // not published

      ddSCPI.RegisterCommand(F(":FREQ?"), &getSqrFreq);
      ddSCPI.RegisterCommand(F(":V#?"),   &getSqrVolts);
      ddSCPI.RegisterCommand(F(":DUTY?"), &getSqrDuty);
ddSCPI.SetCommandTreeBase(F(":TRI"));
      ddSCPI.RegisterCommand(F(":FREQ"),  &setTriFreq);
      ddSCPI.RegisterCommand(F(":V#"),    &setTriVolts);
      ddSCPI.RegisterCommand(F(":DUTY"),  &setTriDuty);

      ddSCPI.RegisterCommand(F(":FREQ?"), &getTriFreq);
      ddSCPI.RegisterCommand(F(":V#?"),   &getTriVolts);
      ddSCPI.RegisterCommand(F(":DUTY?"), &getTriDuty);
ddSCPI.SetCommandTreeBase(F(":IMD"));
      ddSCPI.RegisterCommand(F(":AMP#"),   &setImdAmp);
      ddSCPI.RegisterCommand(F(":FREQ#"),  &setImdFreq);
      ddSCPI.RegisterCommand(F(":AMP#?"),  &getImdAmp);
      ddSCPI.RegisterCommand(F(":FREQ#?"), &getImdFreq);
ddSCPI.SetCommandTreeBase(F(":WHITE"));
      ddSCPI.RegisterCommand(F(":AMP"),    &setWhiteAmp);
      ddSCPI.RegisterCommand(F(":DCOFF"),  &setWhiteDC);
      ddSCPI.RegisterCommand(F(":AMP?"),   &getWhiteAmp);
      ddSCPI.RegisterCommand(F(":DCOFF?"), &getWhiteDC);
ddSCPI.SetCommandTreeBase(F(":PULSE"));
      ddSCPI.RegisterCommand(F(":V#"),     &setPulVolts);
      ddSCPI.RegisterCommand(F(":TLOW"),   &setPulTlo);
      ddSCPI.RegisterCommand(F(":TRISE"),  &setPulTri);
      ddSCPI.RegisterCommand(F(":THIGH"),  &setPulThi);
      ddSCPI.RegisterCommand(F(":TFALL"),  &setPulTfa);

      ddSCPI.RegisterCommand(F(":V#?"),    &getPulVolts);
      ddSCPI.RegisterCommand(F(":TLOW?"),  &getPulTlo);
      ddSCPI.RegisterCommand(F(":TRISE?"), &getPulTri);
      ddSCPI.RegisterCommand(F(":THIGH?"), &getPulThi);
      ddSCPI.RegisterCommand(F(":TFALL?"), &getPulTfa);
ddSCPI.SetCommandTreeBase(F(":STEP"));
      ddSCPI.RegisterCommand(F(":V#"),     &setStepVolts);
      ddSCPI.RegisterCommand(F(":SUP"),    &setStepSup);
      ddSCPI.RegisterCommand(F(":SDN"),    &setStepSdn);
      ddSCPI.RegisterCommand(F(":TUP"),    &setStepTup);
      ddSCPI.RegisterCommand(F(":TDN"),    &setStepTdn);
      ddSCPI.RegisterCommand(F(":REPEAT"), &setStepRepeat);

      ddSCPI.RegisterCommand(F(":V#?"),     &getStepVolts);
      ddSCPI.RegisterCommand(F(":SUP?"),    &getStepSup);
      ddSCPI.RegisterCommand(F(":SDN?"),    &getStepSdn);
      ddSCPI.RegisterCommand(F(":TUP?"),    &getStepTup);
      ddSCPI.RegisterCommand(F(":TDN?"),    &getStepTdn);
      ddSCPI.RegisterCommand(F(":REPEAT?"), &getStepRepeat);
}

void SCPIexec()
{
    ddSCPI.ProcessInput(Serial, "\n");
    if(comms.enabled)
      ddSCPI.ProcessInput(TelnetStream, "\n");
}

void getLastSCPIerror(Stream& interface)
{
    switch(ddSCPI.last_error){
    case ddSCPI.ErrorCode::BufferOverflow: 
      interface.println(F("Buffer overflow error"));
      break;
    case ddSCPI.ErrorCode::Timeout:
      interface.println(F("Communication timeout error"));
      break;
    case ddSCPI.ErrorCode::UnknownCommand:
      interface.println(F("Unknown command received"));
      break;
    case ddSCPI.ErrorCode::NoError:
      interface.println(F("No Error"));
      break;
  }
  ddSCPI.last_error = ddSCPI.ErrorCode::NoError;
}

void GetLastEror(SCPI_C commands, SCPI_P parameters, Stream& interface) {
  getLastSCPIerror(interface);
}

void myErrorHandler(SCPI_C commands, SCPI_P parameters, Stream& interface) {
  Serial.printf("SCPI Error \n");
  GetLastEror(commands, parameters, interface);
  //This function is called every time an error occurs

  /* The error type is stored in ddSCPI.last_error
     Possible errors are:
       SCPI_Parser::ErrorCode::NoError
       SCPI_Parser::ErrorCode::UnknownCommand
       SCPI_Parser::ErrorCode::Timeout
       SCPI_Parser::ErrorCode::BufferOverflow
  */

  /* For BufferOverflow errors, the rest of the message, still in the interface
  buffer or not yet received, will be processed later and probably 
  trigger another kind of error.
  Here we flush the incomming message*/
  if (ddSCPI.last_error == SCPI_Parser::ErrorCode::BufferOverflow) {
    delay(2);
    while (interface.available()) {
      delay(2);
      interface.read();
    }
  }

  /*
  For UnknownCommand errors, you can get the received unknown command and
  parameters from the commands and parameters variables.
  */
}
#endif