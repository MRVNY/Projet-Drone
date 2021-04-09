// La taille de l'image 
#define TAILLE_X 1080
#define TAILLE_Y 1920
// les centre de l'image 
#define CENTREIMAGEX 539.5
#define CENTREIMAGEY 959.5
// la taille de vecteur d'entrée (les hirondelle)
#define TAILLE 4 

void current_state_y(int **cordonnee,int **tab); // renvoie la position du drone sur l'axe des y 
void current_state_x(int **cordonnee, int **tab);  // renvoie la position du drone sur l'a
void isDefine(int **cordonnee,int *hirondelle_defined, int *nb_hirondelle_valide); //  pour une hirondelle(x,y) verifie si elle est bien renseigné par la partie imagerie
void calcule_dx_dy(int **cordonnee, float *dx, float *dy);
void analyseInterpretation(int **cordonnees);
