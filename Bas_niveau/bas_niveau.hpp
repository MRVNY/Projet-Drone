

#ifndef bas_niveau_hpp
#define bas_niveau_hpp

#include <iostream>
#include <vector>

extern "C" {

#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
#include <libavutil/pixdesc.h>
#include <libavutil/imgutils.h>

#include <libswscale/swscale.h>
}

#include <opencv4/opencv2/core.hpp>
#include <opencv4/opencv2/highgui.hpp>
#include <opencv4/opencv2/opencv.hpp>
#include <cmath>

using namespace cv;
#define VOISINAGE 3
#define EUCLID 1
#define DISTANCE_MAX_ENTRE_PIXELS 50


struct SPoint
{
   int x, y,somme,score;
};


int intervalle (int x1,int y1,int x2,int y2);
int orientation(SPoint p, SPoint q, SPoint r);
void voisinage(int voisin,int i, int j,int* S0,int* S1,int* S2,int* S3,cv::Mat image);
bool doIntersect(SPoint p1, SPoint q1, SPoint p2, SPoint q2);
bool not_similar(SPoint p1, SPoint p2);
bool not_collinear(SPoint p1, SPoint p2, SPoint p3);
int is_quadrilateral(SPoint p1, SPoint p2, SPoint p3, SPoint p4);
bool is_triangle(SPoint p1, SPoint q1, SPoint p2);
void erreur(int*** resultat);
int video_reader_process(const char* infile);

void video_reader_close(SwsContext* sws_scaler_ctx, AVFormatContext* av_format_ctx, AVFrame* av_frame, AVFrame* decrame);

void image_processing(cv::Mat image,int*** resultat);

#endif /* bas_niveau_hpp */
