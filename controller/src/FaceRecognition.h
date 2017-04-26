#include "ofMain.h"
#ifdef SHIFT
#undef SHIFT
#endif
#include <dlib/clustering.h>
#include <dlib/string.h>
#include <dlib/dnn.h>
#include <dlib/image_io.h>
#include <dlib/image_processing/frontal_face_detector.h>
#include <dlib/image_processing/render_face_detections.h>
#include <dlib/image_processing.h>

using namespace dlib;
using namespace std; 

template <template <int,template<typename>class,int,typename> class block, int N, template<typename>class BN, typename SUBNET>
using residual = add_prev1<block<N,BN,1,tag1<SUBNET>>>;

template <template <int,template<typename>class,int,typename> class block, int N, template<typename>class BN, typename SUBNET>
using residual_down = add_prev2<avg_pool<2,2,2,2,skip1<tag2<block<N,BN,2,tag1<SUBNET>>>>>>;

template <int N, template <typename> class BN, int stride, typename SUBNET>
using block  = BN<con<N,3,3,1,1,relu<BN<con<N,3,3,stride,stride,SUBNET>>>>>;

template <int N, typename SUBNET> using ares      = relu<residual<block,N,affine,SUBNET>>;
template <int N, typename SUBNET> using ares_down = relu<residual_down<block,N,affine,SUBNET>>;

template <typename SUBNET> using alevel0 = ares_down<256,SUBNET>;
template <typename SUBNET> using alevel1 = ares<256,ares<256,ares_down<256,SUBNET>>>;
template <typename SUBNET> using alevel2 = ares<128,ares<128,ares_down<128,SUBNET>>>;
template <typename SUBNET> using alevel3 = ares<64,ares<64,ares<64,ares_down<64,SUBNET>>>>;
template <typename SUBNET> using alevel4 = ares<32,ares<32,ares<32,SUBNET>>>;

using anet_type = loss_metric<fc_no_bias<128,avg_pool_everything<
                            alevel0<
                            alevel1<
                            alevel2<
                            alevel3<
                            alevel4<
                            max_pool<3,3,2,2,relu<affine<con<32,7,7,2,2,
                            input_rgb_image_sized<150>
                            >>>>>>>>>>>>;

class FaceRecognition : public ofThread
{
	public:

	shape_predictor sp;
	anet_type net;
	frontal_face_detector detector;
	std::vector<matrix<rgb_pixel>> faces;
	array2d<rgb_pixel> img;
	std::vector<dlib::rectangle> fac;
	ofImage p1;
	string name,result;

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

	int find(ofPixels p)
	{		
		img = toDLib(p);
	        fac = detector(img);
		faces.clear();
		for (auto face : fac)
		{
		        auto shape = sp(img, face);
		        matrix<rgb_pixel> face_chip;
		        extract_image_chip(img, get_face_chip_details(shape,150,0.25), face_chip);
		        faces.push_back(move(face_chip));
    		}
		return faces.size();
	}

	std::vector< std::vector<ofImage> > cluster()
	{
		std::vector< std::vector<ofImage> > images_cluster;
		if(faces.size() > 0)
		{
			std::vector<matrix<float,0,1>> face_descriptors = net(faces);
			std::vector<sample_pair> edges;
			for (size_t i = 0; i < face_descriptors.size(); ++i)
			{
				for (size_t j = i+1; j < face_descriptors.size(); ++j)
				{
					if (length(face_descriptors[i]-face_descriptors[j]) < 0.6)
						edges.push_back(sample_pair(i,j));
				}
			}
			std::vector<unsigned long> labels;
			const auto num_clusters = chinese_whispers(edges, labels);

			std::vector<matrix<rgb_pixel>> temp;
			images_cluster.clear();

			for (size_t cluster_id = 0; cluster_id < num_clusters; ++cluster_id)
			{
				temp.clear();
				std::vector<ofImage> cluster_temp;
				for (size_t j = 0; j < labels.size(); ++j)
				{
					if (cluster_id == labels[j]) 
					{
						temp.push_back(faces[j]);
						ofImage im = toOf(faces[j]);
						if(im.isAllocated())
						{
							cluster_temp.push_back(im);
						}
					}
				}
				images_cluster.push_back(cluster_temp);
			}
		}
		return images_cluster;
	}

	void setup(string pred = "model/shape_predictor_68_face_landmarks.dat", string recogn = "model/dlib_face_recognition_resnet_model_v1.dat" )
	{
		detector = get_frontal_face_detector();
		deserialize(ofToDataPath(pred)) >> sp;
    		deserialize(ofToDataPath(recogn)) >> net;
	}

        bool catalog_survey(ofImage fn)
        {
                find(fn);
                std::vector< std::vector<ofImage> > clust = cluster();
                return ((clust.size()>0)?true:false);
        }
	
	void compare(string _name, ofImage _p1)
	{
		p1=_p1;
		name=_name;
		startThread(true);
	}

	std::string getName()
	{
		return result;
	}

	void threadedFunction() 
	{
	        while(isThreadRunning()) 
		{
			uint64_t start = ofGetElapsedTimeMillis();
			if( catalog_survey(p1) )
				result = name;
			else
				result = "Unknown";
			uint64_t end = ofGetElapsedTimeMillis();
			cout<<"face-recognition: [ millis: "<<(end - start)<<" second: "<<((end - start)/1000)<<" ]\n";
			stopThread();
		}
	}
};
