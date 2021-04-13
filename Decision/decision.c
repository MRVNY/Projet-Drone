#include<stdio.h> 
#include<stdlib.h> 
#include "decision.h"
#include <math.h>
#include "../commun.h" 
// #include "../Pilotage/Pilotage.h"

// les variables globales
int sortie[4][3]; // le tableau de la sortie 
float dx_precedent, dy_precedent, dz_precedent ; // la distance entre le centre de la mire et celui de l'image 
int hirondelle_defined[4]; // un tableau de bool sur la disponiblité ou pas des horondelles
int nb_hirondelle_valide = 0;  // le nombre des hirondelle définies 


int is_far_left(int a){
    return(a<(TAILLE_Y/8)*2);
}

int is_left(int a){
    return(a<(TAILLE_Y/8)*3);
}

int is_far_right(int a){
    return(a>(TAILLE_Y/8)*6);
}

int is_right(int a){
    return(a>(TAILLE_Y/8)*5);
}

int is_top(int a){
    return (a<(TAILLE_X/8)*2);
}

int is_mid_top(int a){
    return(a<(TAILLE_X/8)*3);
}

int is_bottom(int a){
    return(a>(TAILLE_Y/8)*6);
}

int is_mid_bottom(int a){
    return(a>(TAILLE_X/8)*5);
}

