#include "bas_niveau.hpp"
#include <typeinfo>
#include <iostream>
#include <chrono> 
#include <thread> 
#include <string.h>
#include <ctime>
   
int intervalle (int x1,int y1,int x2,int y2,int distance){
    /*Fonction qui retourne true si les deux coordonnées sont éloignées d'une distance passée en parametre */
    
    double distance_euclidienne=sqrt((x1-x2)*(x1-x2) + (y1-y2)*(y1-y2));
    return  (distance_euclidienne > distance); //si ils sont loin
     
}

int orientation(SPoint p, SPoint q, SPoint r)
/*Fonction qui determine l'orientation des points : colineaires, au sens des aiguilles d'une horloge, ou sens contraire */
{
    int val = (q.y - p.y) * (r.x - q.x) - (q.x - p.x) * (r.y - q.y);

    if (val >= -MARGE_PIXEL_COLLINEAIRE and val<=MARGE_PIXEL_COLLINEAIRE) // a MARGE_PIXEL_COLLINEAIRE pixels pres
        return 0;
    return (val > 0) ? 1 : 2;
}

bool doIntersect(SPoint p1, SPoint q1, SPoint p2, SPoint q2)
/*Fonction qui verifie si les segments s'intersectent*/
{
    int o1 = orientation(p1, q1, p2);
    int o2 = orientation(p1, q1, q2);
    int o3 = orientation(p2, q2, p1);
    int o4 = orientation(p2, q2, q1);

    if (o1 != o2 && o3 != o4)
        return true;

    return false;
}

bool not_similar(SPoint p1, SPoint p2){
/*Fonction qui verifie si les points ne sont pas les memes*/
   
    if (intervalle(p1.x,p1.y,p2.x,p2.y,DISTANCE_MIN_ENTRE_PIXELS)==1)// si les points assez loin (de DISTANCE_MIN_ENTRE_PIXELS pres) alors ils sont
        return true;

    return false; //si deux points sont les memes, alors on retourne false car un quadrilateral n'est pas possible dans ce cas
}

bool not_collinear(SPoint p1, SPoint p2, SPoint p3)
/*verifie si les points ne sont pas collineaires*/
{
    int x1 = p1.x, y1 = p1.y;
    int x2 = p2.x, y2 = p2.y;
    int x3 = p3.x, y3 = p3.y;
    
    if (x1 * (y2 - y3) + x2 * (y3 - y1) + x3 * (y1 - y2) <= MARGE_PIXEL_COLLINEAIRE and (y3 - y2) * (x2 - x1) - (y2 - y1) * (x3 - x2) >=- MARGE_PIXEL_COLLINEAIRE)
        return false; /*si les points sont collineaires alors on retourne false car un quadrilateral n'est pas possible dans ce cas*/

    return true;
}

int is_quadrilateral(SPoint p1, SPoint p2, SPoint p3, SPoint p4){
       // retourne vrai si les 4 points forment un quadrilateral 
      bool same = true;
      same = same & not_similar(p1, p2);
      same = same & not_similar(p1, p3);
      same = same & not_similar(p1, p4);
      same = same & not_similar(p2, p3);
      same = same & not_similar(p2, p4);
      same = same & not_similar(p3, p4);
    
      // si des points similaires existent
    if (same == false){    
        return 0;     
    }
      
      bool coll = true;
      coll = coll & not_collinear(p1, p2, p3); 
      coll = coll & not_collinear(p1, p2, p4);
      coll = coll & not_collinear(p1, p3, p4);
      coll = coll & not_collinear(p2, p3, p4);
    
      //si des points sont collineaires
    if (coll == false){     
         return 0;    
    }
         
      //verifient si c'est un quadrilateral convexe
     /* int check = 1;
    
      if (doIntersect(p1, p2, p3, p4))
          check = 0;
      if (doIntersect(p1, p3, p2, p4))
          check = 0;
      if (doIntersect(p1, p2, p4, p3))
          check = 0; */     //a tester
    
    return 1;
}

