#include "ofMain.h"
#ifdef SHIFT
#undef SHIFT
#endif
#include <dlib/dnn.h>
#include <dlib/data_io.h>
#include <dlib/image_transforms.h>

using namespace dlib;
using namespace std;

template <template <int,template<typename>class,int,typename> class iblock, int N, template<typename>class BN, typename SUBNET>
using iresidual = add_prev1<block<N,BN,1,tag1<SUBNET>>>;

template <template <int,template<typename>class,int,typename> class iblock, int N, template<typename>class BN, typename SUBNET>
using iresidual_down = add_prev2<avg_pool<2,2,2,2,skip1<tag2<block<N,BN,2,tag1<SUBNET>>>>>>;

template <int N, template <typename> class BN, int stride, typename SUBNET>
using iblock  = BN<con<N,3,3,1,1,relu<BN<con<N,3,3,stride,stride,SUBNET>>>>>;

template <int N, typename SUBNET> using iares      = relu<iresidual<iblock,N,affine,SUBNET>>;
template <int N, typename SUBNET> using iares_down = relu<iresidual_down<iblock,N,affine,SUBNET>>;

template <typename SUBNET> using ilevel1 = iares<512,iares<512,iares_down<512,SUBNET>>>;
template <typename SUBNET> using ilevel2 = iares<256,iares<256,iares<256,iares<256,iares<256,iares_down<256,SUBNET>>>>>>;
template <typename SUBNET> using ilevel3 = iares<128,iares<128,iares<128,iares_down<128,SUBNET>>>>;
template <typename SUBNET> using ilevel4 = iares<64,iares<64,iares<64,SUBNET>>>;

using ianet_type = loss_multiclass_log<fc<1000,avg_pool_everything<
                            ilevel1<
                            ilevel2<
                            ilevel3<
                            ilevel4<
                            max_pool<3,3,2,2,relu<affine<con<64,7,7,2,2,
                            input_rgb_image_sized<227>
                            >>>>>>>>>>>;

class ImageNet : public ofThread 
{
	public:

	std::vector<string> labels;
	ianet_type net;
	matrix<rgb_pixel> img, crop;
	std::vector<std::string> get_res;
	ofImage ximage;

	rectangle make_random_cropping_rect_resnet(const matrix<rgb_pixel>& img, dlib::rand& rnd)
	{
	    double mins = 0.466666666, maxs = 0.875;
	    auto scale = mins + rnd.get_random_double()*(maxs-mins);
	    auto size = scale*std::min(img.nr(), img.nc());
	    rectangle rect(size, size);
	    point offset(rnd.get_random_32bit_number()%(img.nc()-rect.width()),
			 rnd.get_random_32bit_number()%(img.nr()-rect.height()));
	    return move_rect(rect, offset);
	}

	void randomly_crop_images (const matrix<rgb_pixel>& img, dlib::array<matrix<rgb_pixel>>& crops, dlib::rand& rnd, long num_crops)
	{
	    std::vector<chip_details> dets;
	    for (long i = 0; i < num_crops; ++i)
	    {
		auto rect = make_random_cropping_rect_resnet(img, rnd);
		dets.push_back(chip_details(rect, chip_dims(227,227)));
	    }

	    extract_image_chips(img, dets, crops);

	    for (auto&& img : crops)
	    {
		if (rnd.get_random_double() > 0.5) {
		    img = fliplr(img);
		}
		apply_random_color_offset(img, rnd);
	    }
	}

	ofPixels toOf(const matrix<rgb_pixel> rgb)
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

        matrix<rgb_pixel> toDLib(const ofPixels px)
        {
            matrix<rgb_pixel> out;
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
                        assign_pixel( out(n,m), p );
                    }
                    else{
                        rgb_pixel p;
                        p.red   = v[m*3];
                        p.green = v[m*3+1];
                        p.blue  = v[m*3+2];
                        assign_pixel( out(n,m), p );
                    }
                }
            }
            return out;
        }

	void setup(string _resnet = "model/resnet34_1000_imagenet_classifier.dnn")
	{
		deserialize(ofToDataPath(_resnet)) >> net >> labels;
	}

	std::vector<std::string> search()
	{
		std::vector<std::string> out;
		softmax<ianet_type::subnet_type> snet;
		snet.subnet() = net.subnet();
		dlib::array<matrix<rgb_pixel>> images;
		dlib::rand rnd;

	        const int num_crops = 16;
        	randomly_crop_images(img, images, rnd, num_crops);

        	matrix<float,1,1000> p = sum_rows(mat(snet(images.begin(), images.end())))/num_crops;
	        for (int k = 0; k < 5; k++)
        	{
	            unsigned long predicted_label = index_of_max(p);
        	    out.push_back( ofToString(p(predicted_label)) + ": " + ofToString(labels[predicted_label]) );
	            p(predicted_label) = 0;
        	}
		return out;
    	}

	std::vector<string> getPredict(ofPixels _ximg)
	{
		ximage = _ximg;
		img = toDLib(ximage);
		return get_res;
	}

	void threadedFunction()
	{
        	while(isThreadRunning())
	        {
			if(ximage.isAllocated())
			{
				uint64_t start = ofGetElapsedTimeMillis();
				get_res = search();
				uint64_t end = ofGetElapsedTimeMillis();
				cout<<"image-net: [ millis: "<<(end - start)<<" second: "<<((end - start)/1000)<<" ]\n";
				stopThread();
			}
	        }
    	}
};

