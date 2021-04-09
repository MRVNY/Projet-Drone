#include "bas_niveau.hpp"
#include <typeinfo>



int intervalle (int x1,int y1,int x2,int y2){
    //Fonction qui retourne true si les deux coordonnées sont eloignées : pas la meme hirondelle
    
    double distance_euclidienne=sqrt((x1-x2)*(x1-x2) + (y1-y2)*(y1-y2));
    return  (distance_euclidienne > EUCLID);
     
}

void voisinage(int voisin,int i, int j,int* S0,int* S1,int* S2,int* S3,cv::Mat image){

    int x,y;
    (*S0)=0;
    (*S1)=0;
    (*S2)=0;
    (*S3)=0;
    
    for( x=i ; x>i-voisin ; x-- ){
        for( y=j ; y>j-voisin ; y-- ){
            (*S0)+=image.ptr<uchar>(x)[y];
        }
    }

    for( x=i ; x>i-voisin ; x-- ){
        for( y=j+1 ; y<=j+voisin ; y++ ){
            (*S1)+=image.ptr<uchar>(x)[y];
        }
    }

    for( x=i+1 ; x<=i+voisin ; x++ ){
        for( y=j ; y>j-voisin ; y-- ){
            (*S2)+=image.ptr<uchar>(x)[y];
        }
    }

    for( x=i+1 ; x<=i+voisin ; x++ ){
        for( y=j+1 ; y<=j+voisin ; y++ ){
            (*S3)+=image.ptr<uchar>(x)[y];
        }
    }

}


