#include<stdio.h> 
#include<stdlib.h> 
#include "decision.h"
#include <math.h>
#include "../commun.h" 
// #include "../Pilotage/Pilotage.h"



// les variable globales
int sortie[4][3]; // le tableau de la sortie 
float dx_precedent, dy_precedent ; // la distance entre le centre de la mire et celui de l'image 
int hirondelle_defined[4][2]; // un tableau de bool sur la disponiblité ou pas des horondelles



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



void current_state_y(int **cordonnee,int **tab){  
    /*
    Cette fonction renvoie l'etat du dronne selon l'image actuellement recue 
    y1 étant le deuxième cordonnée de la première abeille et y2 de la deuxième abeille
    args : 
        cordonnee: les cordonnee des hirondelle recu sous forme d'une matrice 4*2 
        tab:  le vecteur de sortie
    */

    if((cordonnee[0][1]<(TAILLE_Y/8)*2)||(cordonnee[2][1]<(TAILLE_Y/8)*2)){
        tab[STRAFER][POSITION]=GAUCHE;
        tab[STRAFER][INTENSITE]=FAR;

    }
    else{
        if((cordonnee[0][1]<(TAILLE_Y/8)*3)||(cordonnee[2][1]<(TAILLE_Y/8)*3)){
            tab[STRAFER][POSITION]=GAUCHE;
            tab[STRAFER][INTENSITE]=CLOSE;
        }
        else{
            if((cordonnee[1][1]>(TAILLE_Y/8)*6)||(cordonnee[3][1]>(TAILLE_Y/8)*6)){
                tab[STRAFER][POSITION]=DROITE;
                tab[STRAFER][INTENSITE]=FAR;
            }
            else{
                if((cordonnee[1][1]>(TAILLE_Y/8)*5)||(cordonnee[3][1]>(TAILLE_Y/8)*5)){
                    tab[STRAFER][POSITION]=DROITE;
                    tab[STRAFER][INTENSITE]=CLOSE;
                }
                else{
                    tab[STRAFER][POSITION]=AXE;
                    tab[STRAFER][INTENSITE]=AXE;
                }
            }

        }
    }
}



void current_state_x(int **cordonnee, int **tab){
    /*
    Cette fonction renvoie l'etat du dronne selon l'image actuellement recue 
    y1 étant le deuxième cordonnée de la première abeille et y2 de la deuxième abeille
    */
 
    // on calcule la différence entre les y pour extimer l'état du drone

    if ((cordonnee[0][0]<(TAILLE_X/8)*2)||(cordonnee[1][0]<(TAILLE_X/8)*2)){
        tab[MONTER_DESCENDRE][POSITION]=HAUT;
        tab[MONTER_DESCENDRE][INTENSITE]=FAR;
    }
    else{
        if((cordonnee[0][0]<(TAILLE_X/8)*3)||(cordonnee[1][0]<(TAILLE_X/8)*3)){
            tab[MONTER_DESCENDRE][POSITION]=HAUT;
            tab[MONTER_DESCENDRE][INTENSITE]=CLOSE;
        }
        else{
            if((cordonnee[2][0]>(TAILLE_X/8)*6)||(cordonnee[3][0]>(TAILLE_X/8)*6)){
                tab[MONTER_DESCENDRE][POSITION]=BAS;
                tab[MONTER_DESCENDRE][INTENSITE]=FAR;
            }
            else{
                if((cordonnee[2][0]>(TAILLE_X/8)*5)||(cordonnee[3][0]>(TAILLE_X/8)*5)){
                    tab[MONTER_DESCENDRE][POSITION]=BAS;
                    tab[MONTER_DESCENDRE][INTENSITE]=CLOSE;
                
                }
                else{
                    tab[MONTER_DESCENDRE][POSITION]=AXE;
                    tab[MONTER_DESCENDRE][INTENSITE]=AXE;
                }
            }
        }

    }
}




int isDefine(int x, int y){
    // test si l'hirondelle(x,y) est définie
    return (x>=0 && y>=0);
}



void calcule_dx_dy(int **cordonnee, float *dx, float *dy, int nombre_de_cordonnee_recu){
    // calcule la distance entre le centre de l'image et celui de la mire 
    int a1, a2;

    if(nombre_de_cordonnee_recu >=3){ // donc la on aura une bonne evaluation du déplacement car on peut calculer le bon centre 
        // traitement sur y 
        if(isDefine(cordonnee[0][0],cordonnee[0][1]) && isDefine(cordonnee[1][0],cordonnee[1][1])){
            a1 = cordonnee[0][1];
            a2 = cordonnee[1][1];
        }
        else{
            a1 = cordonnee[2][1];
            a2 = cordonnee[3][1];
        }
        *dy= abs(CENTREIMAGEY-(a1+a2)/2);

        // traitement sur x
        if(isDefine(cordonnee[0][0],cordonnee[0][1]) && isDefine(cordonnee[2][0],cordonnee[2][1])){
                a1 = cordonnee[0][0];
                a2 = cordonnee[2][0];
            }
            else{
                a1 = cordonnee[1][0];
                a2 = cordonnee[3][0];
            }
            *dx= abs(CENTREIMAGEX-(a1+a2)/2);
    }
    else{
        if(nombre_de_cordonnee_recu ==2){





        }
        else{
            if(nombre_de_cordonnee_recu ==1){

            }
        }


    }







}



float distanceCentre(int **cordonnee){
    //  
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



void analyseInterpretation(int **cordonnees,int *ETAT_PRECEDENT0,int *ETAT_PRECEDENT1,float *DISTANCE_PRECEDENTE){
    /*
        Cette fonction renvoie la commande à transmettre selon l'état du drone 
        vecteur : le résultat de la partie 
        cordonnees: les cordonnées des hirondelles 
    */
    int *vecteur=(int *)malloc(sizeof(int)*2);
    int nb_cordonnee = nombre_de_cordonnee_recu(cordonnees);    // on appelle la fonction pour calculer le nombre d'hirondelle recues 
    if(nb_cordonnee <=2 ){  // i.e on a recu que 2 ou 1 ou 0 hirondelle donc si on a un état précedent on pourai l'utilisé pour savoir si vraiment on est à l'extrémité ou c'est juste une erreur de la partie imageri 
        vecteur[0]=*ETAT_PRECEDENT0;  
        vecteur[1]=3;
    }
    else{
        current_state_y(cordonnees,vecteur);
        if(vecteur[0]==0){
            current_state_x(cordonnees,vecteur); // la normamlement on appelle directement une fonction de la partie pilotage en donnant en paramètre nos resulat 
        }
    }
    if((vecteur[0]==*ETAT_PRECEDENT0) && (vecteur[1]==*ETAT_PRECEDENT1)){ // je suis tjr dans la mm zone 
        if(nb_cordonnee!=0){
            if(evaluation(cordonnees,DISTANCE_PRECEDENTE)==0){ // on envoie un ko 
                vecteur[0]=-5;  
                vecteur[1]=0;   
            }
            else{ // on envoie un ok 
                vecteur[0]=5;  
                vecteur[1]=0; 
            }
        }
    }
    int k;
    for(k=0;k<2;k++){
       printf(" etat: %d \n intensité: %d \n",vecteur[0],vecteur[1]); 
    }
    // pilotage(vecteur) 
}

