#include "bas_niveau.hpp"
#include "../Décision/decision.h"
#include <typeinfo>



int** video_reader_process(const char* infile) {
    
    int ret;
    av_register_all();
    int** res; //resultat a retrourner
    AVFormatContext* inctx = nullptr;
    ret = avformat_open_input(&inctx, infile, nullptr, nullptr);
    if (ret < 0) {
        std::cerr << "fail to avforamt_open_input(\"" << infile << "\"): ret=" << ret;
        exit(1);
    }

    ret = avformat_find_stream_info(inctx, nullptr);
    if (ret < 0) {
        std::cerr << "fail to avformat_find_stream_info: ret=" << ret;
        exit(1);
    }


    AVCodec* vcodec = nullptr;
    ret = av_find_best_stream(inctx, AVMEDIA_TYPE_VIDEO, -1, -1, &vcodec, 0);
    if (ret < 0) {
        std::cerr << "fail to av_find_best_stream: ret=" << ret;
        exit(1);
    }
    const int vstrm_idx = ret;
    AVStream* vstrm = inctx->streams[vstrm_idx];


    ret = avcodec_open2(vstrm->codec, vcodec, nullptr);
    if (ret < 0) {
        std::cerr << "fail to avcodec_open2: ret=" << ret;
        exit(1);
    }


    std::cout
        << "infile: " << infile << "\n"
        << "format: " << inctx->iformat->name << "\n"
        << "vcodec: " << vcodec->name << "\n"
        << "size:   " << vstrm->codec->width << 'x' << vstrm->codec->height << "\n"
        << "fps:    " << av_q2d(vstrm->codec->framerate) << " [fps]\n"
        << "length: " << av_rescale_q(vstrm->duration, vstrm->time_base, {1,1000}) / 1000. << " [sec]\n"
        << "pixfmt: " << av_get_pix_fmt_name(vstrm->codec->pix_fmt) << "\n"
        << "frame:  " << vstrm->nb_frames << "\n"
        << std::flush;


    const int dst_width = vstrm->codec->width;
    const int dst_height = vstrm->codec->height;
    const AVPixelFormat dst_pix_fmt = AV_PIX_FMT_GRAY8;


    SwsContext* swsctx = sws_getCachedContext(
        nullptr, vstrm->codec->width, vstrm->codec->height, vstrm->codec->pix_fmt,
        dst_width, dst_height, dst_pix_fmt, SWS_BICUBIC, nullptr, nullptr, nullptr);
    if (!swsctx) {
        std::cerr << "fail to sws_getCachedContext";
        exit(1);
    }
    std::cout << "output: " << dst_width << 'x' << dst_height << ',' << av_get_pix_fmt_name(dst_pix_fmt) << std::endl;

    AVFrame* frame = av_frame_alloc();
    std::vector<uint8_t> framebuf(av_image_get_buffer_size(dst_pix_fmt, dst_width, dst_height,16
                                                           ));
    avpicture_fill(reinterpret_cast<AVPicture*>(frame), framebuf.data(), dst_pix_fmt, dst_width, dst_height);
    
    AVFrame* decframe = av_frame_alloc();
    unsigned nb_frames = 0;
    bool end_of_stream = false;
    int got_pic = 0;
    AVPacket pkt;
    //ticks tick,tick1,tickh;
       //tick = getticks();
    do {
        if (!end_of_stream) {
            // lire le packet
            ret = av_read_frame(inctx, &pkt);
            if (ret < 0 && ret != AVERROR_EOF) {
                std::cerr << "fail to av_read_frame: ret=" << ret;
                exit(1);
            }
            if (ret == 0 && pkt.stream_index != vstrm_idx)
                goto next_packet;
            end_of_stream = (ret == AVERROR_EOF);
        }
        if (end_of_stream) {

            av_init_packet(&pkt);
            pkt.data = nullptr;
            pkt.size = 0;
        }
       //decoder le packet
        avcodec_decode_video2(vstrm->codec, decframe, &got_pic, &pkt);
        if (!got_pic)
            goto next_packet;
        // convertir en image OpenCV
        sws_scale(swsctx, decframe->data, decframe->linesize, 0, decframe->height, frame->data, frame->linesize);
        {
        cv::Mat image(dst_height, dst_width, CV_8UC1, framebuf.data(), frame->linesize[0]);
        if(nb_frames==20){
            
             image_processing(image,&res);
             
        }
        }
        
            ++nb_frames;
            next_packet:
            av_packet_unref(&pkt);
      
    } while (!end_of_stream || got_pic);
    
    video_reader_close(swsctx, inctx, frame, decframe);
    return res;
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
    
   

