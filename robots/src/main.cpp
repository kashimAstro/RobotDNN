#include "ofMain.h"
#include "ofAppNoWindow.h"
#include "ofxGPIO.h"
#include "ofxNetwork.h"

using namespace LogHighLight;

// Next article
//#include "compass.h"
//#include "gps.h"
//#include "bitmap.h"

#define STATE_FORWARD_BACK           0
#define STATE_LEFT_RIGHT             1
#define STATE_BIDIRECTION_LEFT_RIGHT 2
#define STATE_SERVO_MOTOR_MOVE       3
#define STATE_LEFT_RIGHT_DOWN        4

/*


*/

class MotorStepper : public ofBaseApp
{
        public:
		//GPS gps;
		//Compass comp;
		GPIO gpioRight[4];
		GPIO gpioLeft[4];
		long TimeVel  = 55000;
		long TimeGlob = 500;
		int  Direction,State;
                ofxTCPServer TCP;
		OLED oled;
		int port;
		string NAME_ROBOT;

		MotorStepper(string _port,string _name)
		{
			port = ofToInt(_port);	
			NAME_ROBOT=_name;
		}
 
		void setup()
		{
	                oled.setup(OLEDBG::BLACK);
       			oled.clearDisplay();
	                oled.setCursor(0,0);
                        oled.printString(strdup(NAME_ROBOT.c_str()));
			string ipwlan0 = ofSystem("/sbin/ifconfig wlan0 |/bin/grep 'inet addr'|/usr/bin/cut -d':' -f2|/usr/bin/cut -d' ' -f1");
			oled.setCursor(0,1);
                        oled.printString(strdup(ipwlan0.c_str()));


			TCP.setup(port);
		        TCP.setMessageDelimiter("\n");
			//comp.setup(&TCP);
	                //gps.setup(&TCP,"/dev/ttyAMA0",9600);
			ofFile config_pin_motor("config_pi_motor.txt", ofFile::ReadOnly, false);
			if(!config_pin_motor.exists())
			{
				ofLog()<<"configure motor pin file: bin/data/config_pi_motor.txt";
			}
			string conf_buff = config_pin_motor.readToBuffer().getText();
			vector<string> numsp = ofSplitString(conf_buff,"\n");
			vector<string> pins_right = ofSplitString(numsp[0],":");
			vector<string> pins_left  = ofSplitString(numsp[1],":");

                        vector<string> pins_list_right = ofSplitString(pins_right[1],",");
                        vector<string> pins_list_left  = ofSplitString(pins_left[1],",");
			ofLog()<<"Right::"<<pins_list_right[0]<<" "<<pins_list_right[1]<<" "<<pins_list_right[2]<<" "<<pins_list_right[3];
			ofLog()<<"Left::"<<pins_list_left[0]<<" "<<pins_list_left[1]<<" "<<pins_list_left[2]<<" "<<pins_list_left[3];

			//string pinRight[4] = {"4","17","27","22"};
			string pinRight[4] = {pins_list_right[0], pins_list_right[1], pins_list_right[2], pins_list_right[3]};
			for( unsigned int i = 0; i < 4; i++ )
			{
	    			gpioRight[i].setup(pinRight[i]);
				gpioRight[i].export_gpio();
	                        gpioRight[i].setdir_gpio("out");
			}

			//string pinLeft[4] = {"6","13","19","26"};
			string pinLeft[4] = {pins_list_left[0], pins_list_left[1], pins_list_left[2], pins_list_left[3]};
			for( unsigned int i = 0; i < 4; i++ )
			{
    				gpioLeft[i].setup(pinLeft[i]);
				gpioLeft[i].export_gpio();
	                        gpioLeft[i].setdir_gpio("out");
			}
			Log("Setup Robot: "+NAME_ROBOT,FG_WHITE,BG_RED)<<"\n";
		}

