#include "ofThread.h"
#include <iostream>
#include <math.h>

class GPS : public ofThread
{
        public:
                ofxTCPServer *TCP;
                GPSSerial gps;
                float time,lati,longi,alti;

                void setup( ofxTCPServer *_TCP, string dev, int baund )
                {
                        TCP=_TCP;
                        gps.start(dev,baund);
                        startThread();
                }

                void exit()
                {
                        stopThread();
                }

		int getTime(){ return time; }
		int getLati(){ return lati; }
		int getLong(){ return longi; }
		int getAlti(){ return alti; }

		void threadedFunction() 
		{
                        while(isThreadRunning()) 
			{
			     time  = gps.getTime();
                             lati  = gps.getLatitude();
                             longi = gps.getLongitude();
                             alti  = gps.getAltitude();
                             string msg = ofToString(time)+","+ofToString(lati)+","+ofToString(longi)+","+ofToString(alti);
			     for(int i = 0; i < TCP->getLastID(); i++)
                             {
                                        if( !TCP->isClientConnected(i) ) continue;
                                        TCP->send(i,msg);
                                        Log(ofToString(NAME_ROBOT)+":",FG_YELLOW,BG_BLACK);
                                        Log("GPS: ",FG_BLACK,BG_RED) << msg <<"\n";
                             }
                             ofSleepMillis(800);
			}
		}
};
