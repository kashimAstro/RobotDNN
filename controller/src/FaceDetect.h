#include "ofMain.h"
#ifdef SHIFT
#undef SHIFT
#endif
#include <dlib/image_processing/frontal_face_detector.h>
#include <dlib/image_processing/render_face_detections.h>
#include <dlib/image_processing.h>
#include <dlib/image_io.h>
#include <dlib/clustering.h>
#include <dlib/string.h>
#include <dlib/dnn.h>

using namespace dlib;
using namespace std; 

class FaceDetect 
{
	public:
	ofxDelaunay triangulation;
	shape_predictor sp;
	frontal_face_detector detector;
	std::vector<matrix<rgb_pixel>> faces;
	array2d<rgb_pixel> img;
	std::vector<dlib::rectangle> fac;
        std::vector<dlib::full_object_detection> shapes;
	
	ofPixels toOf(const dlib::matrix<dlib::rgb_pixel> rgb)
	{
	    ofPixels p;
	    int w = rgb.nc();
	    int h = rgb.nr();
	    p.allocate(w, h, OF_IMAGE_COLOR);
	    for(int y = 0; y<h; y++)
	    {
		 for(int x=0; x<w;x++)
		 {
			p.setColor(x, y, ofColor(rgb(y,x).red,	
						 rgb(y,x).green,
						 rgb(y,x).blue));
		 }
	    }
	    return p;
	}

	dlib::array2d<dlib::rgb_pixel> toDLib(const ofPixels px)
	{
	    dlib::array2d<dlib::rgb_pixel> out;
	    int width = px.getWidth();
	    int height = px.getHeight();
	    int ch = px.getNumChannels();

	    out.set_size( height, width );
	    const unsigned char* data = px.getData();
	    for ( unsigned n = 0; n < height;n++ )
	    {
		const unsigned char* v =  &data[n * width *  ch];
		for ( unsigned m = 0; m < width;m++ )
		{
		    if ( ch==1 )
		    {
			unsigned char p = v[m];
			dlib::assign_pixel( out[n][m], p );
		    }
		    else{
			dlib::rgb_pixel p;
			p.red = v[m*3];
			p.green = v[m*3+1];
			p.blue = v[m*3+2];
			dlib::assign_pixel( out[n][m], p );
		    }
		}
	    }
	    return out;
	}

	void find(ofPixels p, int _size=150, float _padding=0.25)
	{		
		faces.clear();
		shapes.clear();

		img = toDLib(p);
	        fac = detector(img);
		for (auto face : fac)
		{
		        auto shape = sp(img, face);
		        matrix<rgb_pixel> face_chip;
		        extract_image_chip(img, get_face_chip_details(shape,_size,_padding), face_chip);
		        faces.push_back(move(face_chip));
		        shapes.push_back(shape);
    		}
	}

	void setup(string pred = "model/shape_predictor_68_face_landmarks.dat" )
	{
		detector = get_frontal_face_detector();
		deserialize(ofToDataPath(pred)) >> sp;
	}

	std::vector<ofImage> outc;
	std::vector<ofImage> getCrop()
	{
		return outc;
	}

	void draw(int _x, int _y)
	{
		outc.clear();
		ofPushStyle();
		ofPushMatrix();
		ofTranslate(_x,0,0);
                for(int i = 0; i < fac.size(); i++)
                {
                        ofRectangle posbound(fac[i].left(),fac[i].top(),fac[i].width(),fac[i].height());

			ofNoFill();
			ofPushMatrix();
   			ofImage img = toOf(faces[i]);
			ofSetColor(ofColor::white);
                        img.draw((i*img.getWidth()),_y);
			outc.push_back(img);
			ofPopMatrix();

                        ofSetColor(ofColor::orange);
			ofDrawRectRounded(posbound,10);
			triangulation.reset();
			for (int l = 0; l < shapes[i].num_parts(); l++)
			{
				ofFill();
				ofSetColor(ofColor::red);
				ofDrawCircle(shapes[i].part(l).x(),shapes[i].part(l).y(),shapes[i].part(l).z(),3);
				ofNoFill();
				ofSetColor(ofColor::white);
				ofDrawBitmapString(ofToString(l), shapes[i].part(l).x(),shapes[i].part(l).y(),shapes[i].part(l).z());
				ofSetColor(ofColor::green);
				triangulation.addPoint(ofPoint(shapes[i].part(l).x(),shapes[i].part(l).y()));
				triangulation.triangulate();
			}
			triangulation.draw();
		}
		ofPopStyle();
		ofPopMatrix();
	}
};