		void rotorLeftRightDown(long velocity, int direction)
		{
			if( direction )
			{
				usleep(velocity);
				string pin4[] = {"0","1","0","1"};
	                        for( unsigned int i = 0; i < 4; i++ ) gpioRight[i].setval_gpio(pin4[i]);
				usleep(velocity);
				string pin3[] = {"0","1","1","0"};
	                        for( unsigned int i = 0; i < 4; i++ ) gpioRight[i].setval_gpio(pin3[i]);
				usleep(velocity);
				string pin2[] = {"1","0","1","0"};
	                        for( unsigned int i = 0; i < 4; i++ ) gpioRight[i].setval_gpio(pin2[i]);
				usleep(velocity);
				string pin1[] = {"1","0","0","1"};
	                        for( unsigned int i = 0; i < 4; i++ ) gpioRight[i].setval_gpio(pin1[i]);
			}
			else
			{
				usleep(velocity);
				string pin4[] = {"0","1","0","1"};
				for( unsigned int i = 0; i < 4; i++ ) gpioLeft[i].setval_gpio(pin4[i]);
				usleep(velocity);
				string pin3[] = {"0","1","1","0"};
				for( unsigned int i = 0; i < 4; i++ ) gpioLeft[i].setval_gpio(pin3[i]);
				usleep(velocity);
				string pin2[] = {"1","0","1","0"};
				for( unsigned int i = 0; i < 4; i++ ) gpioLeft[i].setval_gpio(pin2[i]);
				usleep(velocity);
				string pin1[] = {"1","0","0","1"};
				for( unsigned int i = 0; i < 4; i++ ) gpioLeft[i].setval_gpio(pin1[i]);
			}
		}
		
		void rotorLeftRight(long velocity, int direction)
		{
			if( direction )
			{
				usleep(velocity);
				string pin1[] = {"1","0","0","1"};
	                        for( unsigned int i = 0; i < 4; i++ ) gpioRight[i].setval_gpio(pin1[i]);
				usleep(velocity);
				string pin2[] = {"1","0","1","0"};
	                        for( unsigned int i = 0; i < 4; i++ ) gpioRight[i].setval_gpio(pin2[i]);
				usleep(velocity);
				string pin3[4] = {"0","1","1","0"};
	                        for( unsigned int i = 0; i < 4; i++ ) gpioRight[i].setval_gpio(pin3[i]);
				usleep(velocity);
				string pin4[4] = {"0","1","0","1"};
	                        for( unsigned int i = 0; i < 4; i++ ) gpioRight[i].setval_gpio(pin4[i]);
			}
			else
			{
				usleep(velocity);
				string pin1[4] = {"1","0","0","1"};
				for( unsigned int i = 0; i < 4; i++ ) gpioLeft[i].setval_gpio(pin1[i]);
				usleep(velocity);
				string pin2[4] = {"1","0","1","0"};
				for( unsigned int i = 0; i < 4; i++ ) gpioLeft[i].setval_gpio(pin2[i]);
				usleep(velocity);
				string pin3[4] = {"0","1","1","0"};
				for( unsigned int i = 0; i < 4; i++ ) gpioLeft[i].setval_gpio(pin3[i]);
				usleep(velocity);
				string pin4[4] = {"0","1","0","1"};
				for( unsigned int i = 0; i < 4; i++ ) gpioLeft[i].setval_gpio(pin4[i]);
			}
		}

		void rotorForwardBack(long velocity,int direction)
		{
			if( direction )
			{
				usleep(velocity);
				string pin1[4] = {"1","0","0","1"};
		                for( unsigned int i = 0; i < 4; i++ ) {
					gpioRight[i].setval_gpio(pin1[i]);
					gpioLeft[i].setval_gpio(pin1[i]);
				}
				usleep(velocity);
				string pin2[4] = {"1","0","1","0"};
		                for( unsigned int i = 0; i < 4; i++ ) {
					gpioRight[i].setval_gpio(pin2[i]);
					gpioLeft[i].setval_gpio(pin2[i]);
				}
				usleep(velocity);
				string pin3[4] = {"0","1","1","0"};
		                for( unsigned int i = 0; i < 4; i++ ) {
					gpioRight[i].setval_gpio(pin3[i]);
					gpioLeft[i].setval_gpio(pin3[i]);
				}
				usleep(velocity);
				string pin4[4] = {"0","1","0","1"};
		                for( unsigned int i = 0; i < 4; i++ ) {
					gpioRight[i].setval_gpio(pin4[i]);
					gpioLeft[i].setval_gpio(pin4[i]);
				}
			}
			else
			{
				usleep(velocity);
				string pin1[4] = {"0","1","0","1"};
		                for( unsigned int i = 0; i < 4; i++ ) {
					gpioRight[i].setval_gpio(pin1[i]);
					gpioLeft[i].setval_gpio(pin1[i]);
				}
				usleep(velocity);
				string pin2[4] = {"0","1","1","0"};
		                for( unsigned int i = 0; i < 4; i++ ) {
					gpioRight[i].setval_gpio(pin2[i]);
					gpioLeft[i].setval_gpio(pin2[i]);
				}
				usleep(velocity);
				string pin3[4] = {"1","0","1","0"};
		                for( unsigned int i = 0; i < 4; i++ ) {
					gpioRight[i].setval_gpio(pin3[i]);
					gpioLeft[i].setval_gpio(pin3[i]);
				}
				usleep(velocity);
				string pin4[4] = {"1","0","0","1"};
		                for( unsigned int i = 0; i < 4; i++ ) {
					gpioRight[i].setval_gpio(pin4[i]);
					gpioLeft[i].setval_gpio(pin4[i]);
				}
			}
		}

		void rotorBidirectionLeftRight(long velocity,int direction)
		{
			if( direction )
			{
				usleep(velocity);
				string pin1a[4] = {"1","0","0","1"};
				string pin1b[4] = {"0","1","0","1"};
		                for( unsigned int i = 0; i < 4; i++ ) {
					gpioRight[i].setval_gpio(pin1a[i]);
					gpioLeft[i].setval_gpio(pin1b[i]);
				}
				usleep(velocity);
				string pin2a[4] = {"1","0","1","0"};
				string pin2b[4] = {"0","1","1","0"};
		                for( unsigned int i = 0; i < 4; i++ ) {
					gpioRight[i].setval_gpio(pin2a[i]);
					gpioLeft[i].setval_gpio(pin2b[i]);
				}
				usleep(velocity);
				string pin3a[4] = {"0","1","1","0"};
				string pin3b[4] = {"1","0","1","0"};
		                for( unsigned int i = 0; i < 4; i++ ) {
					gpioRight[i].setval_gpio(pin3a[i]);
					gpioLeft[i].setval_gpio(pin3b[i]);
				}
				usleep(velocity);
				string pin4a[4] = {"0","1","0","1"};
				string pin4b[4] = {"1","0","0","1"};
		                for( unsigned int i = 0; i < 4; i++ ) {
					gpioRight[i].setval_gpio(pin4a[i]);
					gpioLeft[i].setval_gpio(pin4b[i]);
				}
			}
			else
			{
				usleep(velocity);
				string pin1a[4] = {"1","0","0","1"};
				string pin1b[4] = {"0","1","0","1"};
		                for( unsigned int i = 0; i < 4; i++ ) {
					gpioLeft[i].setval_gpio(pin1a[i]);
					gpioRight[i].setval_gpio(pin1b[i]);
				}
				usleep(velocity);
				string pin2a[4] = {"1","0","1","0"};
				string pin2b[4] = {"0","1","1","0"};
		                for( unsigned int i = 0; i < 4; i++ ) {
					gpioLeft[i].setval_gpio(pin2a[i]);
					gpioRight[i].setval_gpio(pin2b[i]);
				}
				usleep(velocity);
				string pin3a[4] = {"0","1","1","0"};
				string pin3b[4] = {"1","0","1","0"};
		                for( unsigned int i = 0; i < 4; i++ ) {
					gpioLeft[i].setval_gpio(pin3a[i]);
					gpioRight[i].setval_gpio(pin3b[i]);
				}
				usleep(velocity);
				string pin4a[4] = {"0","1","0","1"};
				string pin4b[4] = {"1","0","0","1"};
		                for( unsigned int i = 0; i < 4; i++ ) {
					gpioLeft[i].setval_gpio(pin4a[i]);
					gpioRight[i].setval_gpio(pin4b[i]);
				}
			}

		}

