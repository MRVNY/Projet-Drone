///////////////////////////////////////////////////////////////////////////////
//                        (:| Partie Bas-Niveau |:)                          //
//                                                                           //
//                       par Meziane Mohamed & Saichi Lina                   //
//                                                                           //
//    But principal: DétéCtion de la mire                                    //
//                                                                           //
//*****-----------------------------------------------------------------*****//
//                                                                           //
//      Comme première solution, notre partie du projet va prendre en entrée //
// un nom de fichier qui représente le flux video qui nous sera remis par la //
// partie pilotage. Ensuite, nous décomposons le flux en frames grace a      //
// FFMPEG puis avec la bibliothèque de OPENCV on transforme chaque matrice de// 
// couleur en niveau de gris. On parcours chaque matrice et on tente de      //
// détecter les quatres hirondelles de la mire en sommant les pixels voisins //
// de chaque pixels de la matrice de manière a obtenir les minimums qui vont //
// représenter le motif des hirondelles. Ainsi dans cette première solution, //
// c'est notre partie qui va contenir la boucle evennementielle sur le flux  //
// video capturé par les pilotes et pour chaque tour de boucle on va appeler // 
// la partie analyse en transmettant nos resultats.                          // 
//                                                                           //
//*****-----------------------------------------------------------------*****//
//                                                                           //
//       Par souçis d'optimisation, nous deçidons d'implementer une deuxieme //
// solution qui consiste a récuperer les donnes brutes de chaque frame par   // 
// les pilotes, ensuite nous decodons grace a FFMPEG chaque frame recue qui  //
// sera de format H264 vers le format RGB requis pour le traitement de openCV//
// , ensuite nous tranformons chaque frame en niveau de gris et pour le reste//
// nous procedons de la même manière que la première sollution.              //
// Dans cette solution nous prenons donc une seule frame en entrée qui nous  // 
// sera transmise par la partie pilotage, de ce fait la boucle               //
// evennementielle sera dans le main et pour chaque tour de boucle il y'aura //
// un appel de notre partie qui va traiter une seule frame.                  //  
//*****-----------------------------------------------------------------*****//
//                                                                           //
//        Dans un premier temps, nous avons d'abors implémenté une fonction  //
// video_reader_process2 qui sera utilisé uniquement pour les tests que nous //
// effectuons sur des vidéos deja enregistré, pour le flux video en direct   //
// nous implementons la fonction video_reader_process(explication ci-dessous)//
//                                                                           //
///////////////////////////////////////////////////////////////////////////////


//////*****-------------------------------------------------------------*****//
//                  Déscription détaillé du code implementé                  //
///////////////////////////////////////////////////////////////////////////////


//////*****-------------------------------------------------------------*****//
//                        Fonctions de traitement d'images                   //
///////////////////////////////////////////////////////////////////////////////
//*****-----------------------------------------------------------------*****//
//                                                                           //
// video_reader_process2 :                                                   //
//                         Afin de tester sur des vidéo enregistré nous avons//
// crée cette fonction qui prend en parametre le nom d'un fichier video, dans//
// un premier temps nous allouons un tableau "resultat" qui contiendra les   //
// points détectés, nous ouvrons le fichier passé en parametre et aprés avoir// 
// recuperé toutes les informations necessaires au traitement OPENCV (width, //
// height, data etc) nous parcourons le flux et pour chaque frame nous       //
// appelons notre fonction "image_processing" qui grace a OPENCV va traiter  //
// chaque image.                                                             //
//*****-----------------------------------------------------------------*****//
// video_reader_process  :                                                   //
//                         Cette fonction est utilisé avec un flux en direct //
// passé en parametre, pour traiter ce flux on utilise le type "VideoCapture"// 
// qui grace a FFMPEG on obtient le decodage direct du flux en parametres du //
// format H264 en BGR, puis on lis chaque frame du flux et partir de la on   //
// procėde au même traitement que dans video_reader_process2                 // 
//                                                                           //               
//*****-----------------------------------------------------------------*****//
//                                                                           //
// image_processing :                                                        //
//                         Cette fonction prend une matrice en paramêtre     //
// ainsi que le tableau resultat passé par réference, nous avons aussi crée  //
// une structure nommée SPoint pour qui contient deux integer x et y         //
// representant la position d'un pixel ainsi que deux autres integer "somme" //
// qui est la somme obtenue pour chaque pixel et "score" qui sera expliqué   //
// dans la partie Robustesse. Nous declarons un tableau de six SPoint "res"  //
// pour récuperer les six meilleurs points de chaque matrice puis nous       //
// parcourons les pixels en calculant deux somme; "somme" qui representent   //  ToDo : renommer somme et somme2
// le motif des hirondelles du haut et "somme2" pour celles du bas, nous     //
// mettons les trois meilleures sommes des hirondelles hautes dans les trois //
// premieres cases de "res" et celles des hirondelles basses dans les trois  //
// cases restantes. Dans la partie restante de cette fonction, nous utilisons//
// plusieurs autres fonctions qu'on a crée afin de déterminer les points     //
// abbérants et les enlever(plus de details dans la section robustesse       //
// ci-dessous). Enfin nous passons les points restants dans le tableau       //
// "resultat" passé en parametre.                                            //
//*****-----------------------------------------------------------------*****//
//                                                                           //
// H264ToRGB :                                                               //
//                         Cette sera utilisé dans la deuxiėme sollution pour// 
// decoder chaque frame H264, elle prend en paramêtres les donnes "data" en  //
// format H264 ainsi que la taille des données et renvoie dans "outbuffer"   //
// passé en paramêtre le decodage en format BGR.                             //    
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

