#include "bas_niveau.hpp"
#include <fstream>      
#include <typeinfo>
#include <iostream>
#include <unistd.h>
#include <string.h>


//Define uniquement pour le bouhon 


using std::cout; using std::cerr;
using std::endl; using std::string;
using std::ifstream;

/*-----------------Métohde bouchon----------------*/

int video_reader_process(const char* infile) {

    string filename(infile);

    VideoCapture capture(infile,CAP_FFMPEG);

    capture.set(CAP_PROP_FOURCC, VideoWriter::fourcc('H','2','6','4'));
    capture.set(CAP_PROP_FRAME_WIDTH, 1280);
    capture.set(CAP_PROP_FRAME_HEIGHT, 720);
    
    if (!capture.isOpened()) {
        printf("NOT OPENED\n");
    }
        
    cv::Mat frame;
    
    while(1) {
        if (!capture.read(frame)) {
            //Error
        }
        cv::imshow("BAS NIVEAU", frame);
    
        cv::waitKey(30);
    } 

}

int video_reader_process_2(const char* infile) {

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
    
   