bool is_triangle(SPoint p1, SPoint q1, SPoint p2){
    // retourne vrai si les 3 points forment un triangle
    return ( not_collinear(p1,q1,p2));
    
}

void voisinage(int voisin,int i, int j,int* S0,int* S1,int* S2,int* S3,cv::Mat image){
    //Permet de calculer la somme des pixels voisin au pixel de la positions i,j 
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

void erreur(int*** resultat){
    //Mets la position de tout les points resultants a la valeurs -1 pour indiquer la non-detection des points
    for( int i=0;i<4;i++){
        for(int j=0;j<2;j++){
             (*resultat)[i][j]=-1;
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
    }

    String filename(infile); // necessaire?

    VideoCapture capture(infile,CAP_FFMPEG);

    capture.set(CAP_PROP_FOURCC, VideoWriter::fourcc('H','2','6','4'));
    capture.set(CAP_PROP_FRAME_WIDTH, 1280);
    capture.set(CAP_PROP_FRAME_HEIGHT, 720);

    if (!capture.isOpened()) {
        printf("NOT OPENED\n");
    }

    cv::Mat frame;
    int cpt=0;
    while(1) {

        

        if (!capture.read(frame)) {
            exit(1);
        }

        cv::Mat greyMat;
        cv::cvtColor(frame, greyMat, COLOR_BGR2GRAY);

        //Calcul temp d'de process d'image
        clock_t begin_process = clock();
        if(cpt%2==0){
            image_processing(greyMat,&resultat);
        }
        clock_t end_process = clock();
        double time_spent_proc = (double)(end_process - begin_process) / CLOCKS_PER_SEC;

        printf("Imageprocessing time:%f\n",time_spent_proc);
        cpt++;
        analyseInterpretation(resultat);
        
        //cv::imshow("BAS NIVEAU", frame);
        //cv::waitKey(30);
    }
} 

int video_reader_process2(const char* infile) {

    // Fonction qui nous permet de tester la detection  des hirondelles sur des videos de test.
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
    }

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
    do {
        printf("\nFrame numero %d\n",nb_frames);
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
            analyseInterpretation(resultat);
        //    int* straffer= analyseInterpretation(resultat);
        //    affichage_haut_niveau_straffer(image,straffer);
        //     cv::imshow("BAS NIVEAU", image);
        //     cv::waitKey(1);
        }
        
            ++nb_frames;
            next_packet:
            av_packet_unref(&pkt);
      
    } while (!end_of_stream || got_pic);
    
    for ( int i = 0 ; i < 4 ; ++i ) {    
        free(resultat[i]) ;
    }
    free(resultat);
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
        res[i].x=-1;
        res[i].y=-1;
        res[i].score=0; 

    }
    // initialisation
    for (int i=0;i<4;i++){
         (*resultat)[i][0]=-1;
         (*resultat)[i][1]=-1;
    }
      
   //PARCOURS DES PIXELS ET DETETION DES 6 POINTS AYANT LA SOMME MINIMUM
                   for( int x=VOISINAGE ; x<image.rows-VOISINAGE  ; x++ ){ //image.rows-VOISINAGE et image.cols-VOISINAGE pour eviter les bordures
                       for( int y=VOISINAGE ; y<image.cols-VOISINAGE ; y++ ){
                           
                        voisinage(VOISINAGE,x,y,&S0,&S1,&S2,&S3,image);
                        
                        somme=255*(VOISINAGE*VOISINAGE*2)+S0-S1-S2+S3;
                        somme2=255*(VOISINAGE*VOISINAGE*2)-S0+S1+S2-S3;
                           
                           
                           if(somme <= res[0].somme && intervalle(x,y,res[1].x,res[1].y,DISTANCE_MIN_ENTRE_PIXELS) && intervalle(x,y,res[0].x,res[0].y,DISTANCE_MIN_ENTRE_PIXELS) ) {
                               
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
                            if(somme <= res[1].somme && somme > res[0].somme && intervalle(x,y,res[0].x,res[0].y,DISTANCE_MIN_ENTRE_PIXELS) && intervalle(x,y,res[1].x,res[1].y,DISTANCE_MIN_ENTRE_PIXELS) ){
                                
                               res[2].somme=res[1].somme;
                               res[2].x=res[1].x;
                               res[2].y=res[1].y;
                                
                               res[1].somme=somme;
                               res[1].x=x;
                               res[1].y=y;
                           }
                           
                            if(somme <= res[2].somme && somme > res[1].somme && somme > res[0].somme && intervalle(x,y,res[0].x,res[0].y,DISTANCE_MIN_ENTRE_PIXELS) && intervalle(x,y,res[1].x,res[1].y,DISTANCE_MIN_ENTRE_PIXELS) ){
                               res[2].somme=somme;
                               res[2].x=x;
                               res[2].y=y;
                           }
                                 
                            if(somme2 <= res[3].somme && intervalle(x,y,res[4].x,res[4].y,DISTANCE_MIN_ENTRE_PIXELS) && intervalle(x,y,res[3].x,res[3].y,DISTANCE_MIN_ENTRE_PIXELS) ) {
                              
                               res[5].somme=res[4].somme;
                               res[5].x=res[4].x;
                               res[5].y=res[4].y;
                               
                               res[4].somme=res[3].somme;
                               res[4].x=res[3].x;
                               res[4].y=res[3].y;

                               res[3].somme=somme2;
                               res[3].x=x;
                               res[3].y=y;
                               
                           }
                            if(somme2 <= res[4].somme && somme2 > res[3].somme && intervalle(x,y,res[3].x,res[3].y,DISTANCE_MIN_ENTRE_PIXELS) && intervalle(x,y,res[4].x,res[4].y,DISTANCE_MIN_ENTRE_PIXELS)   ){
                                
                               res[5].somme=res[4].somme;
                               res[5].x=res[4].x;
                               res[5].y=res[4].y;

                               res[4].somme=somme2;
                               res[4].x=x;
                               res[4].y=y;
                               
                           }
                           
                            if(somme2 <= res[5].somme && somme2 > res[4].somme && somme2 > res[3].somme && intervalle(x,y,res[4].x,res[4].y,DISTANCE_MIN_ENTRE_PIXELS) && intervalle(x,y,res[3].x,res[3].y,DISTANCE_MIN_ENTRE_PIXELS) ){
                            
                               res[5].somme=somme2;
                               res[5].x=x;
                               res[5].y=y;
                               
                            }                       
                       }
                   } //FIN DE LA DETETION DES 6 POINTS AYANT LA SOMME MINIMUM
    
    
    for( int i=0;i<6;i++){
        for(int j=0;j<6;j++){
            if( i != j and intervalle(res[i].x,res[i].y,res[j].x,res[j].y,DISTANCE_MAX_ENTRE_PIXELS)==1){ // Si un point est a une distance superieurs a distance ;ax des pixels on augmente le score du point
                res[i].score += 1;
            }
        }
    }

    // SUPPRESSION DES POINTS ELOIGNÉS DES AUTRES
    int total_points=6;
    for(int i=0;i<6;i++){
        if ( res[i].score >= 3){ //Si un point est loin d'au moins 3 points, on le considere comme aberrant 
            res[i].x = -1;
            res[i].y = -1;
            total_points--;
        }
    }

    // ANALYSE DES POINTS RESTANTS
    if(total_points < 3){// SI IL Y'A MOINS DE 3 POINTS RESTANTS ON DECIDE DE NE RIEN ENVOYER CAR ON A AUCUN MOYEN DE SAVOIR SI 2 OU 1 POINT SONT DANS LA MIRE VU LES VALEURS DES SOMMES QUI NOUS AIDENT PAS
        erreur(resultat);        
    }

    else {
        // ANALYSE DE LA FORME DES POINTS RESTANTS
        if(total_points==3){
            
            struct SPoint triangle[3];
            int pos = 0;
            for(int i=0;i<6;i++){
                if ( res[i].x != -1){
                    triangle[pos]=res[i];
                    pos++;
                }
            }
            if (is_triangle(triangle[0],triangle[1],triangle[2])){
                
                line(image,Point(triangle[0].y,triangle[0].x),Point(triangle[1].y,triangle[1].x),(255,0,0),5);

                line(image,Point(triangle[1].y,triangle[1].x),Point(triangle[2].y,triangle[2].x),(255,0,0),5);

                line(image,Point(triangle[0].y,triangle[0].x),Point(triangle[2].y,triangle[2].x),(255,0,0),5);
            }
        }

        if(total_points==4){
            struct SPoint quad[4];
            int pos = 0;
            for(int i=0;i<6;i++){
                if ( res[i].x != -1){
                    quad[pos]=res[i];
                    pos++;
                }
            }
            if (is_quadrilateral(quad[0],quad[1],quad[2],quad[3])==1){
                
                (*resultat)[0][0]=quad[0].x;
                (*resultat)[0][1]=quad[0].y;

                (*resultat)[1][0]=quad[1].x;
                (*resultat)[1][1]=quad[1].y;
                (*resultat)[2][0]=quad[2].x;
                (*resultat)[2][1]=quad[2].y;

                (*resultat)[3][0]=quad[3].x;
                (*resultat)[3][1]=quad[3].y;
                
                line(image,Point(quad[0].y,quad[0].x),Point(quad[1].y,quad[1].x),(255,0,0),5);

                line(image,Point(quad[1].y,quad[1].x),Point(quad[3].y,quad[3].x),(255,0,0),5);

                line(image,Point(quad[3].y,quad[3].x),Point(quad[2].y,quad[2].x),(255,0,0),5);

                line(image,Point(quad[2].y,quad[2].x),Point(quad[0].y,quad[0].x),(255,0,0),5);
            }
            
        }
        
        if(total_points==5){
            struct SPoint quad[5];
            int pos = 0;
            int point_enleve = -1; // trouver le point manquant pour enlever un autre point 
              for(int i=0;i<6;i++){
                if ( res[i].x != -1){
                  quad[pos]=res[i];
                  pos++;
                }
                else{point_enleve = i;}
            }
            if(point_enleve == 3 or point_enleve == 4){
              quad[2]=quad[4];
              res[2].x=-1;
              res[2].y=-1;
            }
            if(point_enleve < 3 ){
                res[5].x=-1;
                res[5].y=-1;
            }
            
            if (is_quadrilateral(quad[0],quad[1],quad[2],quad[3])==1){
           
                (*resultat)[0][0]=quad[0].x;
                (*resultat)[0][1]=quad[0].y;

                (*resultat)[1][0]=quad[1].x;
                (*resultat)[1][1]=quad[1].y;
                (*resultat)[2][0]=quad[2].x;
                (*resultat)[2][1]=quad[2].y;

                (*resultat)[3][0]=quad[3].x;
                (*resultat)[3][1]=quad[3].y;
                line(image,Point(quad[0].y,quad[0].x),Point(quad[1].y,quad[1].x),(255,0,0),5);

                line(image,Point(quad[1].y,quad[1].x),Point(quad[3].y,quad[3].x),(255,0,0),5);

                line(image,Point(quad[3].y,quad[3].x),Point(quad[2].y,quad[2].x),(255,0,0),5);

                line(image,Point(quad[2].y,quad[2].x),Point(quad[0].y,quad[0].x),(255,0,0),5);
            }
            
        }
             
        
        if(total_points==6){
            //DANS LE CAS OU ON A TROUVE 6 POINTS, ON SE CONTENTE DE SUPPRIMER LES POINTS AVEC LA SOMME LA PLUS GRANDE
            res[2].x=-1;
            res[2].y=-1;
            res[5].x=-1;
            res[5].y=-1;
            if (is_quadrilateral(res[0],res[1],res[3],res[4])==1){// on elimine les derniers de chaque somme
               (*resultat)[0][0]=res[0].x;
               (*resultat)[0][1]=res[0].y;

               (*resultat)[1][0]=res[1].x;
               (*resultat)[1][1]=res[1].y;

                (*resultat)[2][0]=res[3].x;
                (*resultat)[2][1]=res[3].y;

                (*resultat)[3][0]=res[4].x;
                (*resultat)[3][1]=res[4].y;
                line(image,Point(res[0].y,res[0].x),Point(res[1].y,res[1].x),(255,0,0),5);

                line(image,Point(res[1].y,res[1].x),Point(res[4].y,res[4].x),(255,0,0),5);

                line(image,Point(res[4].y,res[4].x),Point(res[3].y,res[3].x),(255,0,0),5);

                line(image,Point(res[3].y,res[3].x),Point(res[0].y,res[0].x),(255,0,0),5);
            }
        }

    }

    
    //re ordonner points pour la partie haut niveau
    if((*resultat)[0][1]>(*resultat)[1][1]){
        int tmp = (*resultat)[0][0];
        int tmp2 = (*resultat)[0][1];
        (*resultat)[0][0]=(*resultat)[1][0];
        (*resultat)[1][0]=tmp;
        (*resultat)[0][1]=(*resultat)[1][1];;
        (*resultat)[1][1]=tmp2;
    }
     if((*resultat)[2][1]>(*resultat)[3][1]){
        int tmp = (*resultat)[2][0];
        int tmp2 = (*resultat)[2][1];
        (*resultat)[2][0]=(*resultat)[3][0];
        (*resultat)[3][0]=tmp;
        (*resultat)[2][1]=(*resultat)[3][1];;
        (*resultat)[3][1]=tmp2;
    }
               
    // affichage en couleurs pour tester nos resultats   
        cv::Mat3b grayBGR;
                            
            cv::cvtColor(image, grayBGR, COLOR_GRAY2BGR);
       
            for(int i=0;i<6;i++){
                 //std::cout << res[i].somme << " \nRESS" << std::endl;
                 //std::cout << res[i].x << " X " << std::endl;
                 //std::cout << res[i].y << " Y\n" << std::endl;//}
                if(i==0){
                    //printf("VOISINAGE : %d\n",VOISINAGE);
                    ellipse(grayBGR, Point(res[i].y,res[i].x),Size(RAYON_CERCLES,RAYON_CERCLES), 0, 0,360, Scalar(255, 144, 30),-1, LINE_AA);} //dodger blue
                    
                if(i==1){
                                                                        
                    ellipse(grayBGR, Point(res[i].y,res[i].x),Size(RAYON_CERCLES,RAYON_CERCLES), 0, 0,360, Scalar(255, 144, 30),-1, LINE_AA);} //dodger blue
                if(i==2){
                                        
                    ellipse(grayBGR, Point(res[i].y,res[i].x),Size(RAYON_CERCLES,RAYON_CERCLES), 0, 0,360, Scalar(0,0,255),-1, LINE_AA);} //crimson
                if(i==3){
                                        
                    ellipse(grayBGR, Point(res[i].y,res[i].x),Size(RAYON_CERCLES,RAYON_CERCLES), 0, 0,360, Scalar(34,139,34),-1, LINE_AA);} //forest green
                
                if(i==4){
                                        
                    ellipse(grayBGR, Point(res[i].y,res[i].x),Size(RAYON_CERCLES,RAYON_CERCLES), 0, 0,360, Scalar(34,139,34),-1, LINE_AA);} //forest green

                if(i==5){
                                        
                    ellipse(grayBGR, Point(res[i].y,res[i].x),Size(RAYON_CERCLES,RAYON_CERCLES), 0, 0,360, Scalar(0,165,255),-1, LINE_AA); //orange
                    
                        } //violetf
                                          
            }
               
            //cv::imwrite("image.jpg", image);
            //cv::imwrite("imagebgr.jpg", grayBGR);
             cv::imshow("BAS NIVEAU", grayBGR);
             cv::waitKey(1); 
            //std::this_thread::sleep_for(std::chrono::milliseconds(500) );
              
}


