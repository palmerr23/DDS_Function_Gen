// web server & SPIFFS routines
#ifndef MYSWEB_H
#define MYSWEB_H

#define INCHANS 1 // not sure we need this
//#include "myLInst.h"
//#include "myLoad.h"
//#include "FS.h"
#include "LittleFS.h"
//#include <AsyncTCP_RP2040W.h>
#include <AsyncWebServer_RP2040W.h>
// Create AsyncWebServer object on port 80
AsyncWebServer server(80);
#include "utilityPages.h"
#include "utilityServer.h"
//#include "my_mx_defs.h"
char JSONstring[4096];

#include "penguin_ico.h"

void slashRequest(AsyncWebServerRequest *request, int client);
void notFound(AsyncWebServerRequest *request);
void webCommands(AsyncWebServerRequest *request);
void listDir(fs::FS &fs, const char * dirname, uint8_t levels);
void makeJSONreads(char screen, int client);
void makeJSONsets(char screen, int client);
void addSet(int chan);
void printParamsX(AsyncWebServerRequest *request);
void dumpHex(const void* data, size_t size) ;
void makeJSONpreset(int preNum);
void printTail(char * str, int tailLen);
int getClient(AsyncWebServerRequest *request);
char getScreen(AsyncWebServerRequest *request);
int registerHTTPslave(AsyncWebServerRequest *request, int session);
void printParams(AsyncWebServerRequest *request);
void listFiles(void);
// chunked - doesn't work well
void sendBigFile(char* filename, char * contentType, AsyncWebServerRequest *request);
// pre-read file
void sendBigFile2(char* filename, char * contentType, AsyncWebServerRequest *request);

void setDig(int chan, int val);
void makeLog(void);
void makeLogJSON(int);
uint8_t setModeB(uint8_t bMode);
uint8_t setMode(uint8_t wave);
//void setOnOffxx(int8_t channel, bool status);
int startBAT(int x);
int startST(int x);
 
bool webScreenChanged = true; // not valid for multiple HTTP sessions - so ignore

const String PARAM_INPUT_CMD = "cmd";
const String PARAM_SCREEN_CMD = "screen";
const String PARAM_CLIENT_CMD = "clientID";
const String PARAM_SESSION_CMD = "sessionID";
const String PARAM_INPUT_1 = "value_1";
const String PARAM_INPUT_2 = "value_2";
#define CMD_TOGGLE 1
#define CMD_SCREEN 2
#define CMD_PAD 3
#define CMD_FADER 4
#define CMD_INAME 5
#define CMD_ONAME 6
#define OUTLEN 16

// cut down web traffic by only sending faders and names occasionally with level packets (unless a chanage has been flagged)
// this needs to be significantly 

#define UPDATESEVERY
#define SENDNAMESEVERY 4
#define SENDFADERSEVERY 3


int levelLoops = 0;