//////*****-------------------------------------------------------------*****//
//                  Stratégies de robustesse de la détéction                 //
///////////////////////////////////////////////////////////////////////////////
//       Dans le but d'augmenter l'efficacité de l'algorithme de détéction,  //
// nous avons décidé d'implementer successivement deux strategies.          //
//       En premier lieu, aprés avoir repéré les six meilleurs points de     //
// l'image dans dans la fonction image_processing, nous attribuons un score a//
// chaque point  puis on détérmine pour chaque points le nombre de points    //
// auquels il est espacé d'une distance de DISTANCE_MAX_ENTRE_PIXELS         //
// (fonction "intervalle") cette distance sera variable en fonction des      // 
// estimations de distances de la partie haute, ainsi si un point a un score //
// supérieur ou égal a 3, qui veut dire qu'il est escpacé d'au moins 3 points//
// (point isolé), nous decidons d'enlever ce points car il est probablement  //
// eloigné du groupement de points qui constituent les hirondelles de la mire//
// , ensuite on passe a l'analyse des points restant, pour moins de trois    //
// points on appelle la fonctions erreur qui mets les valeurs de "resultat"  //
// a -1, sinon si on obtient trois points, on appelle la fonction triangle   //
// qui permet de vérifier que les trois points forment un triangle, si on    //
// obtient quatres points alors on verifie avec la fonction "is_quadrilateral"/
// que les quatres points forment un quadrilatère, enfin si on obtient 5 ou 6//
// points alors on décide d'enlever les points dont la somme est supérieure  //
// aux autres points. Une autre stratégie qui pourrait étre intêrressante    //
// est d'utiliser la valeurs des sommes de nos tests pour fixer un seuil     //
// et enlever d'avantages de points abérrants (avoir aprés les tests dans le //
// centre jean talbot).                                                      //
///////////////////////////////////////////////////////////////////////////////

//////*****-------------------------------------------------------------*****//
//                  Fonctions intèrmediaires de robustesse                   //
///////////////////////////////////////////////////////////////////////////////
//////*****-------------------------------------------------------------*****//
// intervalle :                                                              //
//                Fonction qui retourne true si les deux coordonnées sont    //
// éloignées d'une distance passée en parametre.                             //
//////*****-------------------------------------------------------------*****//
// orientation :                                                             //
//             Fonction qui détérmine l'orientation des points : colineaires,//
// au sens des aiguilles d'une horloge, ou sens contraire. (a une marge prés)//
//////*****-------------------------------------------------------------*****//
// voisinage :                                                               //
//             Permet de calculer la somme des pixels voisin au pixel de la  //
// positions i,j.                                                            // 
//////*****-------------------------------------------------------------*****//
// doIntersect :                                                             //
//             Fonction qui vérifie si les segments que forment les points   //
// s'intersectent.                                                           //
//////*****-------------------------------------------------------------*****//
// not_similar :                                                             //
//             Fonction qui vérifie si les points ne sont pas les memes      //
//////*****-------------------------------------------------------------*****//
// not_collinear :                                                           //
//             verifie si les trois les points ne sont pas collineaires a une//
// d'erreur marge prés.                                                      //
//////*****-------------------------------------------------------------*****//
// is_quadrilateral :                                                        //
//             retourne vrai si les quatres points forment un quadrilateral. //
//////*****-------------------------------------------------------------*****//
// is_triangle :                                                             //
//             retourne vrai si les trois points forment un triangle         //
///////////////////////////////////////////////////////////////////////////////

