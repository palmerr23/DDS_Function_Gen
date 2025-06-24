/* myWiFi.h
basic WiFi for all instruments
- setup Wifi
*/
#ifndef MYWIFI_H
#define MYWIFI_H

#include <WiFi.h>
#include <WiFiMulti.h>
#include <WiFiUdp.h>
#include <TelnetStream.h> // https://github.com/jandrassy/TelnetStream // includes Stream.h
//ESPTelnet TelnetStream;
//#include <WiFiAP.h>
#include <WiFiClient.h>

#include <WiFiServer.h>
#include <LEAmDNS.h>
void setHostNameIP (void);
bool sendSSIDpacket(bool);
void updateWiFi(char * ss, char *pass, bool newCreds);
void printStoredWiFi();
void screenError(const char * message, uint16_t bgcol, int16_t onTime, bool logo);
void updateHostname(void);
void webServerStart();

WiFiMulti wifiMulti;
IPAddress nullIP = {0,0,0,0};
WiFiUDP Udp;

#define CONNWAIT 10

bool wifiBegin(void)
{ 
char psw[] = "";
char buf[256] = "";
char lanList[256] = "";
short i;
bool res;
   updateHostname(); 
   // optional WiFi creds
   File f = FILESYS.open("/wifiSecrets.JSON", "r");
   if(f.isFile())
   {
      JsonDocument doc;
      char json[200];      
      int siz = f.size();
      int rd;
      rd = f.readBytes(json, siz);
      DeserializationError error = deserializeJson(doc, json);
      if(!error)
      {
        const char *ss = doc["ssid"];
        const char* pass = doc["pass"];
        strcpy(comms.STA_ssid[0], ss);
        strcpy(comms.STA_pass[i], pass);
      }
      else
        screenError("Bad wifiSecrets.JSON file", ERR_BG_ERR, 1, false); 
   }
   else
     Serial.println("No wifiSecrets.JSON available");

   //Serial.print("Adding nets: ");
   for (i = 0; i < WIFI_STORED; i++) // Last first
      if(strlen(comms.STA_ssid[i]) > 0)
      {
        wifiMulti.addAP(comms.STA_ssid[i], comms.STA_pass[i]);       
        strcat(lanList, comms.STA_ssid[i]);
        strcat(lanList, "\n");         
        //Serial.printf("%s, ", comms.STA_ssid[i]);   
      }
      Serial.println();
	 //feedLoopWDT();			
   if(WiFi.status() == WL_CONNECTED) 
	   WiFi.disconnect();
	 //feedLoopWDT();
   wifiStatus = WIFI_DISCON;
	 WifiConnected = false;
  // Serial.println("**** Configuring WiFi *****");
   IamAP = false;
   // try connecting to saved networks
   Serial.printf("Trying existing\nWiFi LAN networks:\n%s\n", lanList);   
   sprintf(buf, "Trying saved\nSSIDs:\n%s", lanList);
   screenError(buf, ERR_BG_MSG, 1, false); 
   if(wifiMulti.run() != WL_CONNECTED) 
   {
      screenError("Failed to connect\nto saved LANs", ERR_BG_ERR, 5, false);      
   }
   else // normal connect to existing LAN
     if(WiFi.status() == WL_CONNECTED)
     {     
        String sss = WiFi.SSID();
        const char * ssp = sss.c_str();
        //strcpy(comms.STA_ssid, ssp);    
       
        myIP = WiFi.localIP();
        //myBroadcastIP = WiFi.broadcastIP();
        mySubnetMask = WiFi.subnetMask();
        //Serial.print("STA broadcast ");
        //Serial.println(myBroadcastIP);
       // Serial.printf("MDNS: starting [%s]\n", comms.instName);
        MDNS.begin(comms.instName);    
        String IPaddr = WiFi.localIP().toString();
        
        sprintf(buf, "Connected to\nlocal LAN\nSSID: %s\nHostname\nhttp://%s\nIP: %s", ssp, myHostName, IPaddr.c_str());        
        screenError(buf, ERR_BG_MSG, 5, false);
        setHostNameIP();
        wifiStatus = WIFI_CONN;
        WifiConnected = true;
        //	    strcpy(iSet.conn_ssid, comms.STA_ssid);

        //updateWiFi(ssp, psw, false); // no change, just register connection
        // printStoredWiFi();
        //WiFi.printDiag(Serial);
        return true;
      }
   // Failed LAN connect - set up local AP
   WiFi.disconnect();
   delay(500);

	  // set up soft AP 		 
	  sprintf(buf, "Failed to connect\nto stored SSIDs\nLaunching AP\n SSID: %s\nPass: %s", comms.AP_ssid, comms.AP_pass);
	  screenError(buf, ERR_BG_ERR, 5, false);	 
    //https://www.upesy.com/blogs/tutorials/how-create-a-wifi-acces-point-with-esp32
		delay(1000);  
    WiFi.mode(WIFI_AP);     
		if (WiFi.softAPConfig(myAP_IP, myAP_GATEWAY, myAP_SubnetMask))
		{    
      WiFi.softAP(AP_NETNAME, AP_NETPW);
      //WiFi.softAP("ABCD", "A123456789");
      myIP = WiFi.softAPIP();
      Serial.print("myIP: "); Serial.println(myIP);

      WiFi.setHostname(AP_NETNAME);
			//WiFi.printDiag(Serial);			
			//mySubnetMask = cidr2netmask(WiFi.softAPSubnetCIDR());
			IamAP = true;				
			MDNS.begin(comms.instName);
			setHostNameIP();
      wifiStatus = WIFI_SOFTAP;
      WifiConnected = true;
      Serial.printf("Hostname: %s\n",myHostName);
			return true;
		}	 
   screenError("WifFi AP start failed", ERR_BG_ERR, 5, false);
   //WiFi.printDiag(Serial);  
   return false;
}