void webServerStart()
{      

 // sptr[WEBSTREAM] = WEBSTREAMP;  
  // Servicing for input and Output page requests (new screen)
  server.on("/", HTTP_ANY, [](AsyncWebServerRequest *request)
  {
    // screen changes
    webScreenChanged = true;	
	Serial.print("Serving / ");
  valChange(VAL_CHGD_LOCAL); // new page request gets all settings.
	//_thisClient = getClient(request); // used in processor()
	_thisScreen = getScreen(request);
    slashRequest(request, _thisClient);  
    request->send_P(200, "text/html", index_html, processor);
    //request->send_P(200, "text/html", index_html);
  });
  
  //404
  server.onNotFound(notFound);

  // settings screen page 
  server.on("/settings", HTTP_ANY, [](AsyncWebServerRequest *request)
  {
    // screen changes
	  Serial.print("/settings ");
	 // _thisClient = getClient(request);
	  _thisScreen = getScreen(request);
	  //printParams(request);
    slashRequest(request, _thisClient);     
    request->send_P(200, "text/html",  index_html, processor); //settings_html
  });

  //fast GET from web client: send readings screenID and other data as needed 
  server.on("/readings", HTTP_GET, [](AsyncWebServerRequest *request)
  { 
    char scrn[2] = "M";      // default to main screen
    bool faderCh, padCh, namesCh;

    //_thisClient = getClient(request);
    _thisScreen = getScreen(request);
	  strcpy((char *)&JSONstring, "{\n");   
      //strcat((char *)&JSONstring, ",\n");  //  only append these if valChanged_broadcast
      makeJSONsets(_thisScreen, _thisClient); 
      strcat((char *)&JSONstring, "\n}");
if(strlen(JSONstring) > 200)  Serial.print(JSONstring);
    request->send(200, "text/plain", JSONstring);
  });

  // input controls from Web page
  // Process a GET request to <ESP_IP>/update?output=<inputMessage1>&state=<inputMessage2>
  // JS GET. process commands from web page - update sliders, buttons, change screen
  server.on("/update", HTTP_GET, [] (AsyncWebServerRequest *request)
  {
		 //_thisClient = getClient(request);
	 _thisScreen = getScreen(request);
     webCommands(request);
     request->send(200, "text/plain", "OK");
  });
  
  
// generic files, no need to know which client requests.
  // load CSS, JS and icon files
  server.on("/DDS.css", HTTP_GET, [](AsyncWebServerRequest *request)
  {
#ifdef WEB_GZIP
    AsyncWebServerResponse *response = request->beginResponse(LittleFS, "/DDS.css.gz", "text/css");
    response->addHeader("Content-Encoding", "gzip");
#else
    AsyncWebServerResponse *response = request->beginResponse(LittleFS, "/DDS.css", "text/css");
#endif
    response->addHeader("Cache-Control", "max-age=31536000");
    request->send(response);
  });    
  server.on("/DDS.js", HTTP_GET, [](AsyncWebServerRequest *request)
  {
#ifdef WEB_GZIP
    AsyncWebServerResponse *response = request->beginResponse(LittleFS, "/DDS.js.gz", "text/javascript");
    response->addHeader("Content-Encoding", "gzip");
#else
     AsyncWebServerResponse *response = request->beginResponse(LittleFS, "/DDS.js", "application/javascript");
#endif
    response->addHeader("Cache-Control", "max-age=31536000");
    request->send(response);  
  });
  server.on("/jogDial.min.js", HTTP_GET, [](AsyncWebServerRequest *request)
  {
#ifdef WEB_GZIP
    AsyncWebServerResponse *response = request->beginResponse(LittleFS, "/jogDial.js.gz", "text/javascript");
    response->addHeader("Content-Encoding", "gzip");
#else
    AsyncWebServerResponse *response = request->beginResponse(LittleFS, "/jogDial.min.js", "application/javascript");
    #endif
    response->addHeader("Cache-Control", "max-age=31536000");
    request->send(response); 
  });
  server.on("/favicon.ico", HTTP_GET, [](AsyncWebServerRequest *request){
    AsyncWebServerResponse *response = request->beginResponse(LittleFS, "/favicon.ico", "text/plain");
    response->addHeader("Cache-Control", "max-age=31536000");
    request->send(response);
  });
  server.on("/base_bg.png", HTTP_GET, [](AsyncWebServerRequest *request){
    AsyncWebServerResponse *response = request->beginResponse(LittleFS, "/base_bg.png", "image/png");
    response->addHeader("Cache-Control", "max-age=31536000");
    request->send(response);
  });
  server.on("/base_knob.png", HTTP_GET, [](AsyncWebServerRequest *request){
    AsyncWebServerResponse *response = request->beginResponse(LittleFS, "/base_knob.png", "image/png");
    response->addHeader("Cache-Control", "max-age=31536000");
    request->send(response);   
  });

  configureUtilityPages(); // file manager: URL=/utility

  // Start server
  server.begin();
  //listFiles();
  webServerOn = true;
}

// 404
void notFound(AsyncWebServerRequest *request) {
  String logmessage = "Client:" + request->client()->remoteIP().toString() + " " + request->url();
  Serial.println(logmessage);
  request->send(404, "text/plain", "Not found");
}
// getClient() and getScreen() may be from GET or POST requests. 
// All other variables are GET only
/*
int getClient(AsyncWebServerRequest *request)
{
	String bufS, bufC;
	bool gotClient = false, gotSession = false;
	int session = -1, bufI;
	// find clientID parameter in the request
	// none? flag error
	// if the clientID == -1, register new client
	// return the clientID
	
	if(request->hasParam(PARAM_SESSION_CMD, true)) // POST 
	{
		//Serial.print(" SE_POST ");
		bufS = request->getParam(PARAM_SESSION_CMD, true)->value();	
		//bufI = request->getParam(PARAM_SESSION_CMD, true)->value();	
		gotSession = true;
		session = bufS.toInt();
	}
	
	if(request->hasParam(PARAM_SESSION_CMD)) // GET 
	{
		//Serial.print(" SE_GET ");
		bufS = request->getParam(PARAM_SESSION_CMD)->value();	
		//bufI = request->getParam(PARAM_SESSION_CMD)->value();	
		gotSession = true;
		session = bufS.toInt();
	}
	//Serial.printf("SessionID = |%s| %i\n", bufS.c_str(), session);
	_thisSession = session;
	
	return 1;
}
*/
char getScreen(AsyncWebServerRequest *request)
{
	String ss;
	char screen;
	bool gotScreen = false;
	if (request->hasParam(PARAM_SCREEN_CMD)) // GET
    {
		 //Serial.print(" SC_GET ");
		 ss = request->getParam(PARAM_SCREEN_CMD)->value();	
		 gotScreen = true;
	}
		// convert string to character
	if(request->hasParam(PARAM_SCREEN_CMD, true)) // POST 
	{
		//Serial.print(" SC_POST ");
		ss = request->getParam(PARAM_SCREEN_CMD, true)->value();	
		gotScreen = true;
	}
	if(gotScreen)
	{
		screen = (ss.c_str())[0];	
		return screen;
	}		
	
	Serial.printf("Missing screen ID in request\n");
	return 'M';	// default to Input
}
// look for a spare slot, otherwise kick out first sleepy one

