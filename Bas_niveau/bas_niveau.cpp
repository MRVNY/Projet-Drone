#include "bas_niveau.hpp"
#include <fstream>      
#include <typeinfo>
#include <iostream>
#include <unistd.h>


//Define uniquement pour le bouhon 


using std::cout; using std::cerr;
using std::endl; using std::string;
using std::ifstream;


/*-----------------Métohde bouchon----------------*/
AVCodec         *pCodec = NULL;
AVCodecContext  *pCodecCtx = NULL;
SwsContext      *img_convert_ctx = NULL;
AVFrame         *pFrame = NULL;
AVFrame         *pFrameRGB = NULL;


/*-----------------Métohde bouchon----------------*/
static AVFormatContext *fmt_ctx;
static AVCodecContext *dec_ctx;
AVFilterContext *buffersink_ctx;
AVFilterContext *buffersrc_ctx;
AVFilterGraph *filter_graph;
static int video_stream_index = -1;
static int64_t last_pts = AV_NOPTS_VALUE;

int open_input_file(const char *filename)
{      

    std::cout<<"opend_input:"<<"\n";
    int ret;
    AVCodec *dec;
    if ((ret = avformat_open_input(&fmt_ctx, filename, NULL, NULL)) < 0) {
        av_log(NULL, AV_LOG_ERROR, "Cannot open input file\n");
        return ret;
    }
    std::cout<<"ici\n";
    if ((ret = avformat_find_stream_info(fmt_ctx, NULL)) < 0) {
        av_log(NULL, AV_LOG_ERROR, "Cannot find stream information\n");
        return ret;
    }
    /* select the video stream */
    ret = av_find_best_stream(fmt_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, &dec, 0);
    if (ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "Cannot find a video stream in the input file\n");
        return ret;
    }
    video_stream_index = ret;
    dec_ctx = fmt_ctx->streams[video_stream_index]->codec;
    av_opt_set_int(dec_ctx, "refcounted_frames", 1, 0);
    /* init the video decoder */
    if ((ret = avcodec_open2(dec_ctx, dec, NULL)) < 0) {
        av_log(NULL, AV_LOG_ERROR, "Cannot open video decoder\n");
        return ret;
    }
    return 0;
}

int video_reader_process_v2(const char* infile){

    std::cout<<"ici"<<"\n";
    cv::VideoCapture capture(infile,CAP_ANY);
    std::cout<<"la"<<"\n";

    if (!capture.isOpened()) {
            std::cout<<"Open PB"<<"\n";

    }
    
    cv::namedWindow("Stream", CV_MINOR_VERSION);
    
    cv::Mat frame;
    
    while(1) {
        if (!capture.read(frame)) {
            std::cout<<"Read PB"<<"\n";
        }
        cv::imshow("Stream", frame);
    
        cv::waitKey(30);
    } 
}
int video_reader_process(const char* infile) {

    string filename(infile);

    ifstream input_file(filename);
    if (!input_file.is_open()) {
        cerr << "Could not open the file - '"
             << filename << "'" << endl;
    
    }
    
    int i;
    int j;

    int **tab=(int**)malloc(sizeof(int*)); 
    for (int i = 0; i < 2; i++)
    {
        tab[i]=(int*)malloc(sizeof(int)*2);
    }

    //Compteur pour test watchdog
    int cpt=0;
           
    cout<<"Début boucle évènementielle\n"<<endl;
    while ((input_file>>i)&&(input_file>>j))
    {   
        cpt++;
        if(cpt==10){
            //Simulation de problème de traitement
            printf("bug traitement image\n");
            sleep(2);

        }
        tab[0][0]=i;
        tab[0][1]=j;
        usleep(41000);
        analyseInterpretation(tab);

    }

    //Indication de fin de fichier de coords
    //MAGICNUMBER pour le bouchon ...
    tab[0][0]=1000;
    tab[0][1]=0;
    analyseInterpretation(tab);
    
    input_file.close();

    return 0;

}
    
void video_reader_close(SwsContext* sws_scaler_ctx, AVFormatContext* av_format_ctx, AVFrame* av_frame, AVFrame* decframe) {
    
    sws_freeContext(sws_scaler_ctx);
    
    avformat_close_input(&av_format_ctx);
    avformat_free_context(av_format_ctx);
    
    av_frame_free(&av_frame);
    av_frame_free(&decframe);
    
}

