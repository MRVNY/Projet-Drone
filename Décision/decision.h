
#ifndef _DECISION_H_
#define _DECISION_H_

// La taille de l'image 
#define TAILLE_X 1080
#define TAILLE_Y 1920

int current_state_y(int y1,int y2);
int current_state_x(int x1,int x2);
void cordinate(int **hirondelles);
void commande(int *vecteur, int **cordonnees);

#endif 