void current_state_y(int **cordonnee,int **tab){  
    /*
    Cette fonction renvoie l'etat du dronne selon l'image actuellement recue 
    y1 étant le deuxième cordonnée de la première abeille et y2 de la deuxième abeille
    args : 
        cordonnee: les cordonnee des hirondelle recu sous forme d'une matrice 4*2 
        tab:  le vecteur de sortie
    */
    int y1,y2;
    int verifier=0; // int utiliser pour verifier le cas exeptionnel ou on a seulement 2 points mais on est pas dans l'extremité sur l'axe des y

    if(nb_hirondelle_valide==2){
        if(hirondelle_defined[0]==0 && hirondelle_defined[2]==0){ // on est à l'extrémité gauche 
            tab[STRAFER][POSITION]=NEGATIF;
            tab[STRAFER][INTENSITE]=EXTREME;
            verifier=1;
        }
        if(hirondelle_defined[1]==0 && hirondelle_defined[3]==0){ // on est à l'extrémité droite 
            tab[STRAFER][POSITION]=POSITIF;
            tab[STRAFER][INTENSITE]=EXTREME;
            verifier=1;
        }
    }    

    if(verifier==0){
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
        if(is_far_left(y1)){
            tab[STRAFER][POSITION]=NEGATIF;
            tab[STRAFER][INTENSITE]=FAR;

        }
        else{
            if(is_left(y1)){
                tab[STRAFER][POSITION]=NEGATIF;
                tab[STRAFER][INTENSITE]=CLOSE;
            }
            else{
                if(is_far_right(y2)){
                    tab[STRAFER][POSITION]=POSITIF;
                    tab[STRAFER][INTENSITE]=FAR;
                }
                else{
                    if(is_right(y2)){
                        tab[STRAFER][POSITION]=POSITIF;
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
    int verifier=0; // int utiliser pour verifier le cas exeptionnel ou on a seulement 2 points mais on est pas dans l'extremité sur l'axe des y
    if(nb_hirondelle_valide==2){
        
        if(hirondelle_defined[0]==0 && hirondelle_defined[1]==0){ // on est à l'extrémité haut
            tab[MONTER_DESCENDRE][POSITION]=POSITIF;
            tab[MONTER_DESCENDRE][INTENSITE]=EXTREME;
            verifier=1;
        }
        if(hirondelle_defined[2]==0 && hirondelle_defined[3]==0){ // on est à l'extrémité bas

            tab[MONTER_DESCENDRE][POSITION]=NEGATIF;
            tab[MONTER_DESCENDRE][INTENSITE]=EXTREME;
            verifier=1;
        }
    }    

    if(verifier==0){
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

        if(is_top(y1)){
            tab[MONTER_DESCENDRE][POSITION]=POSITIF;
            tab[MONTER_DESCENDRE][INTENSITE]=FAR;
        }
        else{
            if(is_mid_top(y1)){
                tab[MONTER_DESCENDRE][POSITION]=POSITIF;
                tab[MONTER_DESCENDRE][INTENSITE]=CLOSE;
            }
            else{
                if(is_bottom(y2)){
                    tab[MONTER_DESCENDRE][POSITION]=NEGATIF;
                    tab[MONTER_DESCENDRE][INTENSITE]=FAR;
                }
                else{
                    if(is_mid_bottom(y2)){
                        tab[MONTER_DESCENDRE][POSITION]=NEGATIF;
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

int get_nb_pixel(int **cordonnee){
    if(hirondelle_defined[0]==1 && hirondelle_defined[1]==1){
        return abs(cordonnee[0][1]-cordonnee[1][1]);
    }
    else{
        if(hirondelle_defined[2]==1 && hirondelle_defined[3]==1){ 
            return abs(cordonnee[2][1]-cordonnee[3][1]);
        }
        else{
            if(hirondelle_defined[0]==1 && hirondelle_defined[2]==1){
                return abs(cordonnee[0][0]-cordonnee[2][0]);
            }
            else{
                return abs(cordonnee[1][0]-cordonnee[3][0]);
            }
        }
    }
}

void current_state_z(int **cordonnee, int **tab){
    // a ce stade normalement on voit toute la mire sous forme d'un carrée 
    int nb_pixel = get_nb_pixel(cordonnee); // renvoie le nombre de pixels entre les hirondelles 
    
    if(nb_pixel>= BORNE_FAR_BACK){ // on est trop loin à l'arrière 
        tab[MONTER_DESCENDRE][POSITION]=NEGATIF;
        tab[MONTER_DESCENDRE][INTENSITE]=FAR;
    }
    else{
        if(nb_pixel>= BORNE_CLOSE_BACK){ // on est trop loin à l'arrière 
            tab[MONTER_DESCENDRE][POSITION]=NEGATIF;
            tab[MONTER_DESCENDRE][INTENSITE]=CLOSE;
        }
        else{
            if(nb_pixel >= BORNE_AXE){ // on est trop loin à l'arrière 
                tab[MONTER_DESCENDRE][POSITION]=AXE;
                tab[MONTER_DESCENDRE][INTENSITE]=AXE;
            }
            else{
                if(nb_pixel >= BORNE_CLOSE_FRONT){ // on est trop loin à l'arrière 
                    tab[MONTER_DESCENDRE][POSITION]= POSITIF;
                    tab[MONTER_DESCENDRE][INTENSITE]=CLOSE;         
                }
                else{
                    if(nb_pixel >= BORNE_FAR_FRONT){ // on est trop loin à l'arrière 
                        tab[MONTER_DESCENDRE][POSITION]=POSITIF;
                        tab[MONTER_DESCENDRE][INTENSITE]=FAR;         
                    }
                }
            }

        }
    }
}


void isDefine(int **cordonnee, int *hirondelle_defined, int *nb_hirondelle_valide){
    // renvoie un tableau de bool et le nombre des hirondelle renseignées
    *nb_hirondelle_valide=0; 
    for(int k=0; k<TAILLE ; k++){
            hirondelle_defined[k]=0; 
    }
    for(int k=0; k<TAILLE ; k++){
        if(cordonnee[k][0]>0 && cordonnee[k][1]>0){
            hirondelle_defined[k]=1;  // pour dir qu'elle est renseigné 
            *nb_hirondelle_valide+=1;
        }
    }
    printf("nombre des points definit %d \n",*nb_hirondelle_valide);
    for(int k=0; k<TAILLE;k++){
        printf("%d ",hirondelle_defined[k]);
    }
    printf("\n");
}


void calcule_dx_dy(int **cordonnee, float *dx, float *dy){
    printf("je suis dans le calcule des dx et dy\n");
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
        *dy= abs(CENTREIMAGEY-(cordonnee[a1][1]+cordonnee[a2][1])/2);
        
    }


}

void analyseInterpretation(int **cordonnees){
    /*
        Cette fonction renvoie la commande à transmettre selon l'état du drone 
        vecteur : le résultat de la partie 
        cordonnees: les cordonnées des hirondelles 
    */

    // les variables: 
    printf("je suis dans la fonction interprete\n");
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
        current_state_x(cordonnees,vecteur); // estimation de la position sur y 
        current_state_z(cordonnees, vecteur); // estimation de la position sur z


        
        calcule_dx_dy(cordonnees,&dx, &dy); // on calcule les nouvelle distances 
        if ( dx_precedent==0 && dy_precedent==0){// initialiser dx_precedent et dy_precedent avec les premieres valeurs dx et dy dans la nouvelle etat
            dx_precedent=dx;
            dy_precedent=dy;
        }
        // traitement de straffer:
        if((sortie[STRAFER][POSITION]==vecteur[STRAFER][POSITION])&& (sortie[STRAFER][INTENSITE]==vecteur[STRAFER][INTENSITE])){ // on regarde si on est tjr dans la meme etat
            if(dy <= dy_precedent){
                sortie[STRAFER][EVALUATION] = GOOD;
            }
            else{
                sortie[STRAFER][EVALUATION] = BAD;
            }
        }
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
        dx_precedent = dx; //mettre ajour les distance d'Evaluation
        dy_precedent = dy;
        
        //if((sortie[ROTATION][POSITION]==vecteur[ROTATION][POSITION])&& (sortie[ROTATION][INTENSITE]==vecteur[ROTATION][INTENSITE])){ // on regarde si on est tjr dans la mm zone 
        //     if(dy <= dy_precedent){
        //         sortie[ROTATION][EVALUATION] = GOOD;
        //     }
        //     else{
        //         sortie[ROTATION][EVALUATION] = BAD;
        //     }
        // }
        if((sortie[AVANT_ARRIERE][POSITION]==vecteur[AVANT_ARRIERE][POSITION]) && (sortie[AVANT_ARRIERE][INTENSITE]==vecteur[AVANT_ARRIERE][INTENSITE])){ // on regarde si on est tjr dans la mm zone 
            if(get_nb_pixel(cordonnees)<= dz_precedent){
                sortie[AVANT_ARRIERE][EVALUATION] = GOOD;
            }
            else{
                sortie[AVANT_ARRIERE][EVALUATION] = BAD;
            }
        }


    }
}

}


