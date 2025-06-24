/* update readings quickly, update other controls not as often */ 
var fetchTime = 2000;	// 1000 for production
loopsWithoutJSON = 0;
currentScreen = 'M';
currentScreenType = 'M';
var dial_el; 
var dial;
var lastDial = 0;
var dialX = 0;
var scaleModeX = 0; // 0 = LIN, 1 = LOG
var scaleModeY = 0; 
var settingToChange = null;
var settingToChangeName = null;
var buttonMul = 1;
var disconLoops = 15000/fetchTime;
var plotType = 1;
var PlotSize = 1;
var _currentChan = 0; // not used
var _currentMode = [0, 0];
var nVals = [0,0]; // number of values in current mode
const MAXCONTS = 4; 
const CONTCHAN = 3;
const MAXVALS = 7; // in each setting block
const CONTVALS = 5;  // CONTROL block has less settings
const CONTROLSIZE = 370;
const PLOTBASE_W = 50;
const MARGINS = 40;
const PLOTLOG = 1;
const PLOTLOGALL = 2;
const PLOTWAVE = 10;
const PLOTFFTLIN = 100;
const PLOTFFTLOG = 101;
const SCALELOG =1;
const textColrs = ["Red","blue", "green", "#404040", "black"];
sessionID = -1; // Set random at runtime. Sent with every request. 
//var dial_val = 0;

