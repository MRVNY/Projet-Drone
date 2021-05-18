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
#define BORNE_FAR_BACK 90 // tres loin en arriere 
#define BORNE_CLOSE_BACK 150 // un peu loin en arriere
#define BORNE_CLOSE_FRONT 220 // proche


// à définir avec le calibrage (pour ROTATION)
#define BORNE_FAR_ROTATION 3/10 
#define BORNE_CLOSE_ROTATION 1/10
#define BORNE_AXE_ROTATION 0


int is_far_left(int a);
int is_left(int a);
int is_far_right(int a);
int is_right(int a);
int is_top(int a);
int is_mid_top(int a);
int is_bottom(int a);
 int is_mid_bottom(int a);
void current_state_y(int **cordonnee);
void current_state_x(int **cordonnee);
int get_nb_pixel(int **cordonnee);
void current_state_z(int **cordonnee);
float calcul_ratio1( int a , int b , int c);
float calcul_ratio2( int a , int b , int c);
int is_much_turned(float ratio);
int is_less_turned(float ratio);
int get_direction( int a , int b );
void direction_ratio( int **coordonnee, int *direction, float *ratio );
void current_state_rotation(int **cordonnee);
void isDefine(int **cordonnee, int *hirondelle_defined, int *nb_hirondelle_valide);
void calcule_dx_dy(int **cordonnee, float *dx, float *dy);
int analyseInterpretation_x_y(int **cordonnees);
float calcule_dr(int **cordonnees);
int analyseInterpretation_rotation(int **cordonnees);
int analyseInterpretation_z(int **cordonnees);
void analyseInterpretation(int **cordonnees);