void H264ToRGB(unsigned char* data, unsigned int dataSize, unsigned char* outBuffer)
{
    //Load metadata into packet 1
    AVPacket* avPkt = av_packet_alloc();
    avPkt->size = dataSize;
    avPkt->data = data;

    static AVCodecContext* codecCtx = nullptr;
    if (codecCtx == nullptr) {
        //2. Create and configure codeccontext
        AVCodec* h264Codec = avcodec_find_decoder(AV_CODEC_ID_H264);
        codecCtx = avcodec_alloc_context3(h264Codec);
        avcodec_get_context_defaults3(codecCtx, h264Codec);
        avcodec_open2(codecCtx, h264Codec, nullptr);
    }

    //3. Decoding
    //avcodec_ decode_ Video2 (codecctx, & outframe, & linelength, & avpkt); // interface is discarded and replaced by the lower interface
    auto ret = avcodec_send_packet(codecCtx, avPkt);
    if (ret >= 0) {
        AVFrame* YUVFrame = av_frame_alloc();
        ret = avcodec_receive_frame(codecCtx, YUVFrame);
        if (ret >= 0) {

            //4. YUV to RGB24
            AVFrame* RGB24Frame = av_frame_alloc();
            struct SwsContext* convertCxt = sws_getContext(
                YUVFrame->width, YUVFrame->height, AV_PIX_FMT_YUV420P,
                YUVFrame->width, YUVFrame->height, AV_PIX_FMT_RGB24,
                SWS_POINT, NULL, NULL, NULL
            );

            //Outbuffer will be assigned to rgb24frame > data, AV_ PIX_ FMT_ RGB24 format is only assigned to rgb24frame > data [0]
            av_image_fill_arrays(
                RGB24Frame->data, RGB24Frame->linesize, outBuffer,
                AV_PIX_FMT_RGB24, YUVFrame->width, YUVFrame->height,
                1
            );
            sws_scale(convertCxt, YUVFrame->data, YUVFrame->linesize, 0, YUVFrame->height, RGB24Frame->data, RGB24Frame->linesize);

            //5. Clear objects / context > release memory
            // free context and avFrame
            sws_freeContext(convertCxt);
            av_frame_free(&RGB24Frame);
            // RGB24Frame.
        }
        // free context and avFrame
        av_frame_free(&YUVFrame);
    }
    // free context and avFrame
    av_packet_unref(avPkt);
    av_packet_free(&avPkt);
    // avcodec_free_context(&codecCtx);
}


void affichage_haut_niveau_straffer(cv::Mat image,int* straffer){
    if (straffer[0]==1)
        ellipse(image, Point(image.cols-80,30),Size(RAYON_CERCLES*2,RAYON_CERCLES*2), 0, 0,360, Scalar(0, 255, 0),-1, LINE_AA); //dodger blue

    if (straffer[0]==2)
        ellipse(image, Point(image.cols-80,30),Size(RAYON_CERCLES*2,RAYON_CERCLES*2), 0, 0,360, Scalar(255, 0, 0),-1, LINE_AA); //dodger blue

    if (straffer[0]==-1)
        ellipse(image, Point(80,30),Size(RAYON_CERCLES*2,RAYON_CERCLES*2), 0, 0,360, Scalar(0, 255, 0),-1, LINE_AA); //dodger blue

    if (straffer[0]==-2)
        ellipse(image, Point(80,30),Size(RAYON_CERCLES*2,RAYON_CERCLES*2), 0, 0,360, Scalar(255, 0, 0),-1, LINE_AA); //dodg
}
