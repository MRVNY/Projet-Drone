// La taille de l'image 
#define TAILLE_X 1080
#define TAILLE_Y 1920
// les centre de l'image 
#define CENTREIMAGEX 539.5
#define CENTREIMAGEY 959.5
// la taille de vecteur d'entrée (les hirondelle)
#define TAILLE 4 

void current_state_y(int **cordonnee,int *tab); // renvoie la position du drone sur l'axe des y 
void current_state_x(int **cordonnee, int *tab);  // renvoie la position du drone sur l'a
int isDefine(int x, int y); //  pour une hirondelle(x,y) verifie si elle est bien renseigné par la partie imagerie
float distance_euclidienne(int x,int y); 
float distanceCentre(int **cordonnee);
int evaluation(int **cordonnee,float *DISTANCE_PRECEDENTE);
int nombre_de_cordonnee_recu(int **vecteur);
void analyseInterpretation(int **cordonnees,int *ETAT_PRECEDENT0,int *ETAT_PRECEDENT1,float *DISTANCE_PRECEDENTE);
