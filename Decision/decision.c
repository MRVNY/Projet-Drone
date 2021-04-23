#include<stdio.h> 
#include<stdlib.h> 
#include "decision.h"
#include <math.h>
#include "../commun.h"
#include "../Pilotage/Pilotage.h"

// les variables globales
int sortie[4][2]; // le tableau de la sortie 
float dx_precedent = 0, dy_precedent = 0, dz_precedent = 0, dr_precedent = 0; // la distance entre le centre de la mire et celui de l'image 
int hirondelle_defined[4]; // un tableau de bool sur la disponiblité ou pas des horondelles
int nb_hirondelle_valide = 0;  // le nombre des hirondelle définies 



int is_far_left(int a){
    // la fonction qui renvoie si le drone est trop à gauche et false sinon 
    return(a<(TAILLE_Y/8)*2);
}

int is_left(int a){
    // la fonction qui renvoie 1 si le drone est à gauche  
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
            tab[STRAFER][POS_INTENSITE]=POSITIF*EXTREME;
            verifier=1;
        }
        if(hirondelle_defined[1]==0 && hirondelle_defined[3]==0){ // on est à l'extrémité droite 
            tab[STRAFER][POS_INTENSITE]=NEGATIF*EXTREME;
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
            tab[STRAFER][POS_INTENSITE]=POSITIF*FAR;

        }
        else{
            if(is_left(y1)){
                tab[STRAFER][POS_INTENSITE]=POSITIF*CLOSE;
            }
            else{
                if(is_far_right(y2)){
                    tab[STRAFER][POS_INTENSITE]=NEGATIF*FAR;
                }
                else{
                    if(is_right(y2)){
                        tab[STRAFER][POS_INTENSITE]=NEGATIF*CLOSE;
                    }
                    else{
                        tab[STRAFER][POS_INTENSITE]=AXE;
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
            tab[MONTER_DESCENDRE][POS_INTENSITE]=NEGATIF*EXTREME;
            verifier=1;
        }
        if(hirondelle_defined[2]==0 && hirondelle_defined[3]==0){ // on est à l'extrémité bas

            tab[MONTER_DESCENDRE][POS_INTENSITE]=POSITIF*EXTREME;
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
            tab[MONTER_DESCENDRE][POS_INTENSITE]=NEGATIF*FAR;
        }
        else{
            if(is_mid_top(y1)){
                tab[MONTER_DESCENDRE][POS_INTENSITE]=NEGATIF*CLOSE;
            }
            else{
                if(is_bottom(y2)){
                    tab[MONTER_DESCENDRE][POS_INTENSITE]=POSITIF*FAR;
                }
                else{
                    if(is_mid_bottom(y2)){
                        tab[MONTER_DESCENDRE][POS_INTENSITE]=POSITIF*CLOSE;
                    }
                    else{
                        tab[MONTER_DESCENDRE][POS_INTENSITE]=AXE;
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
        tab[MONTER_DESCENDRE][POS_INTENSITE]=NEGATIF*FAR;
    }
    else{
        if(nb_pixel>= BORNE_CLOSE_BACK){ // on est trop loin à l'arrière 
            tab[MONTER_DESCENDRE][POS_INTENSITE]=NEGATIF*CLOSE;
        }
        else{
            if(nb_pixel >= BORNE_AXE){ // on est trop loin à l'arrière 
                tab[MONTER_DESCENDRE][POS_INTENSITE]=AXE;
            }
            else{
                if(nb_pixel >= BORNE_CLOSE_FRONT){ // on est trop loin à l'arrière 
                    tab[MONTER_DESCENDRE][POS_INTENSITE]= POSITIF*CLOSE;   
                }
                else{
                    if(nb_pixel >= BORNE_FAR_FRONT){ // on est trop loin à l'arrière 
                        tab[MONTER_DESCENDRE][POS_INTENSITE]=POSITIF*FAR;         
                    }
                }
            }

        }
    }
}

float calcul_ratio1( int a , int b , int c){
    // calcule le ratio entre (a-b)/(a-c)
    return abs(a-b)/abs(a-c);
}

float calcul_ratio2( int a , int b , int c){
    // calcule le ratio entre (a-b)/(a-c)
    return abs(a-b)/(2*abs(a-b)+abs(b-c));
}

int is_much_turned(float ratio){
    return ratio > BORNE_FAR_ROTATION;
}

int is_less_turned(float ratio){
    return ratio > BORNE_CLOSE_ROTATION;
}

int get_direction( int a , int b ){
    if (a<b){
        return POSITIF;
    }else{
        if (a>b){
            return NEGATIF;
        }else{
            return AXE;
        }
    }
}

void direction_ratio( int **coordonnee, int *direction, float *ratio ){
    int a1,a2,a3;
    if(hirondelle_defined[0]==1 && hirondelle_defined[1]==1){ 
        a1=0;
        a2=1;
        *direction=get_direction(a1,a2);
        if (*direction==POSITIF){
            if (hirondelle_defined[2]==1)
                *ratio=calcul_ratio1(coordonnee[a1][0],coordonnee[a2][0],coordonnee[2][0]);
            else
                *ratio=calcul_ratio2(coordonnee[a1][0],coordonnee[a2][0],coordonnee[3][0]);
        }else{
            if(*direction==NEGATIF){
                if (hirondelle_defined[3]==1)
                    *ratio=calcul_ratio1(coordonnee[a2][0],coordonnee[a1][0],coordonnee[3][0]);
                else
                    *ratio=calcul_ratio2(coordonnee[a2][0],coordonnee[a1][0],coordonnee[2][0]);
            }else{
                *ratio=0;
            }
        }
    }else{
        if (hirondelle_defined[2]==1 && hirondelle_defined[3]==1){
            a1=2;
            a2=3;
            *direction=get_direction(a1,a2);
            if (*direction==POSITIF){
                if (hirondelle_defined[0]==1)
                    *ratio=calcul_ratio1(coordonnee[a1][0],coordonnee[a2][0],coordonnee[0][0]);
                else
                    *ratio=calcul_ratio2(coordonnee[a1][0],coordonnee[a2][0],coordonnee[1][0]);
            }else{
                if(*direction==NEGATIF){
                    if (hirondelle_defined[1]==0)
                        *ratio=calcul_ratio1(coordonnee[a2][0],coordonnee[a1][0],coordonnee[1][0]);
                    else
                        *ratio=calcul_ratio2(coordonnee[a2][0],coordonnee[a1][0],coordonnee[0][0]);
                }else{
                    *ratio=0;
                }
            }
        }
        
    }    

}

void current_state_rotation(int **cordonnee,int **tab){
    // cette fonction marche que dans le cas ou on a recu 4 ou 3 cordonnées
    int direction;
    float ratio;
    direction_ratio(cordonnee, &direction, &ratio );
    if(direction == AXE){
        tab[ROTATION][POS_INTENSITE] = AXE ;
    }else{
        if(is_much_turned(ratio)){
            tab[ROTATION][POS_INTENSITE] = direction * FAR ;
        }
        else{
            if(is_less_turned(ratio)){
                tab[ROTATION][POS_INTENSITE] = direction * CLOSE ; 
            }
            else{
                tab[ROTATION][POS_INTENSITE] = AXE;
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
    if(nb_hirondelle_valide==0)
        exit(1);
    //printf("nombre des points definit %d \n",*nb_hirondelle_valide);
    // for(int k=0; k<TAILLE;k++){
    //     printf("%d ",hirondelle_defined[k]);
    // }
    // printf("\n");
}

void calcule_dx_dy(int **cordonnee, float *dx, float *dy){
    //printf("je suis dans le calcule des dx et dy\n");
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

int analyseInterpretation_x_y(int **cordonnees, int **vecteur){
    float dx,dy;
    // -------------------------------------- traitement de strafer et monter decendre  ----------------------------------------------------------------------------
    current_state_y(cordonnees,vecteur); // estimation de la position sur x 
    current_state_x(cordonnees,vecteur); // estimation de la position sur y 

    // pour chaque axe ; regarder si on est dans la meme etat DONC on doit EVALUER le déplacement sinon on renvoit le nouvel ETAT 
    calcule_dx_dy(cordonnees,&dx, &dy); // on calcule les nouvelle distances 
    // if ( dx_precedent==0 && dy_precedent==0){// initialiser dx_precedent et dy_precedent avec les premieres valeurs dx et dy dans la nouvelle etat
    //     dx_precedent=dx;
    //     dy_precedent=dy;
    // }
    if(sortie[STRAFER][POS_INTENSITE]==vecteur[STRAFER][POS_INTENSITE] && sortie[STRAFER][POS_INTENSITE]!=AXE){ // on regarde si on est tjr dans le meme etat
        if(dy <= dy_precedent){
            sortie[STRAFER][EVALUATION] = GOOD;
        }
        else{
            sortie[STRAFER][EVALUATION] = BAD;
        }
    }
    else{
        sortie[STRAFER][POS_INTENSITE]=vecteur[STRAFER][POS_INTENSITE];
        sortie[STRAFER][EVALUATION]=0; 

    }

    if(sortie[MONTER_DESCENDRE][POS_INTENSITE]==vecteur[MONTER_DESCENDRE][POS_INTENSITE] && sortie[MONTER_DESCENDRE][POS_INTENSITE]!=AXE){ // on regarde si on est tjr dans la mm zone 
        if(dx <= dx_precedent){
            sortie[MONTER_DESCENDRE][EVALUATION] = GOOD;
        }
        else{
            sortie[MONTER_DESCENDRE][EVALUATION] = BAD;
        }
    }
    else{
        sortie[MONTER_DESCENDRE][POS_INTENSITE]=vecteur[MONTER_DESCENDRE][POS_INTENSITE];
        sortie[MONTER_DESCENDRE][EVALUATION]=0;
    }
    dx_precedent = dx; //mettre ajour les distance d'Evaluation
    dy_precedent = dy;
    return (sortie[STRAFER][POS_INTENSITE]==AXE && sortie[MONTER_DESCENDRE][POS_INTENSITE]==AXE);

}

float calcule_dr(int **cordonnees){
    if(hirondelle_defined[0]==1 && hirondelle_defined[1]==1){
        return abs(cordonnees[0][1]-cordonnees[1][1]);
    }
    else{
        if(hirondelle_defined[2]==1 && hirondelle_defined[3]==1){
            return abs(cordonnees[2][1]-cordonnees[3][1]);
        }
    }
}

int analyseInterpretation_rotation(int **cordonnees,int **vecteur){
    current_state_rotation(cordonnees,vecteur); // estimation de la rotation
    int dr = calcule_dr(cordonnees);
    if(sortie[ROTATION][POS_INTENSITE] == vecteur[ROTATION][POS_INTENSITE]&&sortie[ROTATION][POS_INTENSITE]!=AXE){ // on regarde si on est tjr dans la mm zone 
        if(dr <= dr_precedent){
            sortie[ROTATION][EVALUATION] = GOOD;
        }
        else{
            sortie[ROTATION][EVALUATION] = BAD;
        }
    }
    dr_precedent = dr;
    return (sortie[ROTATION][POS_INTENSITE]==AXE); // si on dans l'AXE meme por la ROTATION on peut alors avancer ou reculer
}

int analyseInterpretation_z(int **cordonnees, int **vecteur){
    current_state_z(cordonnees, vecteur);
    // if (dz_precedent==0){ // initialiser dz_precedent  avec la premiere valeur dz dans le nouvel etat
    //     dz_precedent= get_nb_pixel(cordonnees);
    // }
    int dz = get_nb_pixel(cordonnees);
    if(sortie[AVANT_ARRIERE][POS_INTENSITE]==vecteur[AVANT_ARRIERE][POS_INTENSITE]&& sortie[AVANT_ARRIERE][POS_INTENSITE]!=AXE){ // on regarde si on est tjr dans la mm zone 
        if(dz < dz_precedent){
            if(sortie[AVANT_ARRIERE][POS_INTENSITE]<0){ // SI ON EST DEJA EN ARRIRE  si la distance augmente on avance bien si elle diminue on avance mal 
                sortie[AVANT_ARRIERE][EVALUATION] = BAD;
            }else{
                sortie[AVANT_ARRIERE][EVALUATION] = GOOD;
            }
            
        }
        else{
            if(dz > dz_precedent){
                if(sortie[AVANT_ARRIERE][POS_INTENSITE]<0){
                    sortie[AVANT_ARRIERE][EVALUATION] = GOOD;
                }else{
                    sortie[AVANT_ARRIERE][EVALUATION] = BAD;
                }
            }else{
                sortie[AVANT_ARRIERE][EVALUATION] = GOOD;
            }  
        }
    }
    else{
        sortie[AVANT_ARRIERE][POS_INTENSITE] = vecteur[AVANT_ARRIERE][POS_INTENSITE];
        sortie[AVANT_ARRIERE][EVALUATION]=0;
       
    }
    dz_precedent = dz;
    return (sortie[AVANT_ARRIERE][POS_INTENSITE]==AXE);
}

void analyseInterpretation(int **cordonnees){
    /*
        Cette fonction renvoie la commande à transmettre selon l'état du drone 
        vecteur : le résultat de la partie 
        cordonnees: les cordonnées des hirondelles 
    */

    // les variables: 

    // creation d'un fichier de résultat 
    //FILE* fichier = fopen("tets.txt", "a");
    
   // on écrit les cordonnées données par la partie imagerie 
    // for(int i=0; i<4; i++){
    //     for (int k=0; k<2;k++){
    //         fprintf(fichier,"[%d] ",cordonnees[i][k]);
    //     }
    //     fprintf(fichier,"\n");
    // }
    // fprintf(fichier,"\n");
    // ___________________________la création de la matrice de sortie____________________________________________________
    int **vecteur=(int **)malloc(sizeof(int*)*TAILLE_SORTIE); // la sortie de la partie decision 
    if(!vecteur){
        fprintf(stderr,"Erreur lors de l'allocation  mémoire \n");
    }
    
    for(int k=0; k<TAILLE_SORTIE;k++){ 
        vecteur[k] = (int*)malloc(sizeof(int)*INFO_SORTIE); // 3 est le nombre d'information que contient chque ligne dans la matrice de sortie 
        if(!vecteur[k]){
            fprintf(stderr,"Erreur lors de l'allocation  mémoire \n");
        }
    }
            
    for(int i=0; i<TAILLE_SORTIE; i++){
        for (int k=0; k<INFO_SORTIE;k++){
            vecteur[i][k]=0;
        }
    }
    // ___________________________________________________________________________________________________________________
    // la traitement sur les coordonnées recues 
    isDefine(cordonnees, hirondelle_defined, &nb_hirondelle_valide);


    if(nb_hirondelle_valide ==0){
        //printf("on est dans le cas 0"); 
    }
    else{
        if(nb_hirondelle_valide ==1){ // deja on sais avec que ca qu'on est à l'extrémité 
            //printf("on est dans le cas 1");
        }
        else{
            if(analyseInterpretation_x_y(cordonnees,vecteur)){ // si on est dans l'AXE sur les axes x et y on peut faire la rotation
                // if(analyseInterpretation_rotation(cordonnees,vecteur)){
                //     int res_z = analyseInterpretation_z(cordonnees,vecteur);  // estimation de la position sur z
                // }
                   
            }
        }
    }

    sortie[MONTER_DESCENDRE][POS_INTENSITE]=0;
    sortie[MONTER_DESCENDRE][EVALUATION]=0;


    //pilotage(vecteur);
    callback(sortie,1); // pour l'instant c'est 0 

    // fprintf(fichier,"le résulat d'analyse \n"); //: [[5,100],[5,250],[150,100],[150,250]]\n");
    // printf("le résulat d'analyse \n\n"); 
    // for(int i=0; i<TAILLE_SORTIE; i++){
    //     fprintf(fichier,"sortie[%d]\n",i);
    //     printf("sortie[%d]\n",i);
    //     for (int k=0; k<INFO_SORTIE;k++){
    //         fprintf(fichier,"%d ___",sortie[i][k]);
    //         printf("%d ___",sortie[i][k]);
    //     }
    //     fprintf(fichier,"\n");
    //     printf("\n");
    // }


    // fprintf(fichier,"*******************  ICI VECTEUR ******************************\n");
    // for(int i=0; i<TAILLE_SORTIE; i++){
    //     fprintf(fichier,"vecteur[%d]\n",i);
    //     for(int k=0; k<INFO_SORTIE;k++){
    //         fprintf(fichier,"%d _",vecteur[i][k]);
    //     }
    //     fprintf(fichier,"\n");
    // }
    for(int i=0; i<TAILLE_SORTIE; i++){
        free(vecteur[i]);
    }
    free(vecteur);


}

