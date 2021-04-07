

#ifndef bas_niveau_hpp
#define bas_niveau_hpp

#include <iostream>
#include <vector>

extern "C" {
    #include "../Décision/decision.h"
    #include <libavformat/avformat.h>
    #include <libavcodec/avcodec.h>
    #include <libavutil/avutil.h>
    #include <libavutil/pixdesc.h>
    #include <libavutil/imgutils.h>
    #include <libswscale/swscale.h>
}

#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include<opencv2/opencv.hpp>
using namespace cv;

class SPoint {       // The class
    public:       // Access specifier
    int x;
    int y;// Attribute (int variable)
    int somme;

    SPoint() : x(0),y(0),somme(0) {}
};

int ** video_reader_process(const char* infile);
void video_reader_close(SwsContext* sws_scaler_ctx, AVFormatContext* av_format_ctx, AVFrame* av_frame, AVFrame* decrame);
void image_processing(cv::Mat image,int*** resultat);

#endif /* bas_niveau_hpp */
