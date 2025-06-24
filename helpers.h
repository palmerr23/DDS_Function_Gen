#ifndef LHELPERS_H
#define LHELPERS_H

// SIGN function
#define SIGN(X) (((X) < 0) ? -1 : 1)
#define bitInvert(value, bit) ((value) ^= (1UL << (bit)))
#define isBitSet(value, bit) ((value) & (1UL << (bit)))

static float mapf(float x, float in_min, float in_max, float out_min, float out_max)
{
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

// ---------------------------------------------------------------------------- 
// crc8
//
// Copyright (c) 2002 Colin O'Flynn
// Minor changes by M.Thomas 9/2004 
// ----------------------------------------------------------------------------

#define CRC8INIT    0x00
#define CRC8POLY    0x18              //0X18 = X^8+X^5+X^4+X^0

// ----------------------------------------------------------------------------

uint8_t crc8(uint8_t *data, uint16_t data_length) {
  uint8_t  b;
  uint8_t  bit_counter;
  uint8_t  crc = CRC8INIT;
  uint8_t  feedback_bit;
  uint16_t loop_count;
  
  for (loop_count = 0; loop_count != data_length; loop_count++) {
    b = data[loop_count];
    bit_counter = 8;

    do {
      feedback_bit = (crc ^ b) & 0x01;

      if (feedback_bit == 0x01) {
        crc = crc ^ CRC8POLY;
      }

      crc = (crc >> 1) & 0x7F;

      if (feedback_bit == 0x01) {
        crc = crc | 0x80;
      }

      b = b >> 1;
      bit_counter--;

    } while (bit_counter > 0);
  }
  
  return crc;
}


#define VAL_CHGD -1
#define VAL_CHGD_LOCAL    1       // refresh screen
#define VAL_CHGD_LOCAL_VALS    2  // just refresh values
#define VAL_CHGD_LOCAL_NOUPD 3    // don't refresh screen
#define VAL_CHGD_NOEE   4
#define VAL_CHGD_REMOTE 5 // SCPI
#define VAL_CHGD_WEB    6
void valChange(int16_t cmd)
{
	//Serial.println("VAl-CHG");
	switch (cmd) 
	{
		case VAL_CHGD_LOCAL: // everything gets redrawn
 			updateVal_Scrn = true;
    case VAL_CHGD_LOCAL_VALS: // just the values get redrawn
      updateVal_DDS = true;
      updateVal_Vals = true;
      updateVal_EE = true;
      updateVal_web = true;
      fifoCmd = fifoCmd | VAL_CHANGE_A | VAL_CHANGE_B;
      //updateVal_remote = true; // not sure if this is needed
      break;
    case VAL_CHGD_LOCAL_NOUPD: // no screen update (e.g. switches)
      updateVal_DDS = true;			
			updateVal_EE = true;
      updateVal_web = true;
      fifoCmd = fifoCmd | VAL_CHANGE_A | VAL_CHANGE_B;
      break;
		case VAL_CHGD_NOEE:		// Also TFT? Not a stored value (e.g. run/stop)
			updateVal_Scrn = true;
      updateVal_DDS = true;
      // updateVal_EE = true;
      updateVal_web = true;
			//updateVal_remote = true; // not sure if this is needed
      fifoCmd = fifoCmd | VAL_CHANGE_A | VAL_CHANGE_B;
			break;
		case VAL_CHGD_REMOTE:
			updateVal_Scrn = true; 
      updateVal_DDS = true;
			updateVal_EE = true;
      updateVal_web = true;
			//updateVal_remote = true; // not sure if this is needed
      // fifoCmd set in calling routine
			break;
    case VAL_CHGD_WEB:
      updateVal_DDS = true;
      updateVal_Scrn = true;
      updateVal_EE = true;
      //updateVal_remote = true; // not sure if this is needed
      // fifoCmd set in calling routine
      break;
		default:
		  break;
	}	
	//dynSet = iSet; // update live control set
}
IPAddress cidr2netmask(uint8_t cidr){
	uint8_t temp;
  uint32_t nm = 0xffffffff;
	IPAddress netmask = IPAddress(nm << (32 - cidr)); // in reverse byte order
	// reverse byte order
	temp = netmask[3];
	netmask[3] = netmask[0];
	netmask[0] = temp;
	
	temp = netmask[2];
	netmask[2] = netmask[1];
	netmask[1] = temp;
	
	return netmask;
}
float myRound(float num, int digits)
{	
	return round(num * pow(10, digits)) / pow(10, digits);
}
bool insideF(float val, float lowLim, float highLim)
{
  if (val < lowLim)
    return false;
  if (val > highLim)
    return false;
  return true;
}
bool insideI(int val, int lowLim, int highLim)
{
  if (val < lowLim)
    return false;
  if (val > highLim)
    return false;
  return true;
}

int getSerialChar(void)
{
  int retVal = -1;
  Serial.setTimeout(2); // short timeout to not block code
  if(!Serial.available())
    return -1;
  while(Serial.available())
  {
    byte inByte = Serial.read();
    if(inByte == '1')
      retVal = 1;
    if(inByte == '0')
      retVal = 0;
  }
  if(retVal >= 0) Serial.printf("Ser %i\n",retVal);
  return retVal;
}

// get luminance of an rgb16 value

// ignore low bit of G
struct rgbS {
  uint16_t b : 5;
  uint16_t g : 6;
  uint16_t r : 5;
};
union rgbU {
  uint16_t rgb16;
  rgbS rgbX;
};
#define MAXLUM (31*3)
int lum(int rgb16)
{
  int tmp = rgb16 & 0x1F; // B
  rgb16 = rgb16 >> 6; // kill low bit of G
  tmp += rgb16 & 0x1F;
  rgb16 = rgb16 >> 5; 
  tmp += rgb16 & 0x1F;
  return tmp;
}
// lum: (0.299*R + 0.587*G + 0.114*B)
#define MAXLUMV (310)
int lumVis(int rgb16){
  int tmp = rgb16 & 0x1F; // B
  rgb16 = rgb16 >> 6; // ignore low bit of G
  tmp += (rgb16 & 0x1F)*6;
  rgb16 = rgb16 >> 5; 
  tmp += (rgb16 & 0x1F)*3;
  return tmp;
}
#endif // HELPERS_H