void image_processing(cv::Mat image,int*** resultat){
        
        int width = image.rows;
        int height = image.cols;
        int somme=0;
        int somme2=0;
        
                       int S0,S1,S2,S3;
                       //res contient deux min et deux max triés
                       //int* res=(int*)malloc(4*sizeof(int));
                       *resultat = (int**)malloc(4*sizeof(int*));
                       for(int i=0;i<4;i++)
                            (*resultat)[i]= (int*)malloc(2*sizeof(int));
                      
                       SPoint res[6];
                       res[2].somme=255*36;
                       res[1].somme=255*36;
                       res[0].somme=255*36;
                       res[3].somme=255*36;
                       res[4].somme=255*36;
                       res[5].somme=255*36;
                        
      
        

        for(int x=2;x<image.rows-3;x++){
            for(int y=2;y<image.cols-3;y++){
                
               /* S0=image.at<uchar>(y,x) + image.at<uchar>(y,x-1) + image.at<uchar>(y-1,x-1) + image.at<uchar>(y,x-1);
                S1=image.at<uchar>(y+1,x) + image.at<uchar>(y+2,x) + image.at<uchar>(y+1,x-1) + image.at<uchar>(y+2,x-1);
                S2=image.at<uchar>(y,x+1) + image.at<uchar>(y-1,x+1) + image.at<uchar>(y,x+2) + image.at<uchar>(y-1,x+2);
                S3=image.at<uchar>(y+1,x+1) + image.at<uchar>(y+2,x+1) + image.at<uchar>(y+1,x+2) + image.at<uchar>(y+2,x+2);*/
                
                
                // voisine 2x2
                S0=image.ptr<uchar>(x)[y]+ image.ptr<uchar>(x-1)[y] + image.ptr<uchar>(x-1)[y-1]+image.ptr<uchar>(x)[y-1];
                S1=image.ptr<uchar>(x)[y+1]+image.ptr<uchar>(x)[y+2]+image.ptr<uchar>(x-1)[y+1]+image.ptr<uchar>(x-1)[y+2];
                S2=image.ptr<uchar>(x+1)[y]+image.ptr<uchar>(x+1)[y-1]+image.ptr<uchar>(x+2)[y]+image.ptr<uchar>(x+2)[y-1];
                S3=image.ptr<uchar>(x+1)[y+1]+image.ptr<uchar>(x+1)[y+2]+image.ptr<uchar>(x+2)[y+1]+image.ptr<uchar>(x+2)[y+2];

      

                somme=255*8+S0-S1-S2+S3;
                
                //minimum 0 1 2 
                if(somme <= res[0].somme ) {
                    res[2].somme=res[1].somme;
                    res[2].x=res[1].x;
                    res[2].y=res[1].y;
                    res[1].somme=res[0].somme;
                    res[1].x=res[0].x;
                    res[1].y=res[0].y;
                    res[0].somme=somme;
                    res[0].x=x;
                    res[0].y=y;
                }
                 if(somme <= res[1].somme && somme > res[0].somme ){
                     
                    res[2].somme=res[1].somme;
                    res[2].x=res[1].x;
                    res[2].y=res[1].y;
                    res[1].somme=somme;
                    res[1].x=x;
                    res[1].y=y;
                }
                
                 if(somme <= res[2].somme && somme > res[1].somme && somme > res[0].somme){
                    res[2].somme=somme;
                    res[2].x=x;
                    res[2].y=y;
                }
                
            
            }
        }
        

        for(int x=2;x<image.rows-3;x++){
            for(int y=2;y<image.cols-3;y++){
                
               /* S0=image.at<uchar>(y,x) + image.at<uchar>(y,x-1) + image.at<uchar>(y-1,x-1) + image.at<uchar>(y,x-1);
                S1=image.at<uchar>(y+1,x) + image.at<uchar>(y+2,x) + image.at<uchar>(y+1,x-1) + image.at<uchar>(y+2,x-1);
                S2=image.at<uchar>(y,x+1) + image.at<uchar>(y-1,x+1) + image.at<uchar>(y,x+2) + image.at<uchar>(y-1,x+2);
                S3=image.at<uchar>(y+1,x+1) + image.at<uchar>(y+2,x+1) + image.at<uchar>(y+1,x+2) + image.at<uchar>(y+2,x+2);*/
                
                
                // voisine 2x2
                S0=image.ptr<uchar>(x)[y]+ image.ptr<uchar>(x-1)[y] + image.ptr<uchar>(x-1)[y-1]+image.ptr<uchar>(x)[y-1];
                S1=image.ptr<uchar>(x)[y+1]+image.ptr<uchar>(x)[y+2]+image.ptr<uchar>(x-1)[y+1]+image.ptr<uchar>(x-1)[y+2];
                S2=image.ptr<uchar>(x+1)[y]+image.ptr<uchar>(x+1)[y-1]+image.ptr<uchar>(x+2)[y]+image.ptr<uchar>(x+2)[y-1];
                S3=image.ptr<uchar>(x+1)[y+1]+image.ptr<uchar>(x+1)[y+2]+image.ptr<uchar>(x+2)[y+1]+image.ptr<uchar>(x+2)[y+2];

      
                somme=255*8-S0+S1+S2-S3;
                
                //minimum 3 4 5
                if(somme <= res[3].somme ) {
                   
                    res[5].somme=res[4].somme;
                    res[5].x=res[4].x;
                    res[5].y=res[4].y;
                    res[4].somme=res[3].somme;
                    res[4].x=res[3].x;
                    res[4].y=res[3].y;

                    res[3].somme=somme;
                    res[3].x=x;
                    res[3].y=y;
                    
                }
                 if(somme <= res[4].somme && somme > res[3].somme ){
                     
                    res[5].somme=res[4].somme;
                    res[5].x=res[4].x;
                    res[5].y=res[4].y;

                    res[4].somme=somme;
                    res[4].x=x;
                    res[4].y=y;
                    
                }
                
                 if(somme <= res[5].somme && somme > res[4].somme && somme > res[3].somme){
                     
                    res[5].somme=somme;
                    res[5].x=x;
                    res[5].y=y;
                    
                }
                
            
            }
        }
        

        (*resultat)[0][0]=res[0].x;
        
        (*resultat)[0][1]=res[0].y;

        (*resultat)[1][0]=res[1].x;
        (*resultat)[1][1]=res[1].y;

       (*resultat)[2][0]=res[2].x;
        (*resultat)[2][1]=res[2].y;

        (*resultat)[3][0]=res[3].x;
        (*resultat)[3][1]=res[3].y;
       
    
       
        
    }
    
   

