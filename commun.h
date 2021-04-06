#ifndef _COMMUN_H_
#define _COMMUN_H_
// La convention sur les états 

#define STOP 0
#define DROITE 1
#define GAUCHE -1
#define HAUT 2
#define BAS -2
#define AVANT 3
#define ARRIERE -3
#define TOURNEGAUCHE 4
#define TOURNEDROITE -4
#define TAKEOFF 6
#define LAND -6
//#define OK 5
//#define KO -5

// Pour les intensités: 
#define CLOSE 1
#define FAR 2
#define EXTREME 3

/*---------------Methodes partagées---------------*/
void callback(int *state);

#endif