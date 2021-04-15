#include<stdio.h> 
#include<stdlib.h> 
#include "decision.h"
#include <math.h>

#define REF 0
#define MAXCOORD 430
#define MINCOORD -430

// #include "../Pilotage/Pilotage.h"


// la convention sur les états 

//  1 : droite
// -1 : gauche
//  2 : haut
// -2 : bas
//  3 : avant
// -3 : arriére
//  4 : tourné à gauche
// -4 : tourné à droite 
// ///////////////// pour l'instant 
//  5  :  ok 
// -5 : ko

/*
Pour les intensités: 
    1 : pour loin un peu 
    2 : pour tres loin 
    3 : pour les extrémités

*/


void current_state_y(int **cordonnee,int *tab){  
    /*
    Cette fonction renvoie l'etat du dronne selon l'image actuellement recue 
    y1 étant le deuxième cordonnée de la première abeille et y2 de la deuxième abeille
    args : 
        cordonnee: les cordonnee des hirondelle recu sous forme d'une matrice 4*2 
        info : 
    */

    if((cordonnee[0][1]<(TAILLE_Y/8)*2)||(cordonnee[2][1]<(TAILLE_Y/8)*2)){
        tab[0]=-1;
        tab[1]=2;

    }
    else{

        if((cordonnee[0][1]<(TAILLE_Y/8)*3)||(cordonnee[2][1]<(TAILLE_Y/8)*3)){
            tab[0]=-1;
            tab[1]=1;
        }
        else{
            if((cordonnee[1][1]>(TAILLE_Y/8)*6)||(cordonnee[3][1]>(TAILLE_Y/8)*6)){
                tab[0]=1;
                tab[1]=2;
            }
            else{
                if((cordonnee[1][1]>(TAILLE_Y/8)*5)||(cordonnee[3][1]>(TAILLE_Y/8)*5)){
                    tab[0]=1;
                    tab[1]=1;
                }
                else{
                    tab[0]=0;
                    tab[1]=0;
                }
            }

        }
    }
}

void current_state_x(int **cordonnee, int *tab){
    /*
    Cette fonction renvoie l'etat du dronne selon l'image actuellement recue 
    y1 étant le deuxième cordonnée de la première abeille et y2 de la deuxième abeille
    */
 
    // on calcule la différence entre les y pour extimer l'état du drone

    if ((cordonnee[0][0]<(TAILLE_X/8)*2)||(cordonnee[1][0]<(TAILLE_X/8)*2)){
        tab[0]=2;
        tab[1]=2;
    }
    else{
        if((cordonnee[0][0]<(TAILLE_X/8)*3)||(cordonnee[1][0]<(TAILLE_X/8)*3)){
            tab[0]=2;
            tab[1]=1;
        }
        else{
            if((cordonnee[2][0]>(TAILLE_X/8)*6)||(cordonnee[3][0]>(TAILLE_X/8)*6)){
                tab[0]=-2;
                tab[1]=2;
            }
            else{
                if((cordonnee[2][0]>(TAILLE_X/8)*5)||(cordonnee[3][0]>(TAILLE_X/8)*5)){
                    tab[0]=-2;
                    tab[1]=1;
                
                }
                else{
                    tab[0]=0;
                    tab[1]=0;
                }
            }
        }

    }
}


int isDefine(int x, int y){
    return (x>=0 && y>=0);
}


float distance_euclidienne(int x,int y){
    return sqrt(pow((x-CENTREIMAGEx),2)+pow((y-CENTREIMAGEy),2));
}


float distanceCentre(int **cordonnee){
    float som = 0.0;
    int i;
    int nb=0;
    for(i=0;i<TAILLE;i++){
        if(isDefine(cordonnee[i][0],cordonnee[i][1])){
            nb++;
            som += distance_euclidienne(cordonnee[i][0],cordonnee[i][1]);
        }
    }
    if(nb!=0){
        return som/nb;
    }
}


int evaluation(int **cordonnee,float *DISTANCE_PRECEDENTE){
    /*
        renvoie 1 si on se deplace dans le bon sens et 0 sinon 
    */
    float distanceActuelle = distanceCentre(cordonnee);
    int res = (distanceActuelle < *DISTANCE_PRECEDENTE);  // soit 0 ou 1 
    *DISTANCE_PRECEDENTE = distanceActuelle;
    return res;

}
    
    

int nombre_de_cordonnee_recu(int **vecteur){
    /*
        renvoie le nombre de cordonnée renvoyée par la partie imagerie
    args: 
        vecteur: un tableau de cordonnée recu de partie imagerie
        taille: la taille du vecteur 
    */

    int nb = 0; // au départ le nombre de cordonnée non valide est 0 
    int i;  // l'indice d'ittération sur le tableau 
    for(i=0;i<TAILLE;i++){
        if(vecteur[i][0] >=0 && vecteur[i][1]>=0){ // si les cordonnée sont à -1 i.e que l'hirondelle n'est pas détectée 
            nb++; 
        }
    }
    return nb; // on renvoie le nombre d'hirondelle détectées 
}


/*-----------------Métohde bouchon----------------*/
//Methode bouhcon donc assez brouillon, principalment pour tester les mecanisme de control du drone
void analyseInterpretation(int **cordonnees)
{   
    printf("analyse\n");
    int x =cordonnees[0][0];
    int **tabDec=(int **)malloc(sizeof(int*)*4);

    for (int i = 0; i < 4; i++)
    {
        tabDec[i]=(int*)malloc(sizeof(int)*2);
    }
    
    int ifStop=1;
    tabDec[STRAFER][EVALUATION]=GOOD;

    //Test fin de coordonées
    if(x>MAXCOORD){
        ifStop=STOP;
    }
    
    if(x-REF<0)
    {
        if(x<(MINCOORD+REF)/2){
            tabDec[STRAFER][POS_INTENSITE]=CLOSE*NEGATIF;
        }
        else{
            tabDec[STRAFER][POS_INTENSITE]=FAR*NEGATIF;
        }
    }
    else{

        if(x<(MAXCOORD-REF)/2){
            tabDec[STRAFER][POS_INTENSITE]=CLOSE*POSITIF;
        }
        else{
            tabDec[STRAFER][POS_INTENSITE]=FAR*POSITIF;
        }

    }

    for (int i = AVANT_ARRIERE; i < ROTATION; i++){
        tabDec[i][EVALUATION]=GOOD;
        tabDec[i][POS_INTENSITE]=0;
    }
    
    callback(tabDec,ifStop);
}


