#include "ofMain.h"
#include "ofxNetwork.h"
#include "UIController.h"
#include "ofxGui.h"
#include "ofxDelaunay.h"
#include "FaceDetect.h"
#include "FaceRecognition.h"
#include "ImageNet.h"

#define TIME_SAVE_IMAGE_MILLIS  5500

class RTSPFace : public ofBaseApp
{
    public:
	UIController ctrl;
	std::vector<ofTexture> tex;
	std::vector<std::shared_ptr<FaceDetect>> face;
	std::vector<std::shared_ptr<FaceRecognition>> face_rec;
	std::vector<std::shared_ptr<ImageNet>> image_predict;

	string TRUST_PERSON;
	ofFbo stage;

        std::vector<std::shared_ptr<ofGstVideoUtils>> gst;
        std::vector<std::shared_ptr<ofxTCPClient>> tcpClient;
	std::vector<string> argument;
	std::vector<string> path_photo;

        int w,h,indexconnection;
	bool connected;
	string ip;
	string port;
	string port_rotor;
	string strparsing;
	string tmpnn;
	ofImage imager;
	ofImage admin_person;

	ofVec3f compassCoord;
	float startT;
        float endT;

	ofxPanel gui;
	ofParameter<bool> enable_connect; 
	ofParameter<bool> enable_predict; 
	ofParameter<bool> enable_faces;
	ofParameter<bool> enable_joystick; 
	ofParameter<bool> enable_identify;
	ofParameter<int>  speed_motor; 
	ofParameter<int>  robot_num; 

        RTSPFace(std::vector<string> _argv)
	{
		indexconnection = 0;
		if( !_argv.empty() )
		{
			argument = _argv;
			for(int i = 0; i < argument.size(); i++)
			{
				ofLog()<<argument[i];
				std::vector<string> split = ofSplitString(argument[i],",");
				ip         += split[0]+",";
				port       += split[1]+",";
				port_rotor += split[2]+",";
				path_photo.push_back( split[0] );
				ofDirectory dir(split[0]);
				if(!dir.exists()){
				    dir.create(true);
				}
			}
		}
		else	
		{
			string str = "robot001.lan,5555,11999";
			path_photo.push_back( "robot001.lan" );

			argument.push_back(str);
			std::vector<string> split = ofSplitString(str,",");
		}
	}

	void connect(bool f)
	{
		if(f)
		{
			startT = ofGetElapsedTimeMillis();
			endT   = (int)TIME_SAVE_IMAGE_MILLIS;

			gst.clear();
			tex.clear();
			tcpClient.clear();
			
			std::shared_ptr<FaceDetect>      zface  = std::shared_ptr<FaceDetect>(new FaceDetect);
			std::shared_ptr<FaceRecognition> rface  = std::shared_ptr<FaceRecognition>(new FaceRecognition);
			std::shared_ptr<ImageNet>        predic = std::shared_ptr<ImageNet>(new ImageNet);

			zface->setup();
			rface->setup();
			predic->setup();

			for(int i = 0; i < argument.size(); i++)
			{
                              std::vector<string> split = ofSplitString(argument[i],",");
			      string _ip         = split[0];
			      string _port       = split[1];
			      string _port_rotor = split[2];
			      ofLog()<<"address:"<<_ip<<" port:"<<_port<<" port-motor-sensor:"<<_port_rotor;
			      ofTexture t;
		              t.allocate(w,h,GL_RGB);
                	      tex.push_back(t);
			      std::shared_ptr<ofGstVideoUtils> g = std::shared_ptr<ofGstVideoUtils>(new ofGstVideoUtils);
		              g->setPipeline("tcpclientsrc host="+_ip+" port="+_port+" ! gdpdepay ! rtph264depay ! avdec_h264 ! videoconvert",OF_PIXELS_RGB,true,w,h);
		              g->startPipeline();
                	      g->play();
                              gst.push_back(g);

		              std::shared_ptr<ofxTCPClient> tcp = std::shared_ptr<ofxTCPClient>(new ofxTCPClient);
		              tcp->setup(_ip.c_str(),ofToInt( _port_rotor));
			      tcp->setMessageDelimiter("\n");
                              tcpClient.push_back(tcp);
			      face.push_back(zface);
			      face_rec.push_back(rface);
			      image_predict.push_back(predic);
                        }
			ctrl.setup(tcpClient[indexconnection],ofVec2f(ofGetWidth()-200,20),"asset1",30);
			ctrl.setHoverButton(ofColor::grey, ofColor::red);

			connected = true;
			string msg = "3,1,0";
			ctrl.sendUpDown(msg);
		}
		else
		{
			ctrl.exit();
			for(int i = 0; i < gst.size(); i++) 
			{
				gst[i]->close();
				tcpClient[i]->close();
			}
			connected = false;
		}
	}

	void connects(bool & b)   { connect(b); }
	void speedmotor(int & v)  { ctrl.setSpeed(v); }
	void selectrobot(int & v) { indexconnection = (int) v; ctrl.exit(); ctrl.setup(tcpClient[indexconnection],ofVec2f(ofGetWidth()-200,20),"asset1",30);	}
	void imagenet(bool & b)   { image_predict[indexconnection]->startThread(true); }

        void setup() 
	{
		gui.setup();
		gui.add(enable_connect.set("enable connect", false));
		gui.add(enable_faces.set("enable faces detect",true));
		gui.add(enable_identify.set("enable identify",false));
		gui.add(enable_predict.set("enable predict", false));
		gui.add(enable_joystick.set("enable joystick",false));
		gui.add(speed_motor.set("speed motor",25000,0,100000));
		gui.add(robot_num.set("change robot",0,0,argument.size()));
		gui.setPosition(ofGetWidth()-(gui.getWidth()+10),ofGetHeight()-(gui.getHeight()+10));

		enable_connect.addListener(this,&RTSPFace::connects);
		enable_predict.addListener(this,&RTSPFace::imagenet);
		speed_motor.addListener(this,&RTSPFace::speedmotor);
		robot_num.addListener(this,&RTSPFace::selectrobot);

		w=320,h=240;

		/* compare face recognition */
		TRUST_PERSON="dario.jpg";
		admin_person.load(TRUST_PERSON);
                stage.allocate(admin_person.getWidth()*2,admin_person.getHeight(),GL_RGB);

		connected  = false;
        }

        void update()
	{
		ofSetWindowTitle(ofToString(ofGetFrameRate()));
		if(connected)
		{
			/* joystick control enable */
			if(enable_joystick) ctrl.joystick();

			/* read sensor robot */
			if(tcpClient[indexconnection]->isConnected())
			{
        			strparsing = tcpClient[indexconnection]->receive();
			        std::vector<string> splitp = ofSplitString(strparsing,",");
			        if(splitp.size()>1)
			        {
			    		compassCoord.set(ofToInt(splitp[0]),ofToInt(splitp[1]),ofToInt(splitp[2]));
			        }
			}

			/* gst update */
			for(int i = 0; i < gst.size(); i++)
		                gst[i]->update();
		}
        }

        void draw()
	{
		ofBackgroundGradient(ofColor::silver,ofColor::black);
		if(connected)
		{
			string info="";
			info+="Joystick:";
			info+=(enable_joystick?"enable":"disable");
			info+=" Face-detect:";
			info+=(enable_faces?"enable":"disable");

			/* draw gst */
			for(int i = 0; i < gst.size(); i++)
			{
				imager =  gst[i]->getPixels();
				imager.draw(i*(imager.getWidth()),0,imager.getWidth(),imager.getHeight());
				/* face detect */
				if(enable_faces) 
				{
					face[i]->find(gst[i]->getPixels());
					face[i]->draw(i*(imager.getWidth()),h);
				}
			}
		
			/* face recognition */
			if( enable_identify )
			{
				float timer = ofGetElapsedTimeMillis() - startT;
				if(timer >= endT) 
				{
					startT = ofGetElapsedTimeMillis();
					endT   = (int)TIME_SAVE_IMAGE_MILLIS;
					std::vector<ofImage> crop = face[indexconnection]->getCrop();

					for(int k = 0; k < crop.size(); k++)
					{
						stage.begin();
						ofClear(255,255,255,255);
						admin_person.draw(0,0);
						crop[k].draw(admin_person.getWidth(),0);
						stage.end();
						ofPixels pi;
						stage.readToPixels(pi);
						ofStringReplace(TRUST_PERSON,".jpg","");
						face_rec[indexconnection]->compare(TRUST_PERSON,pi);
					}					
				}
				string nn = face_rec[indexconnection]->getName();
				if(nn!=tmpnn)
				{
					tcpClient[indexconnection]->send("name:"+nn);
					ofLog()<<"name:"+nn;
				}
				tmpnn=nn;
				info+=" Face-recognition:"+tmpnn;
			}

			/* enable predict image-net */
			if(enable_predict)
			{
				info+=" Enable-prediction:";
				info+=(enable_predict?"enable":"disable");
				std::vector<string> pre = image_predict[indexconnection]->getPredict(imager.getPixels());
				for(int d = 0; d < pre.size(); d++)
				{
					ofDrawBitmapStringHighlight(pre[d],2,(h+150)+(d*24),ofColor::orange,ofColor::black);
				}
			}

			ofDrawBitmapStringHighlight(info,0,12);
			ctrl.draw();
		}
		gui.draw();
	}

	void exit()
	{
		ctrl.exit();
		for(int i = 0; i < gst.size(); i++) 
		{
			gst[i]->close();
			tcpClient[i]->close();
		}
	}

	void keyPressed(int key)
	{
		if(key == ' ')
		{
			ofSaveImage(
				imager.getPixels(), 
	                        path_photo[indexconnection]+"/"+ofGetTimestampString()+".jpg",
        	                OF_IMAGE_QUALITY_BEST
			);
		}
	}

	void windowResized(int w, int h)
	{
		if(connected)
		{
			ctrl.exit();
			ctrl.setup(tcpClient[indexconnection],ofVec2f(w-200,20),"asset1",30);
			ctrl.setHoverButton(ofColor::orange, ofColor::blue);
			gui.setPosition(w-(gui.getWidth()+10),h-(gui.getHeight()+10));
		}
	}

	void dragEvent(ofDragInfo dragInfo)
	{
		if(connected)	
		{
			for (int i=0; i<dragInfo.files.size(); i++)
			{
			        cout << dragInfo.files[i] << endl;
				ctrl.dataFile(dragInfo.files[i]);
			}
		}
	}
};

int main(int argc, char** argv)
{
        ofGLFWWindowSettings settings;
        settings.width = 1000;
        settings.height = 700;
        settings.setPosition(ofVec2f(0,0));
        settings.resizable = true;
        std::shared_ptr<ofAppBaseWindow> mainWindow = ofCreateWindow(settings);

        std::vector<string> _argument;
        for( int i = 1; i < argc; i++ )
        {
                _argument.push_back(argv[i]);
        }
        std::shared_ptr<RTSPFace> mainApp(new RTSPFace(_argument));
        ofRunApp(mainWindow, mainApp);
        ofRunMainLoop();
}
