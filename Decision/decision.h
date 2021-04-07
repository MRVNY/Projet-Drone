// La taille de l'image 
#define TAILLE_X 1080
#define TAILLE_Y 1920
#define CENTREIMAGEx 539.5
#define CENTREIMAGEy 959.5
#define TAILLE 4

#include "../commun.h"

void current_state_y(int **cordonnee,int *tab);
void current_state_x(int **cordonnee, int *tab);
int isDefine(int x, int y);
float distance_euclidienne(int x,int y);
float distanceCentre(int **cordonnee);
int evaluation(int **cordonnee,float *DISTANCE_PRECEDENTE);
int nombre_de_cordonnee_recu(int **vecteur);

//En-tête modifiée pour le bouchon, à regularisé avec la décision
void analyseInterpretation(int *tab,int min, int max, int ref);