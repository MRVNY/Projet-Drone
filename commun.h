#ifndef _COMMUN_H_
#define _COMMUN_H_


// La taille du vecteur de sortie 
#define TAILLE_SORTIE 4  // la taille de la matrice de sortie que renvoie la partie decision (le nombre de ligne)
#define INFO_SORTIE 2 // le nombre de colonnes de la matrice de sortie 

// La convention sur les états 
#define STOP 0  
#define NEGATIF -1  // pour gauche , en bas  , rotation à gauche  et  arrière
#define POSITIF  1  // pour droite , haut    , rotation à droite  et  avancer

#define GOOD 1  // l'evaluation  du déplacement est bonne 
#define BAD -1  // l'evaluation  du déplacement n'est pas bonne 


// Pour les intensités: on quatre intensité  
enum position {    
    AXE=0,   // le drone est bien positionné dans l'axe
    CLOSE=1,  // le drone est un peu proche 
    FAR=2, // le drone est loin 
    EXTREME=3    // il est à l'extrémité 
};

// les convention sur les index de la matrice de sortie 
enum mouvement {
    STRAFER=0,  // la première ligne de la matrice de sortie donne les infor sur la position du drone sur l'axe x et y (il est à gauce ou à doite)
    AVANT_ARRIERE=1, // pour la position selon z : il est à l'avant ou à l'arrière 
    MONTER_DESCENDRE=2,   // le deplacement sur y, il est en haut ou en bas
    ROTATION=3  // le deplacement selon la rotation 
};

// convention sur les index dans le tableau de sortie 
enum stateIndex{
    POS_INTENSITE=0, // la première colone renseigne la position et l'intensité au mm temps 
    EVALUATION=1  // pour l'évaluation de mouvemnt 
};

// -1 : pour gauche , en bas  ,rotation à gauche et aarière, ko 
// 1 : pour droite , haut , rotation à droite et avancer, ok 

/*-----------Methode commune------------*/
void callbackPilote(int **state,int ifStop);

#endif 



