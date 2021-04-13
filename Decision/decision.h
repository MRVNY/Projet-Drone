// La taille de l'image 
#define TAILLE_X 1080
#define TAILLE_Y 1920
// les centre de l'image 
#define CENTREIMAGEX 539.5
#define CENTREIMAGEY 959.5
// la taille de vecteur d'entrée (les hirondelle)
#define TAILLE 4

#define BORNE_FAR_BACK 3 
#define BORNE_CLOSE_BACK 1
#define BORNE_AXE 0.5
#define BORNE_CLOSE_FRONT 0.3
#define BORNE_FAR_FRONT 0.1 
#define BORNE_EXTREM_FRONT 0
#define BORNE_EXTREM_BACK  4  // à définir avec le calibrage et la capacité de la partie imagerie 




void current_state_y(int **cordonnee,int **tab); // renvoie la position du drone sur l'axe des y 
void current_state_x(int **cordonnee, int **tab);  // renvoie la position du drone sur l'a
void current_state_z(int **cordonnee, int **tab);
void isDefine(int **cordonnee,int *hirondelle_defined, int *nb_hirondelle_valide); //  pour une hirondelle(x,y) verifie si elle est bien renseigné par la partie imagerie
void calcule_dx_dy(int **cordonnee, float *dx, float *dy);
int get_nb_pixel(int **cordonnee);
void analyseInterpretation(int **cordonnees);
int is_far_left(int a);
int is_left(int a);
int is_far_right(int a);
int is_right(int a);
int is_top(int a);
int is_mid_top(int a);
int is_bottom(int a);
int is_mid_bottom(int a);

