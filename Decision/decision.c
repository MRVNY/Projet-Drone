#include<stdio.h> 
#include<stdlib.h> 
#include "decision.h"
#include <math.h>
#include "../commun.h" 
// #include "../Pilotage/Pilotage.h"



// les variables globales
int sortie[4][3]; // le tableau de la sortie 
float dx_precedent, dy_precedent ; // la distance entre le centre de la mire et celui de l'image 
int hirondelle_defined[4]; // un tableau de bool sur la disponiblité ou pas des horondelles
int nb_hirondelle_valide = 0;  // le nombre des hirondelle définies 


void current_state_y(int **cordonnee,int **tab){  
    /*
    Cette fonction renvoie l'etat du dronne selon l'image actuellement recue 
    y1 étant le deuxième cordonnée de la première abeille et y2 de la deuxième abeille
    args : 
        cordonnee: les cordonnee des hirondelle recu sous forme d'une matrice 4*2 
        tab:  le vecteur de sortie
    */
    int y1,y2;
    if(nb_hirondelle_valide==2){
        if(hirondelle_defined[0]==0 && hirondelle_defined[2]==0){ // on est à l'extrémité NEGATIVE 
            tab[STRAFER][POSITION]=NEGATIVE;
            tab[STRAFER][INTENSITE]=EXTREME;
        }
        if(hirondelle_defined[1]==0 && hirondelle_defined[3]==0){ // on est à l'extrémité POSITIVE 
            tab[STRAFER][POSITION]=POSITIVE;
            tab[STRAFER][INTENSITE]=EXTREME;
        }
    }    

    if(nb_hirondelle_valide>=3){
        if(hirondelle_defined[0]==1){
            y1= cordonnee[0][1];
        }
        else{
            y1= cordonnee[2][1];
        }
        if(hirondelle_defined[1]==1){
            y2= cordonnee[1][1];
        }
        else{
            y2= cordonnee[3][1]; 
        }
        if(y1<(TAILLE_Y/8)*2){
            tab[STRAFER][POSITION]=NEGATIVE;
            tab[STRAFER][INTENSITE]=FAR;

        }
        else{
            if(y1<(TAILLE_Y/8)*3){
                tab[STRAFER][POSITION]=NEGATIVE;
                tab[STRAFER][INTENSITE]=CLOSE;
            }
            else{
                if(y2>(TAILLE_Y/8)*6){
                    tab[STRAFER][POSITION]=POSITIVE;
                    tab[STRAFER][INTENSITE]=FAR;
                }
                else{
                    if(y2>(TAILLE_Y/8)*5){
                        tab[STRAFER][POSITION]=POSITIVE;
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
}



void current_state_x(int **cordonnee, int **tab){
    /*
    Cette fonction renvoie l'etat du dronne selon l'image actuellement recue 
    y1 étant le deuxième cordonnée de la première abeille et y2 de la deuxième abeille
    */
 
    // on calcule la différence entre les y pour extimer l'état du drone

    int y1,y2;
    if(nb_hirondelle_valide==2){
        if(hirondelle_defined[0]==0 && hirondelle_defined[1]==0){ // on est à l'extrémité NEGATIVE 
            tab[STRAFER][POSITION]=POSITIVE;
            tab[STRAFER][INTENSITE]=EXTREME;
        }
        if(hirondelle_defined[2]==0 && hirondelle_defined[3]==0){ // on est à l'extrémité POSITIVE 
            tab[STRAFER][POSITION]=NEGATIVE;
            tab[STRAFER][INTENSITE]=EXTREME;
        }
    }    

    if(nb_hirondelle_valide>=3){
        if(hirondelle_defined[0]==1){
            y1= cordonnee[0][0];
        }
        else{
            y1= cordonnee[1][0];
        }
        if(hirondelle_defined[2]==1){
            y2= cordonnee[2][0];
        }
        else{
            y2= cordonnee[3][0]; 
        }

        if(y1<(TAILLE_X/8)*2){
            tab[MONTER_DESCENDRE][POSITION]=POSITIVE;
            tab[MONTER_DESCENDRE][INTENSITE]=FAR;
        }
        else{
            if(y1<(TAILLE_X/8)*3){
                tab[MONTER_DESCENDRE][POSITION]=POSITIVE;
                tab[MONTER_DESCENDRE][INTENSITE]=CLOSE;
            }
            else{
                if(y2>(TAILLE_X/8)*6){
                    tab[MONTER_DESCENDRE][POSITION]=NEGATIVE;
                    tab[MONTER_DESCENDRE][INTENSITE]=FAR;
                }
                else{
                    if(y2>(TAILLE_X/8)*5){
                        tab[MONTER_DESCENDRE][POSITION]=NEGATIVE;
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
}




void isDefine(int **cordonnee,int *hirondelle_defined, int *nb_hirondelle_valide){
    // renvoie un tableau de bool et le nombre des hirondelle renseignées
    for(int k=0; k<TAILLE; k++){
        if(cordonnee[k][0]>0 && cordonnee[k][1]>0){
            hirondelle_defined[k]=1;  // pour dir qu'elle est renseigné 
            nb_hirondelle_valide++;
        }
    }
}



void calcule_dx_dy(int **cordonnee, float *dx, float *dy){
    // calcule la distance entre le centre de l'image et celui de la mire 
    int a1, a2;

    if(nb_hirondelle_valide >=3){ // donc la on aura une bonne evaluation du déplacement car on peut calculer le bon centre 
        // traitement sur y 
        if(hirondelle_defined[0]==1 && hirondelle_defined[1]==1){
            a1 = cordonnee[0][1];
            a2 = cordonnee[1][1];
        }
        else{
            a1 = cordonnee[2][1];
            a2 = cordonnee[3][1];
        }
        *dy= abs(CENTREIMAGEY-(a1+a2)/2);

        // traitement sur x
        if(hirondelle_defined[0]==1 && hirondelle_defined[2]==1){
                a1 = cordonnee[0][0];
                a2 = cordonnee[2][0];
        }
        else{
            a1 = cordonnee[1][0];
            a2 = cordonnee[3][0];
        }
        *dx= abs(CENTREIMAGEX-(a1+a2)/2);
    }

    if(nb_hirondelle_valide ==2 ){
        // on cherche les hirondelle définies : 

        if(hirondelle_defined[0]==1 && hirondelle_defined[1]==1 ){
            a1 = 0;
            a2 = 1;
        }
        if(hirondelle_defined[0]==1 && hirondelle_defined[1]==1 ){
            a1 = 0;
            a2 = 2;
        }
        if(hirondelle_defined[1]==1 && hirondelle_defined[3]==1 ){
            a1 = 1;
            a2 = 3;
        }
        if(hirondelle_defined[2]==1 && hirondelle_defined[3]==1 ){
            a1 = 2;
            a2 = 3;
        }
        *dx= abs(CENTREIMAGEX-(cordonnee[a1][0]+cordonnee[a2][0])/2);
        *dy= abs(CENTREIMAGEX-(cordonnee[a1][1]+cordonnee[a2][1])/2);
    }


}



void analyseInterpretation(int **cordonnees){
    /*
        Cette fonction renvoie la commande à transmettre selon l'état du drone 
        vecteur : le résultat de la partie 
        cordonnees: les cordonnées des hirondelles 
    */

    // les variables: 
    float dx,dy;
    // la création de la matrice de sortie 
    int **vecteur=(int **)malloc(sizeof(int)*TAILLE_SORTIE); // la sortie de la partie decision 
    if(vecteur){
        for(int k=0; k<TAILLE_SORTIE;k++){ 
            vecteur[k] = (int*)malloc(sizeof(int)*INFO_SORTIE); // 3 est le nombre d'information que contient chque ligne dans la matrice de sortie 
            if(!vecteur[k]){
                perror("Erreur lors de l'allocation  mémoire \n");
            }
        }   
    }
    else{
        perror("Erreur lors de l'allocation  mémoire \n");
    }

    // la traitement sur les coordonnées recues 
    isDefine(cordonnees, hirondelle_defined, &nb_hirondelle_valide);

    if(nb_hirondelle_valide <=1){ // deja on sais avec que ca qu'on est à l'extrémité 
        // on regard deja la position de l'état précédent et on 
        for(int i=0;i<=TAILLE_SORTIE;i++){
            if(sortie[i][INTENSITE]==FAR){
                if(sortie[i][EVALUATION]==BAD){
                    sortie[i][INTENSITE]==EXTREME;
                    sortie[i][EVALUATION]==0;
                }
            }
        }
    }
    else{
        current_state_y(cordonnees,vecteur); // estimation de la position sur x 
        current_state_x(cordonnees,vecteur); // apres on appelle mm z 

        calcule_dx_dy(cordonnees,&dx, &dy); // on calcule les nouvelle distances 
        // traitement de straffer:
        if((sortie[STRAFER][POSITION]==vecteur[STRAFER][POSITION])&& (sortie[STRAFER][INTENSITE]==vecteur[STRAFER][INTENSITE])){ // on regarde si on est tjr dans la mm zone 
            if(dy <= dy_precedent){
                sortie[STRAFER][EVALUATION] = GOOD;
            }
            else{
                sortie[STRAFER][EVALUATION] = BAD;
            }
        }
<<<<<<< HEAD
        else{
            sortie[STRAFER][POSITION]=vecteur[STRAFER][POSITION];
            sortie[STRAFER][INTENSITE]=vecteur[STRAFER][INTENSITE];
            sortie[STRAFER][EVALUATION]=0;
        }
        if((sortie[MONTER_DESCENDRE][POSITION]==vecteur[MONTER_DESCENDRE][POSITION])&& (sortie[MONTER_DESCENDRE][INTENSITE]==vecteur[MONTER_DESCENDRE][INTENSITE])){ // on regarde si on est tjr dans la mm zone 
            if(dx <= dx_precedent){
                sortie[MONTER_DESCENDRE][EVALUATION] = GOOD;
            }
            else{
                sortie[MONTER_DESCENDRE][EVALUATION] = BAD;
            }
        }
        else{
            sortie[MONTER_DESCENDRE][POSITION]=vecteur[MONTER_DESCENDRE][POSITION];
            sortie[MONTER_DESCENDRE][INTENSITE]=vecteur[MONTER_DESCENDRE][INTENSITE];
            sortie[MONTER_DESCENDRE][EVALUATION]=0;
        }



        // A venir 
        // if((sortie[ROTATION][POSITION]==vecteur[ROTATION][POSITION])&& (sortie[ROTATION][INTENSITE]==vecteur[ROTATION][INTENSITE])){ // on regarde si on est tjr dans la mm zone 
        //     if(dy <= dy_precedent){
        //         sortie[ROTATION][EVALUATION] = GOOD;
        //     }
        //     else{
        //         sortie[ROTATION][EVALUATION] = BAD;
        //     }
        // }
        // if((sortie[avant_arriere][POSITION]==vecteur[avant_arriere][POSITION])&& (sortie[avant_arriere][INTENSITE]==vecteur[avant_arriere][INTENSITE])){ // on regarde si on est tjr dans la mm zone 
        //     if(dy <= dy_precedent){
        //         sortie[avant_arriere][EVALUATION] = GOOD;
        //     }
        //     else{
        //         sortie[avant_arriere][EVALUATION] = BAD;
        //     }
        // }


    }
}


=======
    }
    int k;
    for(k=0;k<2;k++){
       printf(" etat: %d \n intensité: %d \n",vecteur[0],vecteur[1]); 
    }
    // pilotage(vecteur) 
}

>>>>>>> 90e95d1ce4c8652fbd75e54af77c4e16e28e47fa
