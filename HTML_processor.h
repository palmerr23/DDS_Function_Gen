//Processor code 
// Replaces placeholders in the web page literal code
// Control element numbers have first three chars identify control, 5th char = channel number [0.. INCHANS-1]
// ********** Avoid using String objects extensively - fragments the heap.
#ifndef MYLHTTPX_H
#define MYLHTTPX_H
//#include <sstream>

char currentWebScreen = 'M'; // start with iNput screen
//char xbuf[64];
#define NUMSCREENS 1
// MAIN, STEP, LOG
char screenNames [][7] = {"Main","Log", "Cal", "Set"}; // goes on the button and in headline
char screenList[] = {'M','L','C', 'S'}; // actual reference
String readingsBlock(void);
String modeButtons(void);
String modeButtonsB(void);
String modeButtonsC(void);
String knobBlock(void);
String modeSelect(int channel, int mode);
String stepBlocks(void);
void setBlock(char * buf, int channel, int modeDefs);
void setBlocks(char * buf);
void printSet();

int _thisClient = -1; // index to the client array, so that the right update data can be sent to each separate session.
char _thisScreen = 'M'; // HTTP screen that the request was issued from, defaults to Main settings screen
int _thisSession = -1;  // unique (randomID for session)

// use a single char buffer for internal processing and convert to String on return - to reduce heap fragmentation.
String processor(const String& var){
  //Serial.println(var);
  String html = "";
  char webString[40000];
  int mode;
    
  // generate settings group
  if(var == "SETTINGS")
  { 
     switch (_thisScreen)
     {
        case 'S' :
        case 'C' :
      		strcpy(webString,  "<div class=\"row\" id=\"all_sets\">");    
	
      		   //html += knobBlock();
            strcat(webString,  "</div>");
      		html = String(webString);       
      	  break;
          
      	case 'M' :
            webString[0] = '\0'; 	
            
           // Serial.println("sBXX"); 
            // A
            strcat(webString,"<div class=\"column2\" style=\"background-color:#e6f7ff;\">"); // width:274px; float:center;width:400px; 
              mode = iSet.waveForm[LEFT];
              setBlock(webString, LEFT, mode); // dummy (modeDefs 0) block, updated by /readings call from client
            strcat(webString,"</div>");

            // B
            strcat(webString,"<div class=\"column2\" style=\"background-color:#e6f7ff;\">");
              mode = iSet.waveForm[RIGHT];
              setBlock(webString, RIGHT, mode);
            strcat(webString, "</div>");  

            // Control + knob
            strcat(webString, "<div class=\"column2\" style=\"background-color:#e6f7ff;\">"); 
              strcat(webString, knobBlock().c_str());
              strcat(webString, "<hr>");
              setBlock(webString, CONTROL_CHAN, WA_CONT); // no mode select for CONTROL             
            strcat(webString, "</div>");   

             // Burst/Sweep
            strcat(webString,"<div class=\"column2\" style=\"background-color:#e6f7ff;\">");  
              mode = iSet.modeA;            
              setBlock(webString, BU_SW_CHAN, mode);
            strcat(webString, "</div>");              
 //Serial.printf("WebString  len %i\n", strlen(webString));              
            html = String(webString);
            // embedded in a row. 2 columns

        		break;
        default :
          Serial.printf("Unknown screen %c\n", _thisScreen);
     }
     return html;
   }
   
   if(var == "KNOB")
   {      
      html = knobBlock();
      return html;
   }

  if(var == "SCREENBUTTONS")
  {  
	 String scn = (String)_thisScreen;   // current client screen
	 String cli = (String)_thisClient;   // this client's ID (-1 if undefined, but shouldn't be!)
	 String sess = (String)_thisSession;
   
	 // screen ID just used by JS
    String cc = (String)screenList[0];
    html += "<input type=\"text\" id=\"screenFI" + cc + "\" name=\"screen\" value=\"" + cc + "\" style=\"display:none;\">";
	  html += "<input type=\"hidden\" id=\"sessionID"+cc+"\" name=\"sessionID\" value=\"" + sess + "\" style=\"display:none;\">"; // dummy for SessionID value - filled in by JS on client
	  html += "<input type=\"hidden\" id=\"screenID\" name=\"screenID\" value=\"" + scn + "\" style=\"display:none;\">";
     return html;
  }

  if (var == "BODYTYPE") // not called from settings screen
  {
	  String btype ="O";
	  if(_thisScreen == 'N')
	  {
		  btype = "N";
	  }
	  html = "<body onload=\"setScreen()\" id=\"body"+ btype+ "\">";
	  return html;
  }
   return String(); // empty string
}


#define TYPE_LOC 2
void setBlock(char * html, int channel, int thisMode)
{  
  char buf[8];
  char idBuf[8];
  String res;
  int i;
  int items = AMODES;
  if (channel == CONTROL_CHAN)
    items = modeDefs[WA_CONT].nValues;

  String chName = (channel == 0) ? "A" : "B";

  if(channel == BU_SW_CHAN)
    chName = "Burst/Sweep";

  //Serial.printf("\nSB ch %i, mo %i, items %i: ", channel, thisMode, items);
  // embedded in a container and Row  + column
  // *** Values are dummy - updated by JS, all disabled initially
  //strcat(html, "<div class=\"row\" style=\"font-weight:bold; margin:auto; text-align:center;\">");
  if(channel == CONTROL_CHAN)
  {
    chName = "Control";
    strcat(html, "<div style=\"font-weight:bold; font-size:20px; margin:auto; text-align:center;\">"); // class ROW is problematic here
  }
  else
    strcat(html, "<div class=\"row\" style=\"font-weight:bold; font-size:20px; margin:auto; text-align:center;\">");
  strcat(html, chName.c_str());
 
  if (channel != CONTROL_CHAN)
  {     
      strcat(html, modeSelect(channel, thisMode).c_str());     
  }
  strcat(html,"</div>"); // row
  // Serial.printf("SB 2 |%s|\n", html);
  // each row has 6 fields:  T=Text, S=Setting, H=HighLim, L=LowLim, U=units, E=Enabled, F=Format
  for (i = 0; i < items; i++) // create slots for all possible values, unused will be hidden after /readings call
  {    
    sprintf((char *)&idBuf,"S%01iX%01i", i,channel ); // char X gets substituted with the field name [TSHLUEF or R]
    //  Serial.printf("|%s|, ", idBuf);
    
    sprintf((char *)&buf,"%01i", i ); 
    // for settings need to both select this one (one mousedown) for jog wheel; and validate any changes
    // blur() takes the focus off the input for mobile.

    idBuf[TYPE_LOC] = 'R'; // Text readonly
    strcat(html,"<div class=\"row\" id=\""); 
    strcat(html, idBuf ); 
    strcat(html, "\"><div style=\"position:relative; height:35px;\">");
    strcat(html, "<input type=\"text\" class=\"texts\" style=\"left:0px;top:3px;\" id=\"" ); //style=\"visibility:visible;\"
    // ID is S[nn]X][oo] : S for setting, nn = setting number, oo = settings group [0=CH_A, 1=CH_B, 2=BurstT/Sweep, 3=control], X = [TSHLUEF] setting element  
    idBuf[TYPE_LOC] = 'T';
    strcat(html, idBuf ); 
    strcat(html, "\" disabled size=\"4\" style=\"color:red; \" value=\"T\"\">\n"); 

    // proxy behind main setting
    idBuf[TYPE_LOC] = 'P'; // proxy setting value - made visible and calculated by JS for types R & V (overlays U)
    strcat(html, "<input type=\"none\" disabled class=\"reads\" id=\""); 
    strcat(html, idBuf);   
    strcat(html, "\" value=\"1\" style=\"left:70px;\" size=\"4\">\n");

    idBuf[TYPE_LOC] = 'S'; // setting value
    strcat(html, "<input type=\"text\" size=\"4\" class=\"reads\" style=\"left:70px;\" id=\""); // 
    strcat(html, idBuf); 
    strcat(html, "\" onfocus=\"selSet(this);\" onmousedown=\"selSetting(this)\" oninput=\"valid(this)\"  ");  //onchange=\"fixVal(this)\"
    strcat(html, "max=\"100\""); 
    strcat(html, " min=\"1\""); 
    strcat(html, " value=\"1");
    strcat(html, buf); 
    strcat(html, "\">\n");

    idBuf[TYPE_LOC] = 'U'; // units - hiddden by JS for format types R & V
    strcat(html, "<input type=\"text\" id=\""); 
    strcat(html, idBuf); 
    strcat(html, "\" disabled class=\"textsl\" style=\"left:150px; top:2px;\" size=\"1\" value=\"U\">\n"); // readonly

    idBuf[TYPE_LOC] = 'F'; // format - hidden, available for JS to display proxy value (from this string)
    strcat(html, "<input type=\"text\" id=\""); 
    strcat(html, idBuf); 
    strcat(html, "\" disabled class=\"texts\" size=\"2\" style=\"visibility:hidden;\" value=\"F\"></div></div>\n"); // readonly / hidden
    
//  Serial.printf("SB 2a %i\n", strlen(html));
  }
  if (channel != CONTROL_CHAN) // start / stop buttons
  {  
    char nameBuf[8]; 
    sprintf((char *)&idBuf,"B0R%01i", channel);
    sprintf((char *)&nameBuf,"BUTS%01i", channel ); // anything unique will do
    strcat(html,"<div class=\"row\" id=\"");
    strcat(html, idBuf);
    strcat(html,"\">");
      sprintf((char *)&idBuf,"B0V%01i", channel);
      strcat(html, "<div class=\"divz\"  style=\"background-color:red\" onclick=\"clickDiv(this)\" id=\"");
        strcat(html, idBuf);
        strcat(html, "\"><input type=\"radio\" class=\"btna\"");
        /*  id=\"");         idBuf[TYPE_LOC] = 'S';         strcat(html, idBuf);        */
        strcat(html, " name=\"");
        strcat(html, nameBuf);
        strcat(html, "A\" value=\"0\" onclick=\"clickSel(this)\"> &nbsp;");
        strcat(html, "<label for=\"");
        strcat(html, nameBuf);
      strcat(html, "A\" class=\"labyr\">Stop</label></div>\n"); //style=\"position:absolute;\"

      sprintf((char *)&idBuf,"B1V%01i", channel);
      sprintf((char *)&nameBuf,"BUTR%01i", channel ); // 
      strcat(html, "<div class=\"divz\"  style=\"background-color:green\" onclick=\"clickDiv(this)\" id=\"");
        strcat(html, idBuf);
        strcat(html, "\"><input type=\"radio\" class=\"btna\"");
        /* id=\"");        idBuf[TYPE_LOC] = 'R';         strcat(html, idBuf);        */
        strcat(html, " name=\"");
        strcat(html, nameBuf);
        strcat(html, "B\" value=\"1\" onclick=\"clickSel(this)\"> &nbsp;");
        strcat(html, "<label for=\"");
        strcat(html, nameBuf);
      strcat(html, "B\" class=\"labyr\">Run</label></div>\n");
    strcat(html,"</div>");          
  }
}

String knobBlock(void)
{
	String html;
	html = "\n";  
	//KNOB - attaches to last selected setting
  // width ~same as jog dial background
	 html += "<div class=\"row\"><div id=\"jog_dial\" style=\"height:110px; width:111px; margin:auto;\"></div>\n"; //class=\"dialX\" 
   //"00,000.00" radio buttons	- spaces make column a little wider
	 html += "&nbsp;&nbsp;<input type=\"radio\" name=\"mulBut\" id=\"mulBut10000\" value=10000 size=1><input type=\"radio\" name=\"mulBut\" id=\"mulBut1000\" value=1000 size=1>,<input type=\"radio\" name=\"mulBut\" id=\"mulBut100\" value=100 size=1><input type=\"radio\" name=\"mulBut\" id=\"mulBut10\" value=10 size=1><input type=\"radio\" name=\"mulBut\" id=\"mulBut1\" value=1 size=1 checked>\n";
	 html += "<span style=\"font-weight:bold; font-size:24px;\">.</span><input type=\"radio\" name=\"mulBut\" id=\"mulBut01\" value=0.1 size=1><input type=\"radio\" name=\"mulBut\" id=\"mulBut001\" value=0.01 size=1>&nbsp;&nbsp;";
	// hidden calculation fields - remove from here and js file for production.
	 html += "\n<input type=\"text\" hidden id=\"dialVal\" value=0 size=1> ";	// hide when not needed - code uses them
	 html += "<input type=\"text\" hidden id=\"deltaVal\" value=0 size=1>";	
	 html += "<input type=\"text\" hidden id=\"testVal\" value=0 size=1>\n</div>";	
	 // error line for all inputs
	// html += "<BR><input type=\"text\" maxlength=\"16\" class=\"errline\" readonly name=\"input_error\" id=\"input_errorID\" value =\" \" style=\"color:red;\" size=\"9\">\n"; 
	
	//Serial.printf("--------------Knob html |%s|\n", html.c_str());
  return html;
}
String modeSelect(int channel, int mode)
{
   String html = "";
   int i;
   char cc[6];
   char vc[3];

   sprintf(cc,"%01i", channel); // leading zeros
   String sc = cc;
   Serial.printf("modeSel chan %i [%s], mode %i", channel, sc.c_str(), mode);
   sprintf(cc, "%01i", channel);
   sc = cc;
   html = "<div id =\"M0D" + sc + "\"><label id =\"M0L" + sc + "\" class=\"lpad\" >";
   html += "<select id=\"M0S" + sc + "\" name=\"M0S" + sc + "\" class=\"divsel\" onclick=\"clickSel(this)\"/>";
   if (channel != BU_SW_CHAN) //
   {
    for(i = 0; i < MODENUM; i++)
    {
        if (modeEnabled[channel] & (1 << i)) // modeDefs enabled for this channel)
        { 
          sprintf(cc,"M%01iV%01i", i, channel); 
          sprintf(vc,"%01i", i); // just need value 
          sc = cc;
          String mn =  modeDefs[i].name;
          String selected = "";
          if(mode == i)
            selected =  " selected";
          html += "\n<option style=\"font-weight:700;\" id=\"" + sc + "\" value=\"" + vc +"\"" + selected + ">" + mn + "</option>"; // option list for this channel
        }
    }
   }
   else 
  {    
    html += "\n<option id=\"M0V" + sc + "\" value=\"0\"";  //MO_NOR
    if(mode == 0)
      html +=" selected";
    html +=">None</option>";

    html += "\n<option id=\"M1V" + sc + "\" value=\"1\""; //MO_SWEEP
        if(mode == 1)
      html +=" selected";
    html +=">Sweep</option>";

    html += "\n<option id=\"M2V" + sc + "\" value=\"2\""; //MO_BURST
        if(mode == 2)
      html +=" selected";
    html +=">Burst</option>";
  }
  // CONTROL has no options
  html += "</select></label></div>\n";
  return html;
}
/*
String modeButtons(void)
{
	String html = "";
 html = "<div class=\"column\" id=\"sCol\" style=\"background-color:#e6f7ff;\" padding-top:10px; padding-bottom:0px;>";  // not sure we need to identify this
   // buttons
   //html += "<div class=\"row\">";

	html += "<div id =\"DCV\" class=\"divz\"><label id =\"LCV\" class=\"lpad\" >";
	html += "<input type=\"checkbox\" id=\"CV\" name=\"NCV\" class=\"btnq\" onclick=\"clickDiv(this)\"/><span id =\"SCV\" class=\"btnz\">CV</span></label></div><BR> "; // CV but     

	html += "<div id =\"DCC\" class=\"divz\"><label id =\"LCC\" class=\"lpad\" >";
	html += "<input type=\"checkbox\" id=\"CC\" name=\"NCC\" class=\"btnq\" onclick=\"clickDiv(this)\"/><span id =\"SCC\" class=\"btnz\">CC</span></label></div><BR> "; // CC but  

	html += "<div id =\"DCP\" class=\"divz\"><label id =\"LCP\" class=\"lpad\" >";
	html += "<input type=\"checkbox\" id=\"CP\" name=\"NCP\" class=\"btnq\" onclick=\"clickDiv(this)\"/><span id =\"SCP\" class=\"btnz\">CP</span></label></div><BR> "; // CP but  

	html += "<div id =\"DCR\" class=\"divz\"><label id =\"LCR\" class=\"lpad\" >";
	html += "<input type=\"checkbox\" id=\"CR\" name=\"NCR\" class=\"btnq\" onclick=\"clickDiv(this)\"/><span id =\"SCR\" class=\"btnz\">CR</span></label></div><BR> "; // CR but  
 html += "</div>"; // column - </div> row
 return html;
}
*/


#endif
