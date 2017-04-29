#include "ofMain.h"
#include "ofxNetwork.h"
#include "ofxJoystick.h"

class UIController
{
    public:
	vector<ofImage> assetArrow;
	vector<ofRectangle> assetRect;
        std::shared_ptr<ofxTCPClient> tcpClient;
	ofColor cup,cdo;

	vector<ofRectangle> roundbox;
	ofxJoystick joy;
        int numButton;
        int valueButton;
        int numAxis;
        int valueAxis;

	bool axisX1;
	bool axisX2;
	bool stopressed;
        bool servo;
	int speed;

	ofFile data_log;

	void setHoverButton(ofColor _a, ofColor _b)
	{
		cup=_a;
		cdo=_b;
	}
	
	void setup(std::shared_ptr<ofxTCPClient> _tcpClient, ofVec2f pos, string folder="asset/", int scale=180)
	{
		axisX1 = false;
		axisX2 = false;
		stopressed = true;

		roundbox.push_back(ofRectangle((ofGetWidth()/2)-50,(ofGetHeight()/2)-150,100,100));
		roundbox.push_back(ofRectangle((ofGetWidth()/2)-50,(ofGetHeight()/2)-150,100,100));
		roundbox.push_back(ofRectangle((ofGetWidth()/2)-150,(ofGetHeight()/2)-150,100,100));
		roundbox.push_back(ofRectangle((ofGetWidth()/2)+150,(ofGetHeight()/2)-150,100,100));

		//if(joy.isThreadRunning())
		//	joy.exit();
		joy.setup("/dev/input/js0");
                if (!joy.isFound())
                    ofLog()<<"Joystick open Failed!";
                else
                    ofLog()<<"Joystick open Start!";
		servo = true;

		assetArrow.clear();
		assetRect.clear();
		tcpClient=_tcpClient;

		ofImage arUp,arLeft,arRight,arDown,arCenter,arCenterL,arCenterR;
		int px,py,pw,ph;
		arUp.load(folder+"/up_arrow.png");
		arLeft.load(folder+"/left_arrow.png");
		arRight.load(folder+"/right_arrow.png");
		arDown.load(folder+"/down_arrow.png");
		arCenter.load(folder+"/center_arrow.png");
		arCenterL.load(folder+"/center_arrow_right.png");
		arCenterR.load(folder+"/center_arrow_left.png");

		px=(arLeft.getWidth()-scale);py=0;pw=(arUp.getWidth()-scale);ph=(arUp.getHeight()-scale);
		assetRect.push_back(ofRectangle(px+pos.x,py+pos.y,pw,ph));
		assetArrow.push_back(arUp);

		px=0;py=(arUp.getHeight()-scale);pw=(arLeft.getWidth()-scale);ph=(arLeft.getHeight()-scale);
		assetRect.push_back(ofRectangle(px+pos.x,py+pos.y,pw,ph/2));
		assetArrow.push_back(arLeft);

		px=(arLeft.getWidth()-scale)+(arCenter.getWidth()-scale);py=(arUp.getHeight()-scale);pw=(arRight.getWidth()-scale);ph=(arRight.getHeight()-scale);
		assetRect.push_back(ofRectangle(px+pos.x,py+pos.y,pw,ph/2));
		assetArrow.push_back(arRight);

		px=(arLeft.getWidth()-scale);py=(arUp.getHeight()-scale)+(arCenter.getHeight()-scale);pw=(arDown.getWidth()-scale);ph=(arDown.getHeight()-scale);
		assetRect.push_back(ofRectangle(px+pos.x,py+pos.y,pw,ph));
		assetArrow.push_back(arDown);

		px=(arLeft.getWidth()-scale);py=(arUp.getHeight()-scale);pw=(arCenter.getWidth()-scale);ph=(arCenter.getHeight()-scale);
		assetRect.push_back(ofRectangle(px+pos.x,py+pos.y,pw/2,ph));
		assetArrow.push_back(arCenterL);

		assetRect.push_back(ofRectangle(px+pos.x+(pw/2),py+pos.y,pw/2,ph));
		assetArrow.push_back(arCenterR);

		px=0;py=(arUp.getHeight()-scale);pw=(arLeft.getWidth()-scale);ph=(arLeft.getHeight()-scale);
		assetRect.push_back(ofRectangle(px+pos.x,py+pos.y+(ph/2),pw,ph/2));
		assetArrow.push_back(arLeft);

		px=(arLeft.getWidth()-scale)+(arCenter.getWidth()-scale);py=(arUp.getHeight()-scale);pw=(arRight.getWidth()-scale);ph=(arRight.getHeight()-scale);
		assetRect.push_back(ofRectangle(px+pos.x,py+pos.y+(ph/2),pw,ph/2));
		assetArrow.push_back(arRight);

		speed = 25000;
		data_log.open(ofToDataPath(ofGetTimestampString()+"_data_log.dat"), ofFile::Append, false);
	}

	void exit()
	{
		data_log.close();
                joy.exit();
        }

	void sendUpDown(string _m)
	{
		tcpClient->send(_m);
	}

	void setSpeed(int _speed)
	{
		speed = _speed;
	}

	void dataFile(string path)
	{
		ofFile f;
		f.open(path, ofFile::ReadWrite, false);
		ofBuffer buff = f.readToBuffer();
		vector<string> d = ofSplitString(buff.getText(),"\n");
		for(int i = 0; i < d.size(); i++)
		{
			tcpClient->send(d[i]);
		}	
	}

	void joystick()
	{
		string message = "";
		numButton   = joy.getButtonNum();
                valueButton = joy.getButtonValue();
                numAxis     = joy.getAxisNum();
                valueAxis   = joy.getAxisValue();
		//
                if(numAxis == 0 && valueAxis<0 && !axisX1)
		{ 
			message="2,"+ofToString(speed)+",0"; tcpClient->send(message);
	 		data_log << message <<"\n";
			axisX1 = true;
		}
		else if(numAxis == 0 && valueAxis>0 && !axisX1)
		{
			message="2,"+ofToString(speed)+",1"; tcpClient->send(message);
	 		data_log << message <<"\n";
			axisX1 = true;
		}
		axisX1 = (numAxis == 0 && valueAxis<0 || valueAxis>0 ) ? true : false;

		//
                if(numAxis == 1 && valueAxis<0 && !axisX2)
		{
                        message="0,"+ofToString(speed)+",1"; tcpClient->send(message);
	 		data_log << message <<"\n";
			axisX2 = true;
		}
		else if(numAxis == 1 && valueAxis>0 && !axisX2)
		{
                        message="0,"+ofToString(speed)+",0"; tcpClient->send(message);
	 		data_log << message <<"\n";
			axisX2 = true;
		}
		axisX2 = (numAxis == 1 && valueAxis<0 || valueAxis>0 ) ? true : false;

		//
	        if(numButton == 0 && valueButton == 1)
		{ 
			message = "3,0,0"; tcpClient->send(message);
	 		data_log << message <<"\n";
		}
                else if(numButton == 1 && valueButton == 1) 
		{ 
			message = "3,1,0"; tcpClient->send(message);
	 		data_log << message <<"\n";
		}

		//
		if(0)
		{
			ofLog()<<"Button:";
	                ofLog()<<numButton;
	                ofLog()<<valueButton;
	                ofLog()<<"Axis:";
	                ofLog()<<numAxis;
	                ofLog()<<valueAxis;
	                ofLog()<<"******************";
		}
	}

	void draw()
	{
		for(int i = 0; i < assetArrow.size(); i++)
		{
			ofPushStyle();
			bool pressed = ofGetMousePressed(0);

			if( assetRect[i].inside(ofGetMouseX(),ofGetMouseY()) )
			{
				ofSetColor(cup);
			}
			if( pressed && assetRect[i].inside(ofGetMouseX(),ofGetMouseY()) && stopressed )
			{
				ofSetColor(cdo);
				string message;
				if(i == 0) { message="0,"+ofToString(speed)+",1"; }//------ up
				if(i == 1) { message="1,"+ofToString(speed)+",0"; }//------ left
				if(i == 2) { message="1,"+ofToString(speed)+",1"; }//------ right
				if(i == 3) { message="0,"+ofToString(speed)+",0"; }//------ down
				if(i == 4) { message="2,"+ofToString(speed)+",0"; }//center left
				if(i == 5) { message="2,"+ofToString(speed)+",1"; }//center right
				if(i == 6) { message="4,"+ofToString(speed)+",0"; }//down   right
				if(i == 7) { message="4,"+ofToString(speed)+",1"; }//down   left
				//ofLog()<<message;
				tcpClient->send(message);
				stopressed = false;
			}
			else if ( !pressed && !stopressed ){
				stopressed = true;
			}
			assetArrow[i].draw(assetRect[i].x,assetRect[i].y,assetRect[i].width,assetRect[i].height);
			ofPopStyle();
		}
	}
};