// service "/" request
// just upload the page (done in server.on() above)
// make sure the next /update includes screen info
void slashRequest(AsyncWebServerRequest *request, int client)
{ 
		currentWebScreen = 'M';	// default for new page requests
}

// service "/update" request
// send JSON of required data
// always readings, screen info and other data as required.
// Most have format CXSY: C= Type (Setting, Mode...), X = setting array index (0..7), S = (S = setting), Y = channel (A, B, burst/Sweep, Control)
// Start/stop buttons are STRTY: Y = channel
/*************** WEB COMMANDS ********************************/
void webCommands(AsyncWebServerRequest *request)
{
    int cs = currentWebScreen - '0'; // char to int conversion
    cs = constrain(cs, 0,7);
    char val1[4];	
    bool foundCmd = false;
    //int btn  = -1;
    int wave = -1;
    int chan = -1;
    int indx = -1;
	  webScreenChanged = true; 
    String inputCmd = "No cmd.";
    String inputParam1 = "No param_1.";
    String inputParam2 = "No param_2.";
    char val_1[128] ="No val_1.";
    char val_2[128] ="No val_2.";
    char val_s[16] ="X";
    char scrnChar = 'X';
	  int scrn;	//[0..7 or 29 for input]

   // all requests should have cmd and scn; may have value-1 and value_2 parameters.
    int cmd = -1;
    // GET input1 value on <ESP_IP>/update?output=<inputMessage1>&state=<inputMessage2>
     if (!request->hasParam(PARAM_INPUT_CMD)) // every request should have this argument
     {
      Serial.println("Error: request missing CMD parameter");
      return;
     }
    
    inputCmd = request->getParam(PARAM_INPUT_CMD)->value();
    cmd = inputCmd.toInt();
    // all requests must have cmd and scrn parameters.
    // Serial.printf("/update: cmd=|%s|",inputCmd);

    if (request->hasParam(PARAM_INPUT_1)) 
    {
       inputParam1 = request->getParam(PARAM_INPUT_1)->value();
       inputParam1.toCharArray(val_1, 128);
       
       // val_1 format: "pad_X" 
       strncpy((char *)&val1, val_1, 3);  // first three chars tell object swpMode
       //btn =  val_1[4] - '0' ;  // not used - index to button array  
       chan = val_1[3] - '0';
       indx = val_1[1] - '0'; // setting number 
       if(chan < CHANS)
        wave = iSet.waveForm[chan];
       else 
        wave = iSet.waveForm[CHAN_A];
       //Serial.printf(" value_1|%s|, chan %i, wave %i; ", val_1, chan, wave);
    }

    if (request->hasParam(PARAM_INPUT_2)) 
    {
      inputParam2 = request->getParam(PARAM_INPUT_2)->value();
      inputParam2.toCharArray(val_2, 128);
      //Serial.printf(" value_2=|%s|\n",val_2);
    }
	  char ssc = getScreen(request);
    if (ssc != 'X') // every request should have this argument
    {
      //inputParam2 = request->getParam(PARAM_SCREEN_CMD)->value();
      //inputParam2.toCharArray(val_s, 16);
      scrnChar = ssc;
	    scrn = scrnChar - '0';
     // Serial.printf(" screen=|%s|%c| \n",val_s, scrnChar);
    }
    else
    {
      Serial.println("Error: request missing SCREEN parameter");
      return;
    }
	float tempF = atof(val_2);
  int   tempI = atoi(val_2);

	// process the commands 
    uint8_t dummy, * vp = &dummy;
	// main screen

	if(strncmp(val_1, "STR",3) == 0) // redundant?
  {
    //Serial.printf("Start ch %i : %i", chan, tempI);
		//setDAC(tempF);	
    valChange(VAL_CHGD_WEB); 
    foundCmd = true;
  }
    
  // value setting
  if(val_1[0] == 'S' && val_1[2] == 'S')
  {
    foundCmd = true;
    valChange(VAL_CHGD_WEB); 
    if (chan < CHANS || chan == CONTROL_CHAN)// A, B Control
    { 
      switch (chan)
      {
        case CHAN_A:
        case CHAN_B:  
        //Serial.printf("AS-SxSx chan %i\n", chan);      
          modeVal[chan][wave][indx] = tempF;
          if((!BURST_MODE && BCOUPLE) || (BURST_MODE && BALTA))
            fifoCmd = fifoCmd | VAL_CHANGE_A | VAL_CHANGE_B; 
          else
            fifoCmd = fifoCmd | ((chan == CHAN_A) ? VAL_CHANGE_A : VAL_CHANGE_B);           
          break;
        case CONTROL_CHAN:
        //Serial.printf(" -Set Cont- %i, %f ", indx, tempF);
          modeVal[0][WA_CONT][indx] = tempF;
          //if(indx == CP_BA || indx == CP_PH) // stop and perhaps restart chan B 
          {
            // change in phase, if coupled, needs a restart
            if(indx == CP_PH && BCOUPLE)
              fifoCmd |= STOP_B | START_B;
            // Stop B if uncoupling - now caught in CPU0 main()
            //if(dds[CHAN_B].run && (indx == CP_BA && BCOUPLE)) fifoCmd |= START_B;
          }
          break;       
      }     
    }
    else // sweep/burst (BU_SW_CHAN) value change
    {      
      switch (iSet.modeA)
      {        
        case MO_BURST :
          //Serial.printf("Bur indx %i -> %i\n",indx, tempI);
          //burst[indx] = tempF;
          modeVal[0][WA_BURST][indx] = tempI; // set both locations
          switch(indx)
          {
            case BU_ON:
              burst.cyclesOn = tempI;
              break;
            case BU_OFF:
              burst.cyclesOff = tempI;
              break;
            case BU_BALTA:
              burst.bAltA = tempI;
              // reset B phase and restart 
              if(tempI)
                fifoCmd = fifoCmd | VAL_CHANGE_B |STOP_B | START_B;
              break;
            case BU_REP:
              burst.repeat = tempI;
              break;
            default:
              break;
          }
          //setBurst(99);
          fifoCmd = fifoCmd | VAL_CHANGE_A; 
          break;

        case MO_SWEEP :
          if(indx == 0) // sweep mode F/A/D
          {
            valChange(VAL_CHGD_WEB);
            valChange(VAL_CHGD_LOCAL); // exception: cause new mode values to be sent to web 
           // sweepVal[wave].swpMode = (sweepMode_t)tempI; // redundant
            sweepVal[iSet.waveForm[CHAN_A]].val[0][0] = (sweepMode_t)tempI;
            Serial.printf("WebCom set swpMode %i\n",(int)sweepVal[iSet.waveForm[CHAN_A]].val[0][0]);
          }
          else 
            sweepVal[wave].val[(int)((sweepMode_t) ((int)sweepVal[iSet.waveForm[CHAN_A]].val[0][0]))][indx] = tempF;
          //Serial.print("Swp ");
          setSweep(99);
          fifoCmd = fifoCmd | VAL_CHANGE_A; 
          if(BCOUPLE)
            fifoCmd = fifoCmd | VAL_CHANGE_B; 
          break;

        case MO_NOR :
        default :
          setNor(99);
          break;
          // do nothing. menu is disabled
      }
    }
    //Serial.printf("Setting ch %i[%i]: %3.2f", chan, indx, tempF);
  }
  
  //start/stop buttons A,B, SW/BU
  if(val_1[0] == 'B') 
  {
    foundCmd = true;
    valChange(VAL_CHGD_WEB);
    bool val = val_1[1] == '1';
    //char chan = val_1[3];
    //Serial.printf("Start/Stop %i: %i\n", chan, val);

    if(chan < CHANS)
    {
     // dds[chan].run = (val == '1');
     // iSet.run[chan] = val;
      if((!BURST_MODE && BCOUPLE) || (BURST_MODE && BALTA))
        fifoCmd = fifoCmd | ((val) ? START_A | START_B : STOP_A | STOP_B);
      else
      {
        if(chan == CHAN_A)
          fifoCmd = fifoCmd | ((val) ? START_A : STOP_A);
        else
          fifoCmd = fifoCmd | ((val) ? START_B : STOP_B);
      }
      
    }
    if(chan == BU_SW_CHAN)
    {
      if(iSet.modeA == MO_SWEEP)
      {
        fifoCmd = fifoCmd | STOP_BURST | ((val) ? START_SWEEP : STOP_SWEEP);
        //burst.run = false;
        iSet.selChan = CHAN_A;        
      }
      if(iSet.modeA == MO_BURST)
      {      
        /*  
        if(val)
          startBurst(99);
        else
          stopBurst(99);
          */
        //swCont.run = false; 
        //burst.run = val;
        fifoCmd = fifoCmd | STOP_SWEEP | ((val) ? START_BURST : STOP_BURST);
        iSet.selChan = CHAN_A;          
      }
      if(iSet.modeA == MO_NOR)
      {
        //fifoCmd = fifoCmd | STOP_A;
        //swCont.run = false;
        //fifoCmd = fifoCmd | STOP_BURST | STOP_SWEEP;
        //burst.run = false;  
       ; // do nothing  
      }   
    }
  }
  if(val_1[0] == 'M') // Wave/mode select
  {
    //valChange(VAL_CHGD_WEB);
    valChange(VAL_CHGD_LOCAL); // exception: cause new mode values to be sent to web 
    foundCmd = true;
    //Serial.printf("Wave/Mode chan %i: mode%i\n", chan, tempI);

    if(chan < CHANS)    
    {
      iSet.waveForm[chan] = (DDSwaves)tempI; 
      setScrMode(chan, (DDSwaves)tempI, MAIN_MENU);
    }
    if(chan == BU_SW_CHAN)
      iSet.modeA = (DDSmodes)tempI;
    valChange(VAL_CHGD_WEB);   
    if((!BURST_MODE && BCOUPLE) || (BURST_MODE && BALTA))
     fifoCmd = fifoCmd | FUNC_CHANGE_A | FUNC_CHANGE_B;  
    else
      fifoCmd = fifoCmd | ((chan == CHAN_A) ? FUNC_CHANGE_A : FUNC_CHANGE_B);      
  } 
    
  if (!foundCmd)
    Serial.printf("WebCmd not found [%s] = [%s]\n", val_1, val_2);   
}
	
/* JSON
 * Readings only
 * All variable names are of the form "ccccIJ". J is context optional. cccc is generally the name of the HTML id of the control, I is the control number usually [0..7]
 */
 // just the core JSON lines. Calling routine needs to add leading { and trailing , or }
void makeJSONreads(char screen, int client)
{
  char nums[16];  
  char scrStr[20] = "0";

	//Serial.printf("Request /update has %i params:\n",request->params());
	// printParams(request);
	//Serial.printf("/levels to %i: flags 0x%4x: ", client, slave[client].vChanged);
	//printClientChanges(client);

	//Serial.printf("faders & gains %i, pads %i, names %i\n", faderGains, pads, names);
  // screen first
  // only add these immediately after a screen change (or name update?)
  // or pending name changes on web page will be overwritten
  //char sc = screen[0];
  
  int scn = screen - '0'; // integer version of screen 
  if(scn < 0 || scn > INCHANS) 
    scn = -1;
  scrStr[0] = screen;

	strcat((char *)&JSONstring, "\"screen\":\"");
	strcat((char *)&JSONstring, scrStr);
	strcat((char *)&JSONstring, "\", ");
	
	strcat((char *)&JSONstring, "\"device\":\"");
	strcat((char *)&JSONstring, myHostName);
	strcat((char *)&JSONstring, "\"");

  // readings
	if(screen =='M')
	{

	}
	if(screen =='S')
	{

  }
   
  //  Serial.printf("JSON-readings: scrn %c |%s| %i\n", screen, JSONstring, strlen(JSONstring));  
  // dumpHex((void *)JSONstring,  strlen(JSONstring));
  webScreenChanged = false;
}

// just the core JSON lines. Calling routine needs to add leading { and trailing , or }
 /************  modified - 1 set per call ****************/
int thisChan = 0;
#define MAXSETS (CHANS+2)
void makeJSONsets(char screen, int client)
{
  char nums[16];  
  char scrStr[20] = "0";
  //  bool pads = false, faderGains = false, names = false;
	//Serial.printf("Request /update has %i params:\n",request->params());
	// printParams(request);
	//Serial.printf("/levels to %i: flags 0x%4x: ", client, slave[client].vChanged);
	//printClientChanges(client);
 
	//Serial.printf("faders & gains %i, pads %i, names %i\n", faderGains, pads, names);
  // screen first
  // only add these immediately after a screen change (or name update?)
  // or pending name changes on web page will be overwritten
  //char sc = screen[0];
  
  int scn = screen - '0'; // integer version of screen 
  if(scn < 0 || scn > INCHANS) 
    scn = -1;
  scrStr[0] = screen;
  //strcpy((char *)&JSONstring, "{\n");

  strcat((char *)&JSONstring, "\"screen\":\"");
  strcat((char *)&JSONstring, scrStr);
  strcat((char *)&JSONstring, "\", ");
  
  strcat((char *)&JSONstring, "\"device\":\"");
  strcat((char *)&JSONstring, myHostName);
  strcat((char *)&JSONstring, "\"\n");
  if(strlen(messageLine))
  {
    Serial.printf("JSON [%s]\n", messageLine);
    strcat((char *)&JSONstring, ",\"ems\":\"");
    strcat((char *)&JSONstring, messageLine);
    strcat((char *)&JSONstring, "\"\n");
  }

  int chan, setFunc;
  if(screen =='M' && updateVal_web)
  {
    updateVal_web = false;
    // DDS channels + control + burst or sweep (separate code) 
    for(chan = 0; chan < CHANS+2; chan++) // chans A & B 
    {      
      //setFunc = iSet.waveForm[chan];
      addSet(chan);   
      //yield();      
    }    
/*
    addSet(CONTROL_CHAN, WA_CONT);

    // add sweep or burst for current swpMode & chan A waveform
    switch (iSet.modeA)
    {
      case MO_BURST :
        addSet(CHAN_A, WA_BURST); 
        break;
      case MO_SWEEP :
        addSet(CHAN_A, WA_SWEEP);
        break;
      case MO_NOR :
      default :
        //break;
        addSet(CHAN_A, WA_NOR); // disables all the values
    }
    */
  }
   //Serial.printf("JSON-settings: scrn %c |%s| %i\n", screen, JSONstring, strlen(JSONstring));  
  //Serial.printf("Changes 0x%x\n", _changedVal);
  // dumpHex((void *)JSONstring,  strlen(JSONstring));
  webScreenChanged = false;
}
 
