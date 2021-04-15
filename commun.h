#ifndef _COMMUN_H_
#define _COMMUN_H_


// La taille du vecteur de sortie 
#define TAILLE_SORTIE 4 
#define INFO_SORTIE 3

// La convention sur les états 
#define STOP 0
#define NEGATIF -1 
#define POSITIF  1
#define GOOD 1
#define BAD -1


// Pour les intensités: 
enum position {
    AXE=0,
    CLOSE=1,
    FAR=2,
    EXTREME=3    
};

// les convention sur les index de la matrice de sortie 
enum mouvement {
    STRAFER=0, 
    AVANT_ARRIERE=1,
    MONTER_DESCENDRE=2,  
    ROTATION=3 
};

// convention sur les index dans le tableau de sortie 

enum stateIndex{
    POS_INTENSITE=0,
    EVALUATION=1 
};


// -1 : pour gauche , en bas  ,rotation à gauche et aarière, ko 
// 1 : pour droite , haut , rotation à droite et avancer, ok 

/*---------------Methodes partagées---------------*/
void callback(int **state, int ifStop);

#endif