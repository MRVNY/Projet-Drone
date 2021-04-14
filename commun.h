#ifndef _COMMUN_H_
#define _COMMUN_H_

// La convention sur les états 

#define STOP 0
#define DROITE 1
#define GAUCHE -1
#define HAUT 1
#define BAS -1
#define AVANT 1
#define ARRIERE -1
#define TOURNEGAUCHE -1
#define TOURNEDROITE 1
#define GOOD 1
#define BAD -1


// Pour les intensités: 
#define AXE 0 
#define CLOSE 1
#define FAR 2
#define EXTREME 3

// convention sur les index dans le tableau de sortie 
#define POSITION 0
#define INTENSITE 1
#define EVALUATION 2 

// les convention sur les index de la matrice de sortie 
#define STRAFER 0 
#define avant_arriere 1
#define MONTER_DESCENDRE 2  
#define ROTATION 3 


// -1 : pour gauche , en bas  ,rotation à gauche et aarière, ko 
// 1 : pour droite , haut , rotation à droite et avancer, ok


/*---------------Methodes partagées---------------*/
void callback(int *state);

#endif