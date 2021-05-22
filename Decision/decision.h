// **********************************************************************************************************************************************

// La taille de l'image 
#define TAILLE_X 540
#define TAILLE_Y 960

// sur la caméra du pc 
// #define TAILLE_X 240
// #define TAILLE_Y 320

// les centre de l'image 
#define CENTREIMAGEX 270
#define CENTREIMAGEY 480

// #define CENTREIMAGEX 120
// #define CENTREIMAGEY 160

// la taille de vecteur d'entrée (les hirondelle)
#define TAILLE 4


// à définir avec le calibrage et la capacité de la partie imagerie (pour avancer_reculer)
#define BORNE_FAR_BACK 63 // tres loin en arriere 
#define BORNE_CLOSE_BACK 85 // un peu loin en arriere
#define BORNE_CLOSE_FRONT 130 // proche


// à définir avec le calibrage (pour ROTATION)
#define BORNE_FAR_ROTATION 0.3
#define BORNE_CLOSE_ROTATION 0.1
#define BORNE_AXE_ROTATION 0



// *********************************************** LES FONCTION PARTAGEES  ********************************************************************

void isDefine(int **cordonnee, int *hirondelle_defined, int *nb_hirondelle_valide);
/*  une fonction pour verifier les hirondelle qu'on a reçues de la partie imagerie
    
    Args : 
        coordonnees (int[4][2])    : les hirondelles reçues
        hirondelle_defined (int[4]): le tableau de sortie : 0 pour dire que l'hirondelle n'est pas renseignée et 1 sinon
        nb_hirondelle_valide (int) : le nombre des hirondelles renseignées


*/




// *********************************************** LES FONCTION DE TRAITEMENT DE L'AXE DES Y ********************************************************************

int is_far_left(int a);
/*
    Une fonction pour savoir si la position de la mire dans l'image est trop loin à gauche
    args : 
        a (int) : la cordonnée selon y d'une hirondelles. 
    return :
        int : O si l'image n'est pas trop loin à gauche 
              1 sinon 
        

*/

int is_left(int a);
/*
    Une fonction pour savoir si la position de la mire dans l'image est proche à gauche
    args : 
        a (int) : la cordonnée selon y d'une hirondelles. 
    return :
        int : O si l'image n'est pas proche à gauche 
              1 sinon 
        

*/

int is_far_right(int a);
/*
    Une fonction pour savoir si la position de la mire dans l'image est trop loin à droite
    args : 
        a (int) : la cordonnée selon y d'une hirondelles. 
    return :
        int : O si l'image n'est pas trop loin à droite
              1 sinon 
        

*/

int is_right(int a);
/*
    Une fonction pour savoir si la position de la mire dans l'image est proche à droite
    args : 
        a (int) : la cordonnée selon y d'une hirondelles. 
    return :
        int : O si l'image n'est pas proche à droite
              1 sinon 
        

*/

void calcule_dy(int **cordonnee, float *dy);
/*
    Une fonction pour evaluer le déplacement sur y 
    args : 
        cordonnee(int[4][2]) : la cordonnée reçues. 
        dy (float *)         : la distance entre le centre de la mire dans limage et le centre de l'image selon y 
*/

void current_state_y(int **cordonnee);
/*
    Une fonction pour estimer la position actuelle du drone selon l'axe des y 
    Args: 
        cordonnee(int[4][2]): les hirondelles reçues
*/

int analyseInterpretation_y(int **cordonnees);
/*
    Une fonction pour estimer la position du drone et evaluer son deplacement sur l'axe des y. 
    Args : 
    cordonnees(int [4][2])  : les hirondelles reçues

    Return (int): 
        0: si il est dans l'axe selon y 
        1 sinon 
*/




// *********************************************** LES FONCTION DE TRAITEMENT DE L'AXE DES x ********************************************************************

int is_top(int a);
/*
    Une fonction pour savoir si la position de la mire dans l'image est trop en haut dans l'image
    args : 
        a (int) : la cordonnée selon x d'une hirondelles. 
    return :
        int : O si l'image n'est pas trop en haut 
              1 sinon 
        

*/

int is_mid_top(int a);
/*

    Une fonction pour savoir si la position de la mire dans l'image est un peu en haut dans l'image
    args : 
        a (int) : la cordonnée selon x d'une hirondelles. 
    return :
        int : O si l'image n'est pas un peu en haut 
              1 sinon 
        

*/

int is_bottom(int a);
/*
    Une fonction pour savoir si la position de la mire dans l'image est trop en bas dans l'image
    args : 
        a (int) : la cordonnée selon x d'une hirondelles. 
    return :
        int : O si l'image n'est pas trop en bas
              1 sinon 
        

*/

int is_mid_bottom(int a);
/*
    Une fonction pour savoir si la position de la mire dans l'image est un peu en bas dans l'image
    args : 
        a (int) : la cordonnée selon x d'une hirondelles. 
    return :
        int : O si l'image n'est pas un peu en bas
              1 sinon 
        

*/

void calcule_dx(int **cordonnee, float *dx);
/*
    Une fonction pour evaluer le déplacement sur x
    args : 
        cordonnee(int[4][2]) : la cordonnée reçues. 
        dx (float *)         : la distance entre le centre de la mire dans limage et le centre de l'image selon x
*/

void current_state_x(int **cordonnee);
/*
    Une fonction pour estimer la position actuelle du drone selon l'axe des x 
    Args: 
        cordonnee(int[4][2]): les hirondelles reçues
*/

int analyseInterpretation_x(int **cordonnees);
/*
    Une fonction pour estimer la position du drone et evaluer son deplacement sur l'axe des x. 
    Args : 
    cordonnees(int [4][2])  : les hirondelles reçues

    Return (int): 
        0: si il est dans l'axe selon x 
        1 sinon 
*/



// *********************************************** LES FONCTION DE TRAITEMENT DE L'AXE DES Z ********************************************************************

int get_nb_pixel(int **cordonnee);
/*
    Une fonction pour evaluer le déplacement sur z: solon la différence des pixels
    args : 
        cordonnee(int[4][2]) : la cordonnée reçues. 

    Return: 
        (int) : le nombre de pixels de différence entre deux hirondelles

*/

void current_state_z(int **cordonnee);
/*
    Une fonction pour estimer la position actuelle du drone selon l'axe des z
    Args: 
        cordonnee(int[4][2]): les hirondelles reçues
*/

int analyseInterpretation_z(int **cordonnees);
/*
    Une fonction pour estimer la position du drone et evaluer son deplacement sur l'axe des z. 
    Args : 
    cordonnees(int [4][2])  : les hirondelles reçues

    Return (int): 
        0: si il est dans l'axe selon z
        1 sinon 
*/




// *********************************************** LES FONCTION DE TRAITEMENT DE LA ROTATION  ********************************************************************

int is_much_turned(float ratio);
/*
    Une fonction pour savoir si le drone est trop tourné 
    args : 
        ratio (float) : le rapport de déformation 
    return :
        int : O si le drone n'est pas trop tourné 
              1 sinon 
*/

int is_less_turned(float ratio);
/*
    Une fonction pour savoir si le drone est un peu tourné 
    args : 
        ratio (float) : le rapport de déformation 
    return :
        int : O si le drone n'est pas un peu tourné 
              1 sinon 
*/

float calcule_dr(int **cordonnees);
/*
    Une fonction pour evaluer le déplacement selon la rotation
    args : 
        cordonnee(int[4][2]) : la cordonnée reçues. 
        
    Return  : 
        (float) : distance entre les coordonnées selon y de deux hirondelles
*/

float calcul_ratio1( int a , int b , int c);
/*
    Une fonction pour calucler le rapport de déformation selon la formule : |a - b| / |a - c|
    Args : 
        a(int): Une coordonnées selon x d'une hirondelles
        b(int): Une coordonnées selon x d'une hirondelles
        c(int): Une coordonnées selon x d'une hirondelles

    Return : 
        (float) : le ratio 

*/

float calcul_ratio2( int a , int b , int c);

/*
    Une fonction pour calucler le rapport de déformation selon la formule : |a - b| / (2 * |a - b| + |b - c|)
    Args : 
        a(int): Une coordonnées selon x d'une hirondelles
        b(int): Une coordonnées selon x d'une hirondelles
        c(int): Une coordonnées selon x d'une hirondelles

    Return : 
        (float) : le ratio 

*/

int get_direction( int a , int b );
/*
    Une fonction pour retrouver la direction de la rotation ( rotation gauche ou rotation droite)
    Args : 
        a(int) : la coordonnée selon x d'une hirondelle
        b(int) : la coordonnée selon x d'une hirondelle

    Return : 
        (int) : -1 si c'est une rotation à gauche 
                 1 si c'est une rotaiton à droite 
*/

void direction_ratio( int **coordonnee, int *direction, float *ratio );
/*
    Une fonction pour retrouver la direction ainsi que le rapport de déformation

    Args:   
        coordonnee(int[4][2]): les hirondelles reçues 
        direction(int *)     : la direction de rotation (1 ou -1)
        ratio(float *)       : le rapport de déformation     
*/

void current_state_rotation(int **cordonnee);
/*
    Une fonction pour estimer la position actuelle du drone selon l'axe de rotation
    Args: 
        cordonnee(int[4][2]): les hirondelles reçues
*/

int analyseInterpretation_rotation(int **cordonnees);
/*
    Une fonction pour estimer la position du drone et evaluer son deplacement selon la rotation 
    Args : 
    cordonnees(int [4][2])  : les hirondelles reçues

    Return (int): 
        0: si il est dans l'axe selon la rotation
        1 sinon 
*/



// *********************************************** LA FONCTION PRINCIPALE  ********************************************************************

void analyseInterpretation(int **cordonnees);
/*
    La fonction principale de la partie decision; elle renvoie la position selon tous les axes( x, y et z) et selon la rotation. 
    Elle renvoie aussi l'evaluation du déplacement . 
    Le résultat sera dirrectement reporté dans la variable partagée de type Sestimation. 

    Args : 
        cordonnees (int[4][2]): les hirondelelles reçues.




*/