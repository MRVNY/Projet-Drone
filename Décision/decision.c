#include<stdio.h> 
#include<stdlib.h> 
#include "decision.h"
#include <math.h>

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


int main(int argc, int *argv){ 
  
    int  ETAT_PRECEDENT0 = 1000;
    int  ETAT_PRECEDENT1 = 1000;
    float DISTANCE_PRECEDENTE = 0.0;

    int **tab1;
    tab1[0][0]= -1;
    tab1[0][1]= -1;
    tab1[1][0]= 50;
    tab1[1][1]= 50;
    tab1[2][0]= -1;
    tab1[2][1]= -1;
    tab1[3][0]= 200;
    tab1[3][1]= 50;


    // int **tab2 = [[50,200],[50,400],[200,200],[200,400]]; 
    // int **tab3 = [[50,150],[50,350],[200,150],[1200,150]];
    // int **tab4 = [[50,200],[50,400],[200,200],[200,400]]; 
    // int **tab5 = [[50,500],[50,700],[200,500],[200,700]]; 
    // int **tab6= [[50,800],[50,1000],[200,800],[200,1000]]; 

    printf(" le resultat pour tab1 est :\n");
    analyseInterpretation(tab1,&ETAT_PRECEDENT0,&ETAT_PRECEDENT1,&DISTANCE_PRECEDENTE);
    // printf(" le resultat pour tab2 est :\n");
    // analyseInterpretation(tab2);
    // printf(" le resultat pour tab3 est :\n");
    // analyseInterpretation(tab3);
    // printf(" le resultat pour tab4 est :\n");
    // analyseInterpretation(tab4);
    // printf(" le resultat pour tab5 est :\n");
    // analyseInterpretation(tab5);
    // printf(" le resultat pour tab6 est :\n");
    // analyseInterpretation(tab6);
    return 0;
}