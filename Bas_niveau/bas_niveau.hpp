

#ifndef bas_niveau_hpp
#define bas_niveau_hpp

#include <iostream>
#include <vector>

extern "C" {
    #include "../Decision/decision.h"
    #include <libavformat/avformat.h>
    #include <libavcodec/avcodec.h>
    #include <libavutil/avutil.h>
    #include <libavutil/pixdesc.h>
    #include <libavutil/imgutils.h>
    #include <libswscale/swscale.h>
    #include <libavfilter/avfiltergraph.h>
    //#include <libavfilter/avcodec.h>
    #include <libavfilter/buffersink.h>
    #include <libavfilter/buffersrc.h>
    #include <libavutil/opt.h>
}

#include <opencv4/opencv2/core.hpp>
#include <opencv4/opencv2/highgui.hpp>
#include <opencv4/opencv2/opencv.hpp>
#include <opencv4/opencv2/videoio.hpp>
#include <cmath>

using namespace cv;
#define VOISINAGE 3
#define EUCLID 3


struct SPoint
{
   int x, y,somme;
};


int intervalle (int x1,int y1,int x2,int y2);

void voisinage(int voisin,int i, int j,int* S0,int* S1,int* S2,int* S3,cv::Mat image);

int video_reader_process(const char* infile);
int video_reader_process_v2(const char* infile);
int open_input_file(const char *filename);

int H264_Init(void);


void video_reader_close(SwsContext* sws_scaler_ctx, AVFormatContext* av_format_ctx, AVFrame* av_frame, AVFrame* decrame);

void image_processing(cv::Mat image,int*** resultat);

#endif /* bas_niveau_hpp */