setInterval( // get settings and readings 
  () => 
{
	loopsWithoutJSON++;
	ems = document.getElementById("input_errorID"); // errorText
 
	//console.log("sType: no fetch", currentScreenType);
	//console.log("Fetching");
	if(loopsWithoutJSON > disconLoops) // (~15 secs) increase value for shorter Intervals
	{
		  //console.log("No FETCH JSON " + loopsWithoutJSON);		 
		  ems.value = "Disconnected";
	}
	var sid = document.getElementById("sessionIDM"); // any one will do
	//console.log("Current screen ", currentScreen);
	var bs;
	var ss;
  var fs; // format field element
  var vs; // value field element
  var us; // units field element
	var numVals;
	var chan;
	var chanI;
	var valArray;
	var uArray;
	var cmsID;
	var valID;
	var ch;
	var i;
	var arr;
  var txtColIndx;
  var txtColr;
	if(currentScreen != 'L')
    fetch("/readings?screen="+currentScreen+"&sessionID="+sid.value)
    .then(function (response) 
    {
      return response.json();
    })
    .then(function (myJson) 
    {      
      loopsWithoutJSON = 0;
      
      //console.log("/readings");
      // floats - 2 decimals
			//console.log("ST", chanI, valArray, numVals);					
			for(let chan = 0; chan < 4; chan++)
      {        
        var enabled = false;
        var fmt ='F'; 
        var units = "";
        // select box text
        cmsID =	"STYPE" + chan;
        //console.log("sname", chan, cmsID);
        if(cmsID in myJson && chan != CONTCHAN) 
        {          
          //bs = document.getElementById(cmsID); 
          var selID = "M0S" + chan;
          //console.log("sname2", selID, myJson[cmsID]);
          ss = document.getElementById(selID);
          ss.value = myJson[cmsID];
          //console.log("sname3", ss.value);
        }       

        for(let i = 0; i < MAXVALS; i++)
        {  
          if(chan == CONTCHAN && i > MAXCONTS) // CONTROL menu has less setting lines
            continue;
          // setting  enabled  
          cmsID =	"S" + i + "E" + chan ;		
          if(cmsID in myJson) 
          {                      
            if(myJson[cmsID] == 1) // hide disabled controls 
            {
              enabled = true;
            }	
            else
              enabled = false;   		
          } 
          else  // all JSON value lines should have an SnEm value
            continue; 

          // colour - will change T and S/P U if visible
          cmsID =	"S" + i + "C" + chan;	
          if(cmsID in myJson)
          {          
            txtColIndx = myJson[cmsID];  
            txtColr = textColrs[txtColIndx];     
            //if(i == 2) console.log("COLR", cmsID, txtColIndx, txtColr);    
          }

          // other fields only present in JSON when enabled
          // format 
          cmsID =	"S" + i + "F" + chan ;
          if(cmsID in myJson)
          {
            fs = document.getElementById(cmsID); 
            fmt = myJson[cmsID];     
            fs.value = fmt;               		
          } 

          // unit type and label
          cmsID =	"S" + i + "U" + chan;	
          us = document.getElementById(cmsID); 
          us.style.visibility = (enabled) ? "visible" : "hidden"; 
          if(cmsID in myJson)
          {
            units = myJson[cmsID]; 
            if(fmt == 'F')
              us.value = units;
            else
              us.value = units;
            if(enabled)
              us.style.color = txtColr;
          }

          // all elements are present on screen, but setting, text values, etc are only in JSON if enabled
          // row - deprecated?
          cmsID =	"S" + i + "R" + chan;	
          bs = document.getElementById(cmsID); 
          bs.style.visibility = (enabled) ? "visible" : "hidden"; 

          // setting value
          cmsID =	"S" + i + "S" + chan;	
          vs = document.getElementById(cmsID); // vs retained for limits, etc
          // proxy field is used when values are non-numeric (e.g. T/F or +/-)
          var proxyID = "S" + i + "P" + chan;
          var cs = document.getElementById(proxyID);           
          vs.style.visibility = (enabled) ? "visible" : "hidden";
          cs.style.visibility = "hidden";           
          vs.disabled = !enabled;
          // if(i == 5) console.log("VIS", cmsID, vs.style.visibility);
          if(cmsID in myJson)
          {     
            if(fmt == 'F')       
              vs.value = myJson[cmsID].toFixed(2);  
            else 
              vs.value = myJson[cmsID].toFixed(0); 
            //if(i == 5) console.log("FF", chan, i , fmt, vs.value);	    
            formatVal(chan, i, fmt, txtColr);  
          }

          //High/low limits: same vs (docID)' as 'S'
          cmsID =	"S" + i + "H" + chan;	          
          if(cmsID in myJson)
          {               
            vs.max = myJson[cmsID].toFixed(2);
          }
          cmsID =	"S" + i + "L" + chan;	          
          if(cmsID in myJson)
          {              
            vs.min = myJson[cmsID].toFixed(2); 
          } 

          // text label
          cmsID =	"S" + i + "T" + chan;	
          bs = document.getElementById(cmsID); 
          bs.style.visibility = (enabled) ? "visible" : "hidden"; 
          // add code for different formats 
          if(cmsID in myJson)
            bs.value = myJson[cmsID];

          if(enabled) // each element in line
          {
            bs.style.color = txtColr;
          }
          // if(i == 5) console.log("VIS", cmsID, bs.style.visibility);
       
        } // JSON single setting line

        // start/stop buttons
        cmsID =	"S0B" + chan;	         
        var onbut;
        if(cmsID in myJson)
        {
          //console.log("BUT",cmsID);
          if(myJson[cmsID] == "1")
            onbut = "B1V" + chan;
          else
            onbut = "B0V" + chan;          
          if(chan != CONTCHAN)
          {
            //console.log("SBC", onbut);
            setButtonColor(onbut);
          }
        }      
        cmsID = "XYZZY";     
      }	// chan loop	
  
    // screens
	  if("screen" in myJson)
	  {             
		 ss = myJson["screen"]; // value to set
		 currentScreen = ss;
		 //console.log("Setting currentScreen to",ss); 
		 dispC = "none";
		 dispA = "center";
		 bs = document.getElementById("screenName");
		 //sb = document.getElementById("submit"+ss);
		 if(ss == "M") 
		 {
			bs.value = "Main";	
			currentScreenType = 'M';
		 }
		 if (ss == "S") // deprecated
	     {
		    bs.value = "Step"; 
		    currentScreenType = 'S';					  
	     }
		 
		 // if (ss == "L") // nothing to do.
    }
    /*
	  if("device" in myJson) // deprecated
	  {
		  console.log("Device", ss);
		  ss = myJson["device"]; // value to set
		  sb = document.getElementById("devName");
		  //sb.value = ss;
	  }
    */
    if("ems" in myJson) // notification line
    {
        console.log ("ems",  myJson["ems"], ems.value);
        ems.value = myJson["ems"];
    }
    else
      ems.value = "";
    })
    .catch(function (error) {
      console.log("Error: " + error, cmsID);
  });
  },
  fetchTime // 500-1000 mS is about right
);