		void update()
		{
			for(unsigned int i = 0; i < (unsigned int)TCP.getLastID(); i++)
			{
				if( !TCP.isClientConnected(i) )continue;
				string message;
				message = TCP.receive(i);
				if(message!="")
				{
					std::size_t found = message.find("name");
					if (found!=std::string::npos)
					{
						vector<string> st = ofSplitString(message,":");
						oled.clearDisplay();
						oled.setCursor(0,0);
			                        oled.printString(strdup("Recognition:"));
						oled.setCursor(0,1);
			                        oled.printString(strdup(st[1].c_str()));

					}
					vector<string> split = ofSplitString(message,",");
					if( split.size() > 1 ) 
					{
						State     = ofToInt(split[0]);
						TimeVel   = ofToInt(split[1]);
						Direction = ofToInt(split[2]);
						string str = split[0]+"-"+split[1]+"-"+split[2];
						Log(NAME_ROBOT+":",FG_GREEN,BG_BLACK);
						Log("protocol["+str+"]",FG_MAGENTA,BG_BLUE)<<"\n";
						if(State == STATE_FORWARD_BACK)
						{
							rotorForwardBack(TimeVel,Direction);
							Log(NAME_ROBOT+":",FG_YELLOW,BG_BLACK) << "forward-backward" <<"\n";
						}
						if(State == STATE_LEFT_RIGHT)
						{
							rotorLeftRight(TimeVel,Direction);
							Log(NAME_ROBOT+":",FG_YELLOW,BG_BLACK) << "left-right" <<"\n";
						}
						if(State == STATE_LEFT_RIGHT_DOWN)
						{
							rotorLeftRightDown(TimeVel,Direction);
							Log(NAME_ROBOT+":",FG_YELLOW,BG_BLACK) << "left-right-down" <<"\n";
						}
						if(State == STATE_BIDIRECTION_LEFT_RIGHT)
						{
							rotorBidirectionLeftRight(TimeVel,Direction);
							Log(NAME_ROBOT+":",FG_YELLOW,BG_BLACK) << "bidirection-left-right" <<"\n";
						}
						/*if(State == STATE_SERVO_MOTOR_MOVE)
						{
							if(TimeVel==0)
							{
								//servo.set(0);
								//Log(NAME_ROBOT+":",FG_YELLOW,BG_BLACK) << "servo-motor: up" <<"\n";
							}
							else
							{
								//servo.set(130);
								//Log(NAME_ROBOT+":",FG_YELLOW,BG_BLACK) << "servo-motor: down" <<"\n";
							}
						}*/
					}
				}
			}
			ofSleepMillis(100);
		}

		void exit()
		{
			//comp.exit();
			//gps.exit();
			for(unsigned int i = 0; i < 4; i++)
			{
				gpioRight[i].unexport_gpio();
				gpioLeft[i].unexport_gpio();
			}
		}

};

int main(int argc, char** argv)
{
	if(argc > 2)
	{
	        ofAppNoWindow window;
		ofSetupOpenGL(&window, 1024,768, OF_WINDOW);
		ofRunApp( new MotorStepper(argv[1],argv[2]) );
	}
	else
	{
		ofLog()<<"Error parameter: port name-robot";
	}
}
