/* LITERAL HTML */
#ifndef MYLITERAL_H
#define MYLITERAL_H
// main page code - settings body with a knob
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <title>DDS</title>
  <meta name="viewport" id="viewport" content="width=825, initial-scale=1, user-scalable=yes, minimum-scale=0.3">
 <link rel="stylesheet" href="DDS.css"> 
</head>
<body onload="setScreen()" id="bodyS"> 
<div class="pg_container" id="plotContOuter" style="width: 820px; height: 700;">
  <div class="buttonTop"> 
    <input type="hidden" id="screenName" class="headlineS" value ="Main" style="pointer-events: none; text-align: center;">
      %SCREENBUTTONS% 
  </div>    
  <div class="tm_container" id="setCont" style="width: 820px; height: 350px;">     
     <div class="row" id="setRow" style="width: 820px; height: 325px;">                   
        %SETTINGS%
     </div> 
     <div class="row" id="errorRow" style="width: 820px; height: 90px; background-color: #ccc;">  
        <input type="text" maxlength="90" size="60" class="errline" readonly name="input_error" id="input_errorID" value="text" style="color: red;">       
     </div> 
   </div>
</div>
<script src="DDS.js"></script>
<script src="jogDial.min.js"></script>  
</body>
</html>
)rawliteral";
/*
<input type="text" id="input_errorID" readonly class="blinking" value ="" style="color: red"><BR>
 <label for="fname">Save as: </label> 
      <input type=type='text' id="fname" name="fname" value="loadLog"> 
      <button type="button" onclick="saveLog();">Click to Save Plot</button><br><br>  
      <textarea id="outLog" rows="4" cols="40">Log data will load here and then save as CSV using the filename above.</textarea>
     <br>
  */
// log page code
/*
const char settings_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <title>DDS</title>
  <meta name="viewport" id="viewport" content="width=device-width, initial-scale=1">
 <link rel="stylesheet" href="DDS.css"> 
</head>
<body onload="setScreen()" id="bodyS">
	<div class="pg_container">
    <div class="buttonTop">
      <input type="text" readonly id="devName" class="headlineL" size=5 value ="XXX">
      <input type="hidden" id="screenName" class="headlineS" value ="Log" style="pointer-events: none; text-align: center;">
      %SCREENBUTTONS% LLL
    </div>
    <div class="tm_container">   
      %SETTINGS%
      </div> 
  <input type="text" id="input_errorID" readonly class="blinking" value ="" style="color: red"><BR>
  </div>

  <script src="DDS.js"></script>   
   <script src="jogDial.min.js"></script>  
    <!--script src="FileSaver.js"></script-->
</body>
</html>
)rawliteral";
*/
/*
		<input type="text" hidden readonly id="devName" class="headlineS" size=4 value="XXX">
     <h3>Load Preset</h3>
      <label for="getFilefile">Preset file to load: </label>
      <input id='getFile' type='file' onchange='openFile(event)'><BR><br>
      <textarea id="inJSON" rows="6" cols="60">JSON preset from file will load here, and then be activated.</textarea>
	  

*/

/*
		  %SETTINGSGROUP%
	  <div class="bm_container">

	  </div> <BR> 
	  
	  
	   <input type="text" readonly id="errorText" class="blinking" value ="" style="color: red"><BR>
	  */
#endif
