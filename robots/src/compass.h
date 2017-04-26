#include "ofThread.h"
#include <iostream>
#include <math.h>

#define COMPASS 0x1e
/*

https://cdn-shop.adafruit.com/datasheets/HMC5883L_3-Axis_Digital_Compass_IC.pdf

Write CRA (00) – send 0x3C 0x00 0x70 (8-average, 15 Hz default, normal measurement)
Write CRB (01) – send 0x3C 0x01 0xA0 (Gain=5, or any other desired gain)
Write Mode (02) – send 0x3C 0x02 0x00 (Continuous-measurement mode)
Wait 6 ms or monitor status register or DRDY hardware interrupt pin
Loop
	Send 0x3D 0x06 (Read all 6 bytes. If gain is changed then this data set is using previous gain)
	Convert three 16-bit 2’s compliment hex values to decimal values and assign to X, Z, Y, respectively.
	Send 0x3C 0x03 (point to first data register 03)
	Wait about 67 ms (if 15 Hz rate) or monitor status register or DRDY hardware interrupt pin
End_loop

---------------------------------------------------------------------------------------------------------------------

Write CRA (00) – send 0x3C 0x00 0x71 (8-average, 15 Hz default, positive self test measurement)
Write CRB (01) – send 0x3C 0x01 0xA0 (Gain=5)
Write Mode (02) – send 0x3C 0x02 0x00 (Continuous-measurement mode)
Wait 6 ms or monitor status register or DRDY hardware interrupt pin

Loop
	Send 0x3D 0x06 (Read all 6 bytes. If gain is changed then this data set is using previous gain)
	Convert three 16-bit 2’s compliment hex values to decimal values and assign to X, Z, Y, respectively.
	Send 0x3C 0x03 (point to first data register 03)
	Wait about 67 ms (if 15 Hz rate) or monitor status register or DRDY hardware interrupt pin
End_loop

Check limits –
If all 3 axes (X, Y, and Z) are within reasonable limits (243 to 575 for Gain=5, adjust these limits basing on the
	gain setting used. See an example below.) Then
	All 3 axes pass positive self test
	Write CRA (00) – send 0x3C 0x00 0x70 (Exit self test mode and this procedure)
Else
If Gain<7
	Write CRB (01) – send 0x3C 0x01 0x_0 (Increase gain setting and retry, skip the next data set)
Else
	At least one axis did not pass positive self test
	Write CRA (00) – send 0x3C 0x00 0x70 (Exit self test mode and this procedure)
End If

*/

class Compass : public ofThread
{
        public:
                I2CBus * busCompass;
                int compassRawData[3];
                ofxTCPServer *TCP;
                float heading;
		string strMsg;
		string NAME_ROBOT;

                void setup( ofxTCPServer *_TCP , string _name)
		{
			TCP=_TCP;
			NAME_ROBOT=_name;
                        busCompass = new I2CBus("/dev/i2c-1");
                        busCompass->addressSet(COMPASS);
			busCompass->writeByte(0, 0b01110000); // Set to 8 samples @ 15Hz
                        busCompass->writeByte(1, 0b00100000); // 1.3 gain LSb / Gauss 1090 (default)
                        busCompass->writeByte(2, 0b00000000); // Continuous sampling

			ofSleepMillis(20);
			startThread();
                }

		void exit()
		{
			stopThread();
		}

		string getString()
		{
			return strMsg;
		}

		void threadedFunction() {
		        while(isThreadRunning()) {
	                        uint8_t block[6];

                        	busCompass->readBlock(0x80 | 0x04, sizeof(block), block);
	                        compassRawData[0] = (int16_t)(block[0] | block[1] << 8);
	                        compassRawData[1] = (int16_t)(block[2] | block[3] << 8);
	                        compassRawData[2] = (int16_t)(block[4] | block[5] << 8);
				compassRawData[1] = -compassRawData[1];
	                        heading = 180 * atan2(compassRawData[1],compassRawData[0])/M_PI;
        	                if(heading < 0)
	                              heading += 360;
	                        string msg=ofToString(compassRawData[0])+
        	                       ","+ofToString(compassRawData[1])+
	                               ","+ofToString(compassRawData[2])+
	                               ","+ofToString(heading)+"\n";
				strMsg = msg;
				for(int i = 0; i < TCP->getLastID(); i++)
				{
		                        if( !TCP->isClientConnected(i) ) continue;
		                        TCP->send(i,msg);
                                        Log(NAME_ROBOT+":",FG_RED,BG_BLACK);
					Log("coord: ",FG_BLACK,BG_WHITE) << msg;
				}
				ofSleepMillis(500);
			}
                }
};
