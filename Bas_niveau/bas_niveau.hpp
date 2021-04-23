

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
#include "../Decision/decision.h"
#include <libswscale/swscale.h>
}

#include <opencv4/opencv2/core.hpp>
#include <opencv4/opencv2/highgui.hpp>
#include <opencv4/opencv2/imgcodecs.hpp>
#include <opencv4/opencv2/opencv.hpp>

#include <cmath>

using namespace cv;
#define VOISINAGE 3 /* Voisinage utilisé pour un pixel donnée : Si VOISINAGE=A , le voisinage utilisé est AxA */
#define DISTANCE_MIN_ENTRE_PIXELS 1 /* distance minimum entre deux points, pour qu'ils ne soient pas dans le même point */
#define DISTANCE_MAX_ENTRE_PIXELS 50
#define MARGE_PIXEL_COLLINEAIRE 10
#define RAYON_CERCLES 5


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
int video_reader_process2(const char* infile);

void video_reader_close(SwsContext* sws_scaler_ctx, AVFormatContext* av_format_ctx, AVFrame* av_frame, AVFrame* decrame);

void image_processing(cv::Mat image,int*** resultat);
void affichage_haut_niveau_straffer(cv::Mat image,int* straffer);
#endif /* bas_niveau_hpp */