function formatVal(chan, set, fmt, colr)
{
  var thisID = "S" + set + "P" + chan;
  var ps = document.getElementById(thisID); 
  thisID = "S" + set + "S" + chan;
  //console.log("FV", thisID);
  var vs = document.getElementById(thisID); 
  thisID = "S" + set + "U" + chan;
  var us = document.getElementById(thisID); 
  var units = us.value;
  //console.log("FV", thisID, units, vs.value);

  if(fmt == 'R') // display units character indexed by setting value 
  {
    ps.value = units.charAt(vs.value);
    //console.log("FMT R:", vs.value, units, units.charAt(parseInt(vs.value)));
  }
  if(fmt == 'V') // display units split string indexed by setting value 
  {
    const splitFmt = units.split(" "); 
    ps.value = splitFmt[parseInt(vs.value)];   
    //console.log("V", vs.value,us.value, ps.value);
  }  
  if(fmt == 'R' || fmt == 'V')
  {
    vs.style.color = "transparent";  
    us.style.visibility = "hidden";               
    ps.style.visibility = "visible"; 
    ps.style.color = colr;
  }
  else
  {
    vs.style.color = colr;  
    us.style.visibility = "visible";               
    ps.style.visibility = "hidden";
  }
}

function fixVal(el)
{
  var conID = el.id; 
  var chan = conID.charAt(3);
  var set = conID.charAt(1);
  //console.log("FV", conID, chan, set);
  conID = "S" + set + "F" + chan;
  var fmt = document.getElementById(conID).value;
  var set = conID.charAt(1);
  //console.log("FixV", conID, chan, fmt, set);
  formatVal(chan, set, fmt, "black");
}

// screen load
function setScreen(element) {
	var i;
		// a random number to differentiate from other sessions from the same IP - may get changed on each page load?
	if (sessionStorage.getItem('sessionID') == null)
	{
		
		ssval = parseInt(Math.random() * 1000000);
		sessionStorage.setItem('sessionID', ssval); 
	}
	sessionID = sessionStorage.getItem('sessionID');
	//console.log("SessionID", sessionID);
	var scr = document.getElementById("screenID");
	for(i of ['M']) //,'S','L'
	{ 
		ss = document.getElementById("sessionID"+i)	;	
		ss.value = sessionID;
	 }
	currentScreen = scr.value;		
	if (document.body.id == "bodyS")
	{
		 currentScreenType = 'S';	//a settings screen with a knob
	
		//console.log("starting knob");
		dial_el = document.getElementById("jog_dial");
		var dial_options = {wheelSize:'111px', knobSize:'30px', minDegree:null, maxDegree:null, degreeStartAt: 0};//debug: true, 
		dial = JogDial(dial_el, dial_options).on('mousemove', function(evt)
    { 
			var delta = 0;
			var dialVar = document.getElementById("dialVal");
			var dTest = document.getElementById("testVal");
			var ddV = document.getElementById("deltaVal");
			var mBut = document.getElementsByName("mulBut");
			var errLine = document.getElementById("input_errorID");
			var decimals = 2;
			if(["time1S", "steps1S",  "time2S", "steps2S", "cycS", "riseS"].includes(settingToChangeName)) // integers
				decimals = 0;
			for(i = 0; i < mBut.length; i++)
				if(mBut[i].checked)
				{
					//console.log("mBut found", i);
					delta = +mBut[i].value;
					ddV.value = delta;
				}
			//console.log("DM", delta);
			var dial_val = evt.target.rotation;
			dialX = 0;
			if( dial_val - lastDial > 5)
			{	
				dialX = +delta;			
				lastDial = dial_val;				
			}
			else if(dial_val - lastDial < -5)
			{
				dialX = -delta;	
				lastDial = dial_val;			
			}
			dialVar.value = dialX; 
			if(dialX == 0)
				return;
			
			if(settingToChange != null) // don't allow overruns
			{ 
				dTest.value = +settingToChange.value + dialX;
				if(+settingToChange.value + dialX > +settingToChange.max)
				{
					settingToChange.value = (+settingToChange.max).toFixed(decimals);
					setChange(settingToChange);
					errLine.value = "Max value";				
				}
				else
					if(+settingToChange.value + dialX < +settingToChange.min)
					{				
						settingToChange.value = (+settingToChange.min).toFixed(decimals);
						errLine.value = "Min value";
						setChange(settingToChange);
					}
					else {
						settingToChange.value = (+settingToChange.value + dialX).toFixed(decimals);
						errLine.value = " ";
						setChange(settingToChange);
					}
          valid(settingToChange);
			}
		});
	}
	else // bodyL
	{
		 sb = document.getElementById("submitL"); 
		 //sb.style.color = "white";	
		 currentScreenType = 'L';	// log screen/  will be updated on first GET?	 
	}
}

