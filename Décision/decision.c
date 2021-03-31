#include<stdio.h> 
#include<stdlib.h> 
#include "decision.h"
#include "../Pilotage/Pilotage.h"

//convention pour les etats
// -1 : proche à droite || un peu en haut 
// -2 : loin à droite   || tres en haut 
//  0 : dans l'axe      || dans l'axe 
//  1 : proche à gauche || un peu en bas 
//  2 : loin à gauche   || trop en bas 




int current_state_y(int y1,int y2){
    /*
    Cette fonction renvoie l'etat du dronne selon l'image actuellement recue 
    y1 étant le deuxième cordonnée de la première abeille et y2 de la deuxième abeille
    */
 
    // on calcule la différence entre les y pour extimer l'état du drone

    if ((y1<(TAILLE_Y/8)*2)||(y2<(TAILLE_Y/8)*2)){
        return 2;
    }
    else{
        if((y1<(TAILLE_Y/8)*3)||(y2<(TAILLE_Y/8)*3)){
            return 1;
        }
        else{
            if((y1>(TAILLE_Y/8)*6)||(y2>(TAILLE_Y/8)*6)){
                return -2;
            }
            else{
                if((y1>(TAILLE_Y/8)*5)||(y2>(TAILLE_Y/8)*5)){
                    return -1;
                
                }
                else{
                    return 0;
                }
            }
        }

    }
}



int current_state_x(int x1,int x2){
    /*
    Cette fonction renvoie l'etat du dronne selon l'image actuellement recue 
    y1 étant le deuxième cordonnée de la première abeille et y2 de la deuxième abeille
    */
 
    // on calcule la différence entre les y pour extimer l'état du drone

    if ((x1<(TAILLE_X/8)*2)||(x2<(TAILLE_X/8)*2)){
        return -2;
    }
    else{
        if((x1<(TAILLE_X/8)*3)||(x2<(TAILLE_X/8)*3)){
            return -1;
        }
        else{
            if((x1>(TAILLE_X/8)*6)||(x2>(TAILLE_X/8)*6)){
                return 2;
            }
            else{
                if((x1>(TAILLE_X/8)*5)||(x2>(TAILLE_X/8)*5)){
                    return 1;
                
                }
                else{
                    return 0;
                }
            }
        }

    }
}


/* convention de la sortie: 
Pour les instruction: 
    on renvoie un vecteur de type (instruction,intensité) 
    strafferG/starafferD : 1 pour gauche -1 pour droite 
    monter/descendre : 2 pour monter , -2 pour descendre 
    avant/arrière:  3 pour avant ; -3 pour arriére 
    rotation: 4 pour rotation à guache, -4 pour rotation à droite 

Pour les intensités: 
    1 : poiur une commande élémentaire 
    2: pour le double

*/


void cordinate(int **hirondelles){
    /* re_ordonne le tableau des horondelle de sorte à avoir les 2 premiers pour le mouvement 
    selon x et les deux dernière pour le mouvement selon y 
    */
    int i, j; 
    int x,y;
    for(i=0;i<3;i++){
        for(j=i+1;j<4;j++){
            if(hirondelles[i][0]<hirondelles[j][0]){
                x=hirondelles[i][0];
                y=  hirondelles[i][1];
                hirondelles[i][0]=hirondelles[j][0];
                hirondelles[i][1]=hirondelles[j][1];
                hirondelles[j][0]=x;
                hirondelles[j][1]=y;
            }
       }
    }
}


void commande(int *vecteur, int **cordonnees){
    /*
    Cette fonction renvoie la commande à transmettre selon l'état du drone 
    */
    cordinate(cordonnees); 
    int etatx,etaty;
    etaty = current_state_y(cordonnees[1][1],cordonnees[2][1]);
    printf("etaty est %d \n",etaty);
    printf("affichage de l'estimation de la position du drone\n\n ");
    switch (etaty){
        case -1 :
            printf("proche à droite\n");
            vecteur[0]=1; 
            vecteur[1]=1; 
            break;

        case -2 :
            printf("loin à droite\n");
            vecteur[0]=1; 
            vecteur[1]=2; 
            break;
        
        case 1 :
            printf("proche à gauche\n");
            vecteur[0]=-1; 
            vecteur[1]=1; 
            break;
        
        case 2 :
            printf("loin à gauche\n");
            vecteur[0]=-1; 
            vecteur[1]=2; 
            break;
        
        case 0 : 
            printf(" dans l'axe des X donc on regard selon Y\n ");
            etatx = current_state_x(cordonnees[0][0],cordonnees[1][0]);
            switch (etatx){
                case -1 :
                    printf("un peu en haut\n");
                    vecteur[1]=-2; 
                    vecteur[1]=1; 
                    break;

                case -2 :
                    printf("tres en haut\n");
                    vecteur[0]=-2; 
                    vecteur[1]=2; 
                    break;
                
                case 1 :
                    printf("un peu en bas\n");
                    vecteur[0]=2; 
                    vecteur[1]=1;
                    break; 
                
                case 2 :
                    printf("tres en bas\n");
                    vecteur[0]=2; 
                    vecteur[1]=2;
                    break; 
            }
        default:
            printf("\n");
    }

}