int video_reader_process(const char* infile) {

    int ** resultat= (int**) malloc( 4 * sizeof (int*) );

    if(resultat == NULL){
        fprintf(stderr,"probleme d'allocation\n");
        return 1;
    }

    for ( int i = 0 ; i < 4 ; ++i ) {       
        resultat[i] = ( int * ) malloc( 2 * sizeof(int) ) ;       
        if (  resultat[i]== NULL ){
            fprintf(stderr,"probleme d'allocation\n");
            return 1;
        }
       
    } //resultat a donner a la partie haut niveau

    int ret;
    
    av_register_all();
    
    AVFormatContext* inctx = nullptr;
    ret = avformat_open_input(&inctx, infile, nullptr, nullptr);
    if (ret < 0) {
        std::cerr << "fail to avforamt_open_input(\"" << infile << "\"): ret=" << ret;
        return 1;
    }

    ret = avformat_find_stream_info(inctx, nullptr);
    if (ret < 0) {
        std::cerr << "fail to avformat_find_stream_info: ret=" << ret;
        return 1;
    }


    AVCodec* vcodec = nullptr;
    ret = av_find_best_stream(inctx, AVMEDIA_TYPE_VIDEO, -1, -1, &vcodec, 0);
    if (ret < 0) {
        std::cerr << "fail to av_find_best_stream: ret=" << ret;
        return 1;
    }
    const int vstrm_idx = ret;
    AVStream* vstrm = inctx->streams[vstrm_idx];


    ret = avcodec_open2(vstrm->codec, vcodec, nullptr);
    if (ret < 0) {
        std::cerr << "fail to avcodec_open2: ret=" << ret;
        return 1;
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
        return 1;
    }
    std::cout << "output: " << dst_width << 'x' << dst_height << ',' << av_get_pix_fmt_name(dst_pix_fmt) << std::endl;

    AVFrame* frame = av_frame_alloc();
    std::vector<uint8_t> framebuf(av_image_get_buffer_size(dst_pix_fmt, dst_width, dst_height,16
                                                           ));
    avpicture_fill(reinterpret_cast<AVPicture*>(frame), framebuf.data(), dst_pix_fmt, dst_width, dst_height);
    
    /*AVFrame* frame = av_frame_alloc();
    int numBytes=av_image_get_buffer_size(dst_pix_fmt, dst_width, dst_height,16);
    uint8_t * framebuf = NULL;
    framebuf =(uint8_t *)av_malloc(numBytes*sizeof(uint8_t)); 
    avpicture_fill((AVPicture*)frame, framebuf, dst_pix_fmt, dst_width, dst_height);*/

    
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
                return 1;
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
       
        
            image_processing(image,&resultat);
        //traitement haut niveau
        //pilotage
            
             
        
        }
        
            ++nb_frames;
            next_packet:
            av_packet_unref(&pkt);
      
    } while (!end_of_stream || got_pic);
    
    video_reader_close(swsctx, inctx, frame, decframe);
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

    int somme=0; //Somme relative aux hirondelles du haut
    int somme2=0; //Somme relative aux hirondelles du bas
 
    int S0=0;
    int S1=0;
    int S2=0;
    int S3=0;

    struct SPoint res[6];
    for (int i=0;i<6;i++){
        res[i].somme=255*(VOISINAGE*VOISINAGE*2);
        res[i].x=0;
        res[i].y=0; 
    }
                                  
                                  
                 
                  

                   for( int x=VOISINAGE ; x<image.rows-VOISINAGE ; x++ ){
                       for( int y=VOISINAGE ; y<image.cols-VOISINAGE ; y++ ){
                           
                        voisinage(VOISINAGE,x,y,&S0,&S1,&S2,&S3,image);
                        somme=255*(VOISINAGE*VOISINAGE*2)+S0-S1-S2+S3;
                           
                           
                           if(somme <= res[0].somme && intervalle(x,y,res[1].x,res[1].y) && intervalle(x,y,res[0].x,res[0].y) ) {
                               
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
                            if(somme <= res[1].somme && somme > res[0].somme && intervalle(x,y,res[0].x,res[0].y) && intervalle(x,y,res[1].x,res[1].y) ){
                                
                               res[2].somme=res[1].somme;
                               res[2].x=res[1].x;
                               res[2].y=res[1].y;
                               res[1].somme=somme;
                               res[1].x=x;
                               res[1].y=y;
                           }
                           
                            if(somme <= res[2].somme && somme > res[1].somme && somme > res[0].somme && intervalle(x,y,res[0].x,res[0].y) && intervalle(x,y,res[1].x,res[1].y) ){
                               res[2].somme=somme;
                               res[2].x=x;
                               res[2].y=y;
                           }
                           
                       
                       }
                   }
                   

                   for( int x=VOISINAGE ; x<image.rows-VOISINAGE ; x++ ){
                       for( int y=VOISINAGE ; y<image.cols-VOISINAGE ; y++ ){
                           
                         
                           
                           voisinage(VOISINAGE,x,y,&S0,&S1,&S2,&S3,image);
                           somme=255*(VOISINAGE*VOISINAGE*2)-S0+S1+S2-S3;

                           
                           if(somme <= res[3].somme && intervalle(x,y,res[4].x,res[4].y) && intervalle(x,y,res[3].x,res[3].y) ) {
                              
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
                            if(somme <= res[4].somme && somme > res[3].somme && intervalle(x,y,res[3].x,res[3].y) && intervalle(x,y,res[4].x,res[4].y)   ){
                                
                               res[5].somme=res[4].somme;
                               res[5].x=res[4].x;
                               res[5].y=res[4].y;

                               res[4].somme=somme;
                               res[4].x=x;
                               res[4].y=y;
                               
                           }
                           
                            if(somme <= res[5].somme && somme > res[4].somme && somme > res[3].somme && intervalle(x,y,res[4].x,res[4].y) && intervalle(x,y,res[3].x,res[3].y) ){
                            


                            
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


         /*cv::Mat3b grayBGR;
                               
                cv::cvtColor(image, grayBGR, COLOR_GRAY2BGR);

                for(int i=0;i<6;i++){

                    std::cout << res[i].somme << " RESS" << std::endl;
                    std::cout << res[i].x << " X " << std::endl;
                    std::cout << res[i].y << " Y" << std::endl;
                    if(i==0){
                        
                        ellipse(grayBGR, Point(res[i].y,res[i].x),Size(10, 10), 0, 0,360, Scalar(255, 144, 30),-1, LINE_AA);} //dodger blue
                        
                    if(i==1){
                                           
                        ellipse(grayBGR, Point(res[i].y,res[i].x),Size(10, 10), 0, 0,360, Scalar(255, 144, 30),-1, LINE_AA);} //dodger blue
                    if(i==2){
                                           
                        ellipse(grayBGR, Point(res[i].y,res[i].x),Size(10, 10), 0, 0,360, Scalar(0,0,255),-1, LINE_AA);} //crimson 
                     if(i==3){
                                           
                        ellipse(grayBGR, Point(res[i].y,res[i].x),Size(10, 10), 0, 0,360, Scalar(34,139,34),-1, LINE_AA);} //forest green
                    

                     if(i==4){
                                           
                        ellipse(grayBGR, Point(res[i].y,res[i].x),Size(10, 10), 0, 0,360, Scalar(34,139,34),-1, LINE_AA);} //forest green

                     if(i==5){
                                           
                        ellipse(grayBGR, Point(res[i].y,res[i].x),Size(10, 10), 0, 0,360, Scalar(0,165,255),-1, LINE_AA); //orange
                       
                        } //violet                         
                                              
                }

               
               // cv::imwrite("image.jpg", image);
               // cv::imwrite("imagebgr.jpg", grayBGR);
                cv::imshow("defilement",grayBGR);
                cv::waitKey(1);*/
               
                
               
       
    
       
        
    }
    
   