// Channel and Sweep/Burst Run/Stop buttons 
// this may cause a loop, by triggering clickDiv
// set both buttons
function setButtonColor(con)
{
	var i;
  var chan = con.charAt(3);
	divS = "B0V" + chan;
  divR = "B1V" + chan;
	//spa = "S"+con;	// the span
	divSEl = document.getElementById(divS);		// Stop
	divREl = document.getElementById(divR);		// Run
   // labEl = document.getElementById(lab);		// label
	//spaEl = document.getElementById(spa);		// span
	//var oncolor = "blue";
//console.log("SBC", con, divS);
	if (con == divR) // run
	{
	//console.log("Run");
	  divSEl.style.background = "#ccc"; 
    divSEl.style.color = "black";
	  divREl.style.background = "green";
	  divREl.style.color = "white";
	}
	else
	{
		//	console.log("Stop");
	  divREl.style.background = "#ccc";
    divREl.style.color = "black";
	  divSEl.style.background = "red";
	  divSEl.style.color = "white";
	}
	//console.log("setButtonColor ID =", con, divS);
}

// clicked a run/stop button 
// BxVch
function clickDiv(element) {
	var xhr = new XMLHttpRequest();
	con = element.id; 
  var val = con.charAt(1);
	setButtonColor(con);	
  console.log("clickDiv", con);
	//var sv = (conEl.checked) ? +1 : +0 ;
	xhr.open("GET", "/update?cmd=6&value_1="+con+"&value_2="+val+"&screen="+currentScreen, true); 
	xhr.send();		
}
// clicked a select box 
// MxVc 
function clickSel(element) 
{
	var xhr = new XMLHttpRequest();
	var con = element.id; //  control name
	var val = element.value;
	//console.log("clickSel:", con, val);
	xhr.open("GET", "/update?cmd=3&value_1="+con+"&value_2="+ val+"&screen="+currentScreen, true); 
	xhr.send();		
}

function selSet(element)
{
	clearSettingBorders(0);
	el = document.getElementById(element.id);		
	//el.style.backgroundColor = "white";
	el.style.border = "1px solid red";
	el.blur();
  //console.log("SS");
}

function clearSettingBorders(side)
{
	//console.log("Clear setting Backgrounds", side, currentScreen);
  var i;
  var j;
  var contID;
  var maxVals;
  for(j = 0; j < MAXCONTS; j++)
  {
    if (j == CONTCHAN)
      maxVals = CONTVALS;
    else
      maxVals = MAXVALS;
    for(i = 0; i < maxVals; i++)
    {
        contID = "S" + i + "S" + j;
        var el = document.getElementById(contID);	
        el.style.border = "none";
    }
  }  
}

// Update the current value (each time you move the knob) CMD_MODEVAL
// specific for mode values: have to encode channel, set_num and value into ID
// value_1 is ID [nn][oo] : nn = channel,  oo = setting number (mode is implicit)
function setChange(element) {
    var x3hr = new XMLHttpRequest();
    var strx = "/update?cmd=4&value_1="+element.id+"&value_2="+element.value+"&screen="+currentScreen;
    x3hr.open("GET", strx, true); x3hr.send();
   // console.log("set Change",strx);
}

// start test button
function startTest(element) {
    var x3hr = new XMLHttpRequest();
    var strx = "/update?cmd=7&value_1="+element.id+"&value_2="+element.value+"&screen="+currentScreen;
    x3hr.open("GET", strx, true); x3hr.send();
   // console.log("set Change",strx);
}

var RegEx = new RegExp(/^-?\d*\.?\d*$/); // pos, neg floats or integers only

function selSetting(elem)
{
	settingToChange = elem; // this one gets changed by knob
	settingToChangeName = elem.name;
	//console.log("SSS:", settingToChangeName);
}

function valid(elem) 
{
  //console.log("Valid", elem.id)
	var val = elem.value;	
	var errLine = document.getElementById("input_errorID");
  fixVal(elem);
 
	if (RegEx.test(val)) 
  {
		errLine.value = " ";
		// should probably include tests here for max/min being present
		if (+val > +elem.max) 
			errLine.value = "Max = " + elem.max;
		if (+val < +elem.min) 
			errLine.value = "Min = " + elem.min;
	} else 
  {
		//elem.value = val;
		errLine.value = "Illegal char.";
	}
}//end