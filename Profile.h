/* Profile storage 
	EEPROM / LittleFS VERSION
	Factory reset data stored in File System on first boot, from compiled values
	Profile elements:
	* profile
  * iSet
  * comms
  * modeVal
  * sweepVal
  * screenCal
  * myWifi (extra wifi creds - are they ever used?)	
*/
#ifndef _PROFILE_H
#define _PROFILE_H

#include <ZEeprom_RP.h> // changed I2C write BUFFER_LENGTH

#define EEPROM_ADDRESS AT24Cxx_BASE_ADDR
ZEeprom * eeprom;
#define EE_PROFILE_ADDR 0

//#define PROF_TEST	// comment out for production - writes then reads back into dummy structures, compares

uint8_t checkProfileHdr(void);
void factory_reset(void);
void writeEE(void);
bool readEE(void);
bool setComms(bool create);
void printProfile();

struct profile {
  uint8_t crc;      // must be first in struct
  uint8_t softVer; 
  uint8_t eeVer;
  uint8_t hardVer; 
}  myProfile = {0, SOFT_VERSION, EE_VERSION, HARD_VERSION};

struct profile eeProfile;	// read EE into here to check against software version, etc

// store Profile first)
#define EE_MP_ADDR 0
#define MPSIZE        (sizeof(myProfile))
#define EE_SET_ADDR     (EE_MP_ADDR + MPSIZE)
#define SETSIZE       (sizeof(iSet))
#define EE_COMM_ADDR    (EE_SET_ADDR + SETSIZE)
#define COMMSIZE      (sizeof(comms))
#define EE_MOD_ADDR     (EE_COMM_ADDR + COMMSIZE)
#define MODSSIZE      (sizeof(modeVal))
#define EE_SWP_ADDR     (EE_MOD_ADDR + MODSSIZE)
#define SWPSIZE       (sizeof(sweepVal))
#define EE_CAL_ADDR     (EE_SWP_ADDR + SWPSIZE)
#define CALSIZE       (sizeof(vCal))
#define EE_SCAL_ADDR    (EE_CAL_ADDR + CALSIZE)  
#define SCALSIZE      (sizeof(screenCal))
//#define EE_WIFI_ADDR    (EE_SCAL_ADDR + SCALSIZE)
//#define WIFISIZE      (sizeof(myWiFi))
#define EESIZE          (EE_SCAL_ADDR + SCALSIZE - EE_MP_ADDR)

int eeBegin()
{
  //return false;
   //Serial.println("EEPROM start");

  myWire.setSDA(SDA1PIN);
  myWire.setSCL(SCL1PIN);
  myWire.setClock(I2CSPEED);

  eeprom= new ZEeprom();
  eeprom->begin(myWire, AT24Cxx_BASE_ADDR, AT24C256); // starts Wire
  
  // check for EEPROM
  myWire.beginTransmission(AT24Cxx_BASE_ADDR);
  EEpresent = (myWire.endTransmission() == 0);
  if(!EEpresent)
    return -1;
  if (checkProfileHdr()) 
  {      
    char buf[] = "Stored EEPROM\nprofile\nmismatch\nfactory reset";
      Serial.println(buf);
 
      screenError(buf, ERR_BG_ERR ,5, false); 
      writeEE();   
      delay(5000);
      rp2040.reboot();
      // never gets here!
      eeprom->readBytes(0, sizeof(eeProfile), (byte *)&eeProfile);
      if (checkProfileHdr()) 
      {
        Serial.println("Failed reset!");
        return -2; 
      }
  }
  //printProfile();
  readEE();
  //printProfile();
  return 0;
}

int EEcntr = 0;
bool saveSoon = false;
// called every VL from main loop
void saveSettings(void)
{	
	if(saveSoon)
			EEcntr++;
		
	if(updateVal_EE) // detect and reset change
	{
		saveSoon = true;
		//Serial.println("*** EE Changed");
		EEcntr = 0;	// new change? reset counter
    needToSaveEE = true;
		updateVal_EE = false;
	}

	if(EEcntr > SAVE_EE_AFTER)
	{
		EEcntr = 0;
		//Serial.print("*** Saving settings\n");
		setComms(false);
		writeEE();
    needToSaveEE = false;
		updateVal_EE = false;
		saveSoon = false;
	}		
}

void writeEE(void){
	myProfile.crc = crc8((uint8_t *)&myProfile.softVer, sizeof(myProfile) - 1);
//Serial.printf("Writing Profile, crc = %i, %i bytes @ %i secs\n  mo[000] = %3.2f, Swp[0][0] %3.2f,\n",     myProfile.crc, EESIZE, millis()/1000,  modeVal[0][0][0],sweepVal[0].val[0][1]);
 	eeprom->writeBytes(EE_MP_ADDR,   MPSIZE,   (byte *)&myProfile);
  eeprom->writeBytes(EE_SET_ADDR,  SETSIZE,  (byte *)&iSet);
  eeprom->writeBytes(EE_COMM_ADDR, COMMSIZE, (byte *)&comms);
	eeprom->writeBytes(EE_MOD_ADDR,  MODSSIZE, (byte *)&modeVal);
	eeprom->writeBytes(EE_SWP_ADDR,  SWPSIZE,  (byte *)&sweepVal);	
  eeprom->writeBytes(EE_CAL_ADDR,  CALSIZE,  (byte *)&vCal);	
	eeprom->writeBytes(EE_SCAL_ADDR, SCALSIZE, (byte *)&screenCal);	
	//eeprom->writeBytes(EE_WIFI_ADDR, WIFISIZE, (byte *)&myWiFi);
}