// setFunc is waveform for A & B, ignored for Control and OR Sweep
void addSet(int cChan)
{
  int setFunc = -999;
  char indx[3];
  char cha[3];
  bool enabled;
  char nums[16];
  int swpMode = -1;
  int altFunc;
  int chan;
  int tmp;

  bool sweepBurstIsNOR = false;
  bool isChanCol = false;
  bool isBurst = false;
  bool isSweep = false;
  bool isControlCol = false;

  switch (cChan)
  {
    case CHAN_A:
    case CHAN_B:
      chan = cChan;
      isChanCol = true;
      setFunc = altFunc = iSet.waveForm[chan];
      break;
    case CONTROL_CHAN:
      isControlCol = true;
      setFunc = WA_CONT;
      altFunc = iSet.waveForm[CHAN_A];
      chan = 0; // want to access CHAN_A values
      break;
    case BU_SW_CHAN:
      chan = 0; // want to access CHAN_A values
      //isBuSwpCol = true;
      altFunc = iSet.waveForm[CHAN_A];
      if(iSet.modeA == MO_BURST)
      {
        isBurst = true;
        setFunc = WA_BURST;
      }
      if(iSet.modeA == MO_SWEEP)
      {
        isSweep = true;
        setFunc = WA_SWEEP;
        tmp = sweepVal[altFunc].val[0][0]; //sweepVal[altFunc].swpMode;
        //Serial.printf("SWM swf %i (%1.3f), altf %i\n", tmp, sweepVal[altFunc].val[0][0], altFunc);
        swpMode = (sweepMode_t) tmp;  // VFD = [0..2]
      }
      if(iSet.modeA == MO_NOR) // 
      {
        sweepBurstIsNOR = true;
        setFunc = WA_NOR; // nothing there!
      }
      break;
    default:
      Serial.printf("Bad addSet chan %i\n", chan);
  }

  sprintf((char *)&cha, "%i", cChan); // channel ID string
  strcat((char *)&JSONstring, ",\"SNAME");
  strcat((char *)&JSONstring, cha);
  strcat((char *)&JSONstring, "\":\"");
  if(cChan == BU_SW_CHAN)
    strcat((char *)&JSONstring, "Burst/Sweep");
  else
    strcat((char *)&JSONstring, modeDefs[setFunc].name);
  strcat((char *)&JSONstring, "\" ");

  strcat((char *)&JSONstring, ",\"STYPE"); // value for select box
  strcat((char *)&JSONstring, cha);
  strcat((char *)&JSONstring, "\":\"");    
  if(cChan == BU_SW_CHAN)
  {
    sprintf((char *)&nums, "%i", iSet.modeA); 
    //Serial.printf("add BU SW %i\n", iSet.modeA);
  }
  else
    sprintf((char *)&nums, "%i", iSet.waveForm[chan]); 
  strcat((char *)&JSONstring, nums);
  strcat((char *)&JSONstring, "\" ");
  if (cChan != CONTROL_CHAN) // no start/stop buttons for Control
  {
    strcat((char *)&JSONstring, ",\"S0B"); // start/stop
    strcat((char *)&JSONstring, cha);
    strcat((char *)&JSONstring, "\":\"");
    if(isSweep)
      sprintf((char *)&nums, "%i", swCont.run); 
    if(isBurst) 
      sprintf((char *)&nums, "%i", burst.run); 
    if(isChanCol)
      sprintf((char *)&nums, "%i",  dds[chan].run); 
    strcat((char *)&JSONstring, nums);
    strcat((char *)&JSONstring, "\"\n");
  }
  else
    strcat((char *)&JSONstring, "\n");  

  for(int i = 0; i < MODEVALUES; i++)
  {
    sprintf((char *)&indx, "%i", i); 
    if(isSweep)
      enabled = (i < SWEEP_VALS);
    else
      enabled = (i < modeDefs[setFunc].nValues); // CHANNELS + burst + control
    if (sweepBurstIsNOR)
       enabled = false;
       
    if(enabled)
    {
      // setting text
      strcat((char *)&JSONstring, ",\"S"); // setting
      strcat((char *)&JSONstring, indx);
      strcat((char *)&JSONstring, "T"); // Description (Text)
      strcat((char *)&JSONstring, cha); // channel and waveform/wave
      strcat((char *)&JSONstring, "\":\"");
      strcat((char *)&JSONstring, modeDefs[setFunc].text[i]); // text label
      strcat((char *)&JSONstring, "\"");

      // setting value
      strcat((char *)&JSONstring, ",\"S"); 
      strcat((char *)&JSONstring, indx);

      strcat((char *)&JSONstring, "S"); // actual setting value
      strcat((char *)&JSONstring, cha);
      strcat((char *)&JSONstring, "\":");
      if(isSweep) // setting values from a different location  
        if(i == 0)          
          sprintf((char *)&nums, "%3.3f", sweepVal[altFunc].val[0][0]);   // sweep mode in first location of function line
        else
          sprintf((char *)&nums, "%3.3f", sweepVal[altFunc].val[swpMode][i]);          
      else
        sprintf((char *)&nums, "%3.3f", modeVal[chan][setFunc][i]); 
      strcat((char *)&JSONstring, nums);
      //if (isSweep) Serial.printf("AS Cont SW [%s], swM %i, altFunc %i, indx %i\n", nums, swpMode, altFunc, i);

      // high lim
      strcat((char *)&JSONstring, ",\"S"); 
      strcat((char *)&JSONstring, indx);
      strcat((char *)&JSONstring, "H");
      strcat((char *)&JSONstring, cha);
      strcat((char *)&JSONstring, "\":");
      if(isSweep && i > 0 && i < 3) // sweep max and min values for START and END 
        sprintf((char *)&nums, "%3.3f", sweepLim[altFunc].max[swpMode]); 
      else
      {
       if(isSweep && i == 0) // VFD limits
       {
         sprintf((char *)&nums, "%i", sweepLim[altFunc].options - 1); // limits for Sweep modes
         //Serial.printf("Addset swp opts %i",sweepLim[altFunc].options - 1);
       }
       else
         sprintf((char *)&nums, "%3.3f", modeDefs[setFunc].highLim[i]); 
      }
      strcat((char *)&JSONstring, nums);

      // low lim
      strcat((char *)&JSONstring, ",\"S"); 
      strcat((char *)&JSONstring, indx);
      strcat((char *)&JSONstring, "L");
      strcat((char *)&JSONstring, cha);
      strcat((char *)&JSONstring, "\":");
      if(isSweep && i > 0 && i < 3) // sweep max and min values for START and END 
        sprintf((char *)&nums, "%3.3f", sweepLim[altFunc].min[swpMode]); 
      else
      {
       if(isSweep && i == 0) // VFD limits
       {
         sprintf((char *)&nums, "%i", 0); // low imit for VFD always 0
         //Serial.printf("Addset swp opts %i",sweepLim[altFunc].options - 1);
       }
       else
         sprintf((char *)&nums, "%3.3f", modeDefs[setFunc].lowLim[i]); 
      }
      strcat((char *)&JSONstring, nums);        

      // units
      strcat((char *)&JSONstring, ",\"S"); 
      strcat((char *)&JSONstring, indx);
      strcat((char *)&JSONstring, "U");
      strcat((char *)&JSONstring, cha);
      strcat((char *)&JSONstring, "\":\"");     
      strcat((char *)&JSONstring, modeDefs[setFunc].units[i]);
      strcat((char *)&JSONstring, "\"");

      // Format
      strcat((char *)&JSONstring, ",\"S"); 
      strcat((char *)&JSONstring, indx);
      strcat((char *)&JSONstring, "F");
      strcat((char *)&JSONstring, cha);
      strcat((char *)&JSONstring, "\":\"");
      sprintf((char *)&nums, "%c", modeDefs[setFunc].fmt[i]); 
      strcat((char *)&JSONstring, nums); 
      strcat((char *)&JSONstring, "\"");   
      // colour  
      strcat((char *)&JSONstring, ",\"S"); 
      strcat((char *)&JSONstring, indx);
      strcat((char *)&JSONstring, "C");
      strcat((char *)&JSONstring, cha);
      strcat((char *)&JSONstring, "\":\"");
      sprintf((char *)&nums, "%i", getColor(modeDefs[setFunc].clr[i])); 
      strcat((char *)&JSONstring, nums); 
      strcat((char *)&JSONstring, "\"");   
    } 

    // enabled - always - to setFunc visibility of option
    strcat((char *)&JSONstring, ",\"S"); 
    strcat((char *)&JSONstring, indx);
    strcat((char *)&JSONstring, "E");
    strcat((char *)&JSONstring, cha);
    strcat((char *)&JSONstring, "\":");
    sprintf((char *)&nums, "%i", enabled); 
    strcat((char *)&JSONstring, nums);

    strcat((char *)&JSONstring, "\n");
  }
}
void printTail(char * str, int tailLen)
{
  int sl = strlen(str);
  if(sl  <  tailLen)
    tailLen = sl;
  str += (sl - tailLen);
  Serial.printf("Tail: ...|%s|\n",str);
}
void dumpHex(const void* data, size_t size) {
  char ascii[17];
  size_t i = 0, j;
  ascii[16] = '\0';
 Serial.printf("\n%3i: ", i);
  for (i = 0; i < size; ++i) {
    Serial.printf("%02X ", ((unsigned char*)data)[i]);
    if (((unsigned char*)data)[i] >= ' ' && ((unsigned char*)data)[i] <= '~') {
      ascii[i % 16] = ((unsigned char*)data)[i];
    } else {
      ascii[i % 16] = '.';
    }
    if ((i+1) % 8 == 0 || i+1 == size) {
      Serial.printf(" ");
      if ((i+1) % 16 == 0) {
        Serial.printf("|  %s \n%3i: ", ascii, i+1);
      } else if (i+1 == size) {
        ascii[(i+1) % 16] = '\0';
        if ((i+1) % 16 <= 8) {
          Serial.printf(" ");
        }
        for (j = (i+1) % 16; j < 16; ++j) {
          Serial.printf("   ");
        }
        Serial.printf("|  %s \n", ascii);
      }
    }
  }
}
void printParams(AsyncWebServerRequest *request)
{
	int params = request->params();
	Serial.printf(" Request has %i params\n", params);
	for(int i=0;i<params;i++)
	{
	  AsyncWebParameter* p = request->getParam(i);
	  if(p->isFile()) //p->isPost() is also true
	  {
		Serial.printf(" FILE[%s]: %s, size: %u\n", p->name().c_str(), p->value().c_str(), p->size());
	  } else 
		  if(p->isPost())
		  {
			Serial.printf(" POST[%s]: %s\n", p->name().c_str(), p->value().c_str());
		  } else 
		  {
			Serial.printf(" GET[%s]: %s\n", p->name().c_str(), p->value().c_str());
		  }
	}
}

void listFiles(void)
{
  File entry;
  int fileTot = 0;
  Serial.println("Files:");
  bool fsFound = FILESYS.begin();
  if(!fsFound)
    Serial.println("No FS");
  File dir = FILESYS.open("/","r");
  while(entry = dir.openNextFile())
  {  
	  bool isDir = entry.isDirectory(); // single level for SPIFFS, possibly not true for LittleFS
   // output += (isDir) ? "dir:" : "file: ";
   // output += "\t";
		if(isDir) 
			Serial.printf("Dir: %s\n", entry.name()); 
    else
    {
      fileTot += entry.size();
      Serial.printf("File: [%s] %i\n", entry.name(), entry.size()); 
    }
    entry.close();
  }	
  Serial.printf("Files total size: %i\n", fileTot);
}

// see https://github.com/khoih-prog/AsyncWebServer_RP2040W#chunked-response
// https://www.reddit.com/r/esp32/comments/zs1q9x/download_large_file_from_sd_card_using/
//********** Broken not re-entrant as it uses globals - required by lambda code.
/*
#define BSIZ 2048
#define FILEBUFSIZ 20000
void sendBigFile(char* filename, char * contentType, AsyncWebServerRequest *request)
{
  //lambda code breaks if these aren't global in the capture list
  uint8_t xbuffer[BSIZ+10]; 
  File thisFile; 
  size_t fileLen; 
  uint8_t fileBuf[FILEBUFSIZ];
  Serial.printf("File %s is ", filename);
  thisFile = FILESYS.open(filename, "r");
  if(!thisFile)
  {
    Serial.println("not available");
    //filledLength = 0;
  }
  else
  {   
    fileLen = thisFile.available();
    Serial.printf(" %i bytes\n", fileLen);
    thisFile.read(fileBuf, fileLen); // need to read entire file before starting - FLASH conflict? speed? 
  }
  // filledLength & maxLen are lambda variables
  AsyncWebServerResponse *response = request->beginChunkedResponse(contentType, [fileLen, fileBuf](uint8_t *xbuffer, size_t maxLen, size_t filledLength) -> size_t
  {
    size_t len = min(maxLen, fileLen - filledLength);
    len = min(len, BSIZ);
    memcpy(xbuffer, fileBuf + filledLength, len);
    //Serial.printf("Sent %i bytes [%c%c]\n", len, xbuffer[0],xbuffer[1]);
    //AWS_LOGDEBUG1("Total Bytes sent =", filledLength);    
    //AWS_LOGDEBUG1("Bytes sent in chunk =", len);    
    return len;
  });
 // response->addHeader("Transfer-Encoding", "chunked");
  //response->addHeader("Server","AsyncWebServer_RP2040W");
  request->send(response);
  thisFile.close();
}
void sendBigFile2(char* filename, char * contentType, AsyncWebServerRequest *request)
{
  File thisFile; 
  size_t fileLen; 
  //String ct = contentType;
  uint8_t fileBuf[FILEBUFSIZ];
  Serial.printf("File %s is ", filename);
  thisFile = FILESYS.open(filename, "r");
  if(!thisFile)
  {
    Serial.println("not available");
    //filledLength = 0;
  }
  else
  {   
    fileLen = thisFile.available();
    Serial.printf(" %i bytes\n", fileLen);
    thisFile.read(fileBuf, fileLen); // need to read entire file before starting - FLASH conflict? speed?
    *(fileBuf+1) = '\0'; // make it into a string. 
  }
 
  //request->addHeader("Cache-Control", "max-age=604800");
  //response->addHeader("Server","AsyncWebServer_RP2040W");
  request->send(200, contentType, (char *)fileBuf);//.addHeader("Cache-Control", "max-age=604800");
  thisFile.close();
}
*/

#endif