//////*****-------------------------------------------------------------*****//
//                                ToDo Liste                                 //
///////////////////////////////////////////////////////////////////////////////
//      * Changer les #define en variables globales qui varient selon la     //
//        la distance.                                                       //
//      * Faire le lien avec la partie HAUT_NIVEAU pour reçevoir l'estimation//
//        des distances afin de faire varier le voisinage la distance minimum// 
//        et maximum des pixels ainsi que la marge de colinéarité            //
//      * Mesurer le temps de traitement des frames                          //
//      * Modifier la fonction video_reader_process pour qu'elle prennent une//
//        seule frame et appeler la nouvelle fonction H264ToRGB pour tester  //
//        le décodage des frames avec la partie PILOTAGE                     //
//      * Faire de nouvelle mesure dans le centre jean talbot avec la        //
//        la nouvelle mire(1,2,3 et 4m) pour changer les variations des      //
//        distances et voisinage et aussi, obtenir les valeurs des sommes    //
//        pour tester le ratio.                                              //
//      * Rajouter la contrainte du quadrilateral convexe/concave            // 
///////////////////////////////////////////////////////////////////////////////

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
#include "../commun.h"
#include <libswscale/swscale.h>
}

#include <opencv4/opencv2/core.hpp>
#include <opencv4/opencv2/highgui.hpp>
#include <opencv4/opencv2/imgcodecs.hpp>
#include <opencv4/opencv2/opencv.hpp>

#include <cmath>

using namespace cv;
#define VOISINAGE 3/* Voisinage utilisé pour un pixel donnée : Si VOISINAGE=A , le voisinage utilisé est AxA */
#define DISTANCE_MIN_ENTRE_PIXELS 2 /* distance minimum entre deux points, pour qu'ils ne soient pas dans le même point */
#define DISTANCE_MAX_ENTRE_PIXELS 150
#define MARGE_PIXEL_COLLINEAIRE 0
#define RAYON_CERCLES 5


//////*****-----------------------------------------------------------------*****//
//                         Structure des points                                  //
///////////////////////////////////////////////////////////////////////////////////
/**/struct SPoint                                                                //
/**/{                                                                            //
/**/   int x, y,somme,score;                                                     //
/**/};                                                                           //
/**////////////////////////////////////////////////////////////////////////////////



//////*****-----------------------------------------------------------------*****//
//                      Fonctions intèrmediaires de robustesse                   //
///////////////////////////////////////////////////////////////////////////////////
int intervalle (int x1,int y1,int x2,int y2);                                    // 
int orientation(SPoint p, SPoint q, SPoint r);                                   //
void voisinage(int voisin,int i, int j,int* S0,int* S1,int* S2,int* S3,cv::Mat image);
bool doIntersect(SPoint p1, SPoint q1, SPoint p2, SPoint q2);                    //
bool not_similar(SPoint p1, SPoint p2);                                          //
bool not_collinear(SPoint p1, SPoint p2, SPoint p3);                             //
int is_quadrilateral(SPoint p1, SPoint p2, SPoint p3, SPoint p4);                //
bool is_triangle(SPoint p1, SPoint q1, SPoint p2);                               //
///////////////////////////////////////////////////////////////////////////////////



//////*****-----------------------------------------------------------------*****//
//                        Fonctions de traitement d'images                       //
///////////////////////////////////////////////////////////////////////////////////
void erreur(int*** resultat);// METS LES POSITIONS DES POINTS A -1               //
void * video_reader_process(const char* infile);                                    //
int video_reader_process2(const char* infile);                                   //
void video_reader_close(SwsContext* sws_scaler_ctx, AVFormatContext* av_format_ctx, AVFrame* av_frame, AVFrame* decrame); 
//                                                                               // FONCTION DE DESALLOCATION
void image_processing(cv::Mat image,int*** resultat);                            //
void affichage_haut_niveau_straffer(cv::Mat image,int* straffer);                // fonction partie HAUT NIVEAU
void H264ToRGB(unsigned char* data, unsigned int dataSize, unsigned char* outBuffer);
///////////////////////////////////////////////////////////////////////////////////


#endif /* bas_niveau_hpp */