bool readEE(void)
{
  if(checkProfileHdr()) 
    return false;
	//Serial.printf("Profile: EE [%i], Soft %i, Hard %i\n", myProfile.eeVer, myProfile.softVer, myProfile.hardVer);	
  eeprom->readBytes(EE_MP_ADDR,   MPSIZE,   (byte *)&myProfile);
  eeprom->readBytes(EE_SET_ADDR,  SETSIZE,  (byte *)&iSet);
  eeprom->readBytes(EE_COMM_ADDR, COMMSIZE, (byte *)&comms);
	eeprom->readBytes(EE_MOD_ADDR,  MODSSIZE, (byte *)&modeVal);
	eeprom->readBytes(EE_SWP_ADDR,  SWPSIZE,  (byte *)&sweepVal);	
  eeprom->readBytes(EE_CAL_ADDR,  CALSIZE,  (byte *)&vCal);		
	eeprom->readBytes(EE_SCAL_ADDR, SCALSIZE, (byte *)&screenCal);	
	//eeprom->readBytes(EE_WIFI_ADDR, WIFISIZE,(byte *)&myWiFi);  
  //Serial.printf("  mo[000] = %3.2f, Swp[0][0] %3.2f,\n",  modeVal[0][0][0],sweepVal[0].val[0][1]);
	return true;
}

// put up hold screen - unplug to abort, touch to reset.
// if touched, write bad value to EEPROM and restart
int factReset(int)
{
  Serial.println("Factory Reset");
  delay(500); // avoid hangover touch    
  wasTouched = false;
  vlTimer = millis(); // delay EE write
  updateVal_EE = true;
  myProfile.eeVer = 255;  
  factoryReset = true;
  screenError("Factory reset:\nTouch screen\nto reset after\n30 secs.\n\nUnplug power\nto abort.", ERR_BG_ERR ,-1, false);  // hold until screen touch.
  return 0;
}

uint8_t checkProfileHdr(void){
  short i, crc;
  // Major software version, controlled hardware (MODEL) and CRC must be correct.
   eeprom->readBytes(EE_MP_ADDR, MPSIZE, (byte *)&eeProfile);
   //Serial.printf("EE Hdr %i, %i, %i\n",eeProfile.softVer,eeProfile.hardVer, eeProfile.eeVer);
   if(eeProfile.softVer != SOFT_VERSION)   
    {
      Serial.println("Profile check: Major software version difference.");
      Serial.print("Stored:  "); Serial.println(eeProfile.softVer);    
      Serial.print("Current: "); Serial.println(myProfile.softVer);
      return 1;
    }
	
    if(eeProfile.hardVer != HARD_VERSION)   
    {
      Serial.println("Profile check: Major hardware version difference.");
      Serial.print("Stored:  "); Serial.println(eeProfile.softVer);    
      Serial.print("Current: "); Serial.println(myProfile.softVer);
      return 2;
    }
    
  if (eeProfile.eeVer != EE_VERSION)
  {
    Serial.println("Profile check: Wrong hardware eeVer.");
    Serial.print("Stored:  "); Serial.println(eeProfile.eeVer);    
    Serial.print("Current: "); Serial.println(EE_VERSION);
    return 3;
  }
  /*  crc needs to be calculated from rest of  profile after CRC  */
  crc = crc8((uint8_t *)&eeProfile.softVer, sizeof(eeProfile) - 1);
  if (eeProfile.crc != crc)
  {
    Serial.println("CRC error.");
    Serial.print("Stored CRC:  "); Serial.println(eeProfile.crc);
    Serial.print("Calculated CRC: "); Serial.println(crc);
    return 4;
  }
  return 0;
}
void printProfile()
{
  Serial.printf("PrintProfile: EESIZE %i = MP %i + Comm %i + Mode %i  + Swp %i + vCal %i + sCal %i\n", EESIZE, MPSIZE, COMMSIZE, MODSSIZE, SWPSIZE, CALSIZE, SCALSIZE);
  Serial.printf("Print Profile: iSet: WF %i|%i, ", iSet.waveForm[0],iSet.waveForm[1]);
  Serial.printf("Cont Ph %3.2f, Bur CyOn %3.2f, Swp Sin Ini %3.2f, vCal %3.2f, ", modeVal[0][9][1], modeVal[0][8][0], sweepVal[0].val[0][1], vCal[0]);
  Serial.printf("CoupleOn %i, BisA %i\n ", BCOUPLE, BCOUPLE);
}

#endif