void wifiEnd(bool wifioff = false){
	bool ret;
	Serial.printf("Wifi off\n");
  if(IamAP)
	  ret = WiFi.softAPdisconnect(wifioff);
	//Serial.printf("Soft AP disconnect %i\n", ret);
  else
	  ret = WiFi.disconnect(wifioff);
	//Serial.printf("STA disconnect %i\n", ret);
   WifiConnected = false;
}

void UDPstart(void)
{
  TelnetStream.begin(SCPI_PORT);
}
void updateHostname(void)
{
	// update hostname
	strcpy(myHostName, comms.instName);
	strcat(myHostName, ".local");
}
// set hostname and IP in string format
void setHostNameIP (void)
{
	updateHostname();		
	sprintf(IPstring, "%i.%i.%i.%i", myIP[0], myIP[1], myIP[2], myIP[3]);
	
	//Serial.printf("Hostname: (%s)\n",myHostName);
	//Serial.printf("IP: %s\n",IPstring);
}

bool netBegin(void)
{
      bl.t[0] = bl.t[1] = 3;// 300 mS flash
      bool ok = wifiBegin();
      if(!webServerOn)
        webServerStart();     
      UDPstart();
      bl.t[0] = 0;
      bl.t[1] = 10; // solid on
      return ok;
}

// make a copy of comms net credentials to test for change after OSK edit
// ********** unused???
/*
int copyLAN(int x) // button callback, argument not used
{
	//Serial.printf("copyLAN Copy lanPkt to comms\n"); 
	//strcpy(lanPkt.ssid, comms.STA_ssid);
	//strcpy(lanPkt.pass, comms.STA_pass);
	return CALL_EX;
}
*/
void printStoredWiFi(void)
{
  Serial.println("Stored WiFi STA:\n");
  for(int i = 0; i < WIFI_STORED; i++) 
    Serial.printf("%i: |%s| |%s|\n",i, comms.STA_ssid[i], comms.STA_pass[i]);
  Serial.printf("Last conn: %i\n",comms.lastConn);
  Serial.printf("AP: |%s| |%s|\n",i, comms.AP_ssid, comms.AP_pass);
}


#endif /* MYWIFI_H */
