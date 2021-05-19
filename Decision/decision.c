#include <stdio.h>
#include <stdlib.h>
#include "decision.h"
#include <math.h>
#include "../commun.h"
#include "../Pilotage/Pilotage.h"



// les variables globales
float dx_precedent = 0, dy_precedent = 0, dz_precedent = 0, dr_precedent = 0; // la distance entre le centre de la mire et celui de l'image
int hirondelle_defined[4];                                                    // un tableau de bool sur la disponiblité ou pas des horondelles
int nb_hirondelle_valide = 0;                                                 // le nombre des hirondelle définies
int index_courant = 0;                                                        // l'indice de la case du tableau à lire
int index_historique = 0;                                                     // l'indice de la case du tableau qui contient l'historique
int compteur_indefined = 10; // a fixer avec les résultat de l'algo de bas niveau



int is_far_left(int a)
{
    // la fonction qui renvoie si le drone est trop à gauche et false sinon
    return (a < (TAILLE_Y / 8) * 2);
}

int is_left(int a)
{
    // la fonction qui renvoie 1 si le drone est à gauche
    return (a < (TAILLE_Y / 8) * 3);
}

int is_far_right(int a)
{
    return (a > (TAILLE_Y / 8) * 6);
}

int is_right(int a)
{
    return (a > (TAILLE_Y / 8) * 5);
}

int is_top(int a)
{
    return (a < (TAILLE_X / 8) * 2);
}

int is_mid_top(int a)
{
    return (a < (TAILLE_X / 8) * 3);
}

int is_bottom(int a)
{
    return (a > (TAILLE_X / 8) * 6);
}

int is_mid_bottom(int a)
{
    return (a > (TAILLE_X / 8) * 5);
}

void current_state_y(int **cordonnee)
{
    /*
    Cette fonction renvoie l'etat du drone selon l'image actuellement recue 
    y1 étant le deuxième coordonnée de la première abeille et y2 de la deuxième abeille
    args : 
        cordonnee: les cordonnee des hirondelle recu sous forme d'une matrice 4*2 
        tab:  le vecteur de sortie
    */

    int y1, y2;
    int verifier = 0; // int utiliser pour verifier le cas exeptionnel ou on a seulement 2 points mais on est pas dans l'extremité sur l'axe des y

    if (nb_hirondelle_valide == 2)
    {
        if (hirondelle_defined[0] == 0 && hirondelle_defined[2] == 0)
        { // on est à l'extrémité gauche
            tab_Sestimatin[index_courant].matrice[STRAFER][POS_INTENSITE] = POSITIF * EXTREME;
            verifier = 1;
        }
        if (hirondelle_defined[1] == 0 && hirondelle_defined[3] == 0)
        { // on est à l'extrémité droite
            tab_Sestimatin[index_courant].matrice[STRAFER][POS_INTENSITE] = NEGATIF * EXTREME;
            verifier = 1;
        }
    }

    if (verifier == 0)
    {
        if (hirondelle_defined[0] == 1)
        {
            y1 = cordonnee[0][1];
        }
        else
        {
            y1 = cordonnee[2][1];
        }
        if (hirondelle_defined[1] == 1)
        {
            y2 = cordonnee[1][1];
        }
        else
        {
            y2 = cordonnee[3][1];
        }
        if (is_far_left(y1))
        {
            tab_Sestimatin[index_courant].matrice[STRAFER][POS_INTENSITE] = POSITIF * FAR;
        }
        else
        {
            if (is_left(y1))
            {
                tab_Sestimatin[index_courant].matrice[STRAFER][POS_INTENSITE] = POSITIF * CLOSE;
            }
            else
            {
                if (is_far_right(y2))
                {

                    tab_Sestimatin[index_courant].matrice[STRAFER][POS_INTENSITE] = NEGATIF * FAR;
                }
                else
                {
                    if (is_right(y2))
                    {

                        tab_Sestimatin[index_courant].matrice[STRAFER][POS_INTENSITE] = NEGATIF * CLOSE;
                    }
                    else
                    {

                        tab_Sestimatin[index_courant].matrice[STRAFER][POS_INTENSITE] = AXE;
                    }
                }
            }
        }
    }
}

void current_state_x(int **cordonnee)
{
    /*
    Cette fonction renvoie l'etat du dronne selon l'image actuellement recue 
    y1 étant le deuxième cordonnée de la première abeille et y2 de la deuxième abeille
    */

    // on calcule la différence entre les y pour extimer l'état du drone
    int y1, y2;
    int verifier = 0; // int utiliser pour verifier le cas exeptionnel ou on a seulement 2 points mais on est pas dans l'extremité sur l'axe des y
    if (nb_hirondelle_valide == 2)
    {

        if (hirondelle_defined[0] == 0 && hirondelle_defined[1] == 0)
        { // on est à l'extrémité haut
            tab_Sestimatin[index_courant].matrice[MONTER_DESCENDRE][POS_INTENSITE] = NEGATIF * EXTREME;
            verifier = 1;
        }
        if (hirondelle_defined[2] == 0 && hirondelle_defined[3] == 0)
        { // on est à l'extrémité bas

            tab_Sestimatin[index_courant].matrice[MONTER_DESCENDRE][POS_INTENSITE] = POSITIF * EXTREME;
            verifier = 1;
        }
    }

    if (verifier == 0)
    {
        if (hirondelle_defined[0] == 1)
        {
            y1 = cordonnee[0][0];
        }
        else
        {
            y1 = cordonnee[1][0];
        }
        if (hirondelle_defined[2] == 1)
        {
            y2 = cordonnee[2][0];
        }
        else
        {
            y2 = cordonnee[3][0];
        }

        if (is_top(y1))
        {
            tab_Sestimatin[index_courant].matrice[MONTER_DESCENDRE][POS_INTENSITE] = NEGATIF * FAR;
        }
        else
        {
            if (is_mid_top(y1))
            {
                tab_Sestimatin[index_courant].matrice[MONTER_DESCENDRE][POS_INTENSITE] = NEGATIF * CLOSE;
            }
            else
            {
                if (is_bottom(y2))
                {
                    tab_Sestimatin[index_courant].matrice[MONTER_DESCENDRE][POS_INTENSITE] = POSITIF * FAR;
                }
                else
                {
                    if (is_mid_bottom(y2))
                    {
                        tab_Sestimatin[index_courant].matrice[MONTER_DESCENDRE][POS_INTENSITE] = POSITIF * CLOSE;
                    }
                    else
                    {
                        tab_Sestimatin[index_courant].matrice[MONTER_DESCENDRE][POS_INTENSITE] = AXE;
                    }
                }
            }
        }
    }
}

int get_nb_pixel(int **cordonnee)
{
    if (hirondelle_defined[0] == 1 && hirondelle_defined[1] == 1)
    {
        return abs(cordonnee[0][1] - cordonnee[1][1]);
    }
    else
    {
        if (hirondelle_defined[2] == 1 && hirondelle_defined[3] == 1)
        {
            return abs(cordonnee[2][1] - cordonnee[3][1]);
        }
        else
        {
            if (hirondelle_defined[0] == 1 && hirondelle_defined[2] == 1)
            {
                return abs(cordonnee[0][0] - cordonnee[2][0]);
            }
            else
            {
                return abs(cordonnee[1][0] - cordonnee[3][0]);
            }
        }
    }
}

void current_state_z(int **cordonnee)
{   
    // a ce stade normalement on voit toute la mire sous forme d'un carrée
    int nb_pixel = get_nb_pixel(cordonnee); // renvoie le nombre de pixels entre les hirondelles
    if (nb_pixel <= BORNE_FAR_BACK)
    { // on est trop loin à l'arrière
        
        tab_Sestimatin[index_courant].matrice[AVANT_ARRIERE][POS_INTENSITE] = NEGATIF * FAR;
        
    }
    else
    {
        if (nb_pixel <= BORNE_CLOSE_BACK){
             // on est trop loin à l'arrière
            tab_Sestimatin[index_courant].matrice[AVANT_ARRIERE][POS_INTENSITE] = NEGATIF * CLOSE;
        }
        else
        {
            if (nb_pixel <= BORNE_CLOSE_FRONT){
                 // axe
                tab_Sestimatin[index_courant].matrice[AVANT_ARRIERE][POS_INTENSITE] = AXE;
            }
            else
            {
                tab_Sestimatin[index_courant].matrice[AVANT_ARRIERE][POS_INTENSITE] = POSITIF * CLOSE;
            }
        }
    }
}

float calcul_ratio1(int a, int b, int c)
{
    return ((float)abs(a - b) / abs(a - c));
}

float calcul_ratio2(int a, int b, int c)
{
    return (float)abs(a - b) / (2 * abs(a - b) + abs(b - c));
}

int is_much_turned(float ratio)
{
    return ratio > BORNE_FAR_ROTATION;
}

int is_less_turned(float ratio)
{
    return ratio > BORNE_CLOSE_ROTATION;
}

int get_direction(int a, int b)
{
    // printf("je suis dans get direction\n");
    if (a < b)
    {
        return POSITIF;
    }
    else
    {
        if (a > b)
        {
            return NEGATIF;
        }
        else
        {
            return AXE;
        }
    }
}

void direction_ratio(int **coordonnee, int *direction, float *ratio)
{   
    int a1, a2, a3;
    float a;
    if (hirondelle_defined[0] == 1 && hirondelle_defined[1] == 1)
    {
        a1 = 0 ;
        a2 = 1 ;
        *direction = get_direction(coordonnee[a1][0],coordonnee[a2][0]);
        
        if (*direction == POSITIF)
        {
            
            if (hirondelle_defined[2] == 1){
                *ratio = calcul_ratio1(coordonnee[a1][0], coordonnee[a2][0], coordonnee[2][0]);
            }
            else{
                *ratio = calcul_ratio2(coordonnee[a1][0], coordonnee[a2][0], coordonnee[3][0]);
            }
        }
        else
        {
            if (*direction == NEGATIF)
            {
                if (hirondelle_defined[3] == 1){
                    *ratio = calcul_ratio1(coordonnee[a2][0], coordonnee[a1][0], coordonnee[3][0]);
                }
                else{
                    *ratio = calcul_ratio2(coordonnee[a2][0], coordonnee[a1][0], coordonnee[2][0]);
                }
            }
            else
            {
                *ratio = 0;
            }
        }
    }
    else
    {
        if (hirondelle_defined[2] == 1 && hirondelle_defined[3] == 1)
        {
            a1 = 2;
            a2 = 3;
            *direction = get_direction(coordonnee[a1][0], coordonnee[a2][0]);
            if (*direction == POSITIF)
            {
                if (hirondelle_defined[0] == 1){
                    *ratio = calcul_ratio1(coordonnee[a1][0], coordonnee[a2][0], coordonnee[0][0]);
                }
                else{
                    *ratio = calcul_ratio2(coordonnee[a1][0], coordonnee[a2][0], coordonnee[1][0]);
                }
            }
            else
            {
                if (*direction == NEGATIF)
                {
                    if (hirondelle_defined[1] == 0){
                        *ratio = calcul_ratio1(coordonnee[a2][0], coordonnee[a1][0], coordonnee[1][0]);
                    }
                    else{
                        *ratio = calcul_ratio2(coordonnee[a2][0], coordonnee[a1][0], coordonnee[0][0]);
                    }
                }
                else
                {
                    *ratio = 0;
                }
            }
        }
    }
}

void current_state_rotation(int **cordonnee)
{   
    // cette fonction marche que dans le cas ou on a recu 4 ou 3 cordonnées
    int direction;
    float ratio;
    direction_ratio(cordonnee, &direction, &ratio);
    if (direction == AXE)
    {
        tab_Sestimatin[index_courant].matrice[ROTATION][POS_INTENSITE] = AXE;
    }
    else
    {
        if (is_much_turned(ratio))

        {
            tab_Sestimatin[index_courant].matrice[ROTATION][POS_INTENSITE] = direction * FAR;
        }
        else
        {
            if (is_less_turned(ratio))
            {
                tab_Sestimatin[index_courant].matrice[ROTATION][POS_INTENSITE] = direction * CLOSE;
            }
            else
            {
                tab_Sestimatin[index_courant].matrice[ROTATION][POS_INTENSITE] = AXE;
            }
        }
    }
}

void isDefine(int **cordonnee, int *hirondelle_defined, int *nb_hirondelle_valide)
{
    // renvoie un tableau de bool et le nombre des hirondelle renseignées
    *nb_hirondelle_valide = 0;
    for (int k = 0; k < TAILLE; k++)
    {
        hirondelle_defined[k] = 0;
    }
    for (int k = 0; k < TAILLE; k++)
    {
        if (cordonnee[k][0] > 0 && cordonnee[k][1] > 0)
        {
            hirondelle_defined[k] = 1; // pour dir qu'elle est renseigné
            *nb_hirondelle_valide += 1;
        }
    }
    if (nb_hirondelle_valide == 0)
        exit(1);
}

void calcule_dx_dy(int **cordonnee, float *dx, float *dy)
{
    // calcule la distance entre le centre de l'image et celui de la mire
    int a1, a2;

    if (nb_hirondelle_valide >= 3)
    { // donc la on aura une bonne evaluation du déplacement car on peut calculer le bon centre
        // traitement sur y
        if (hirondelle_defined[0] == 1 && hirondelle_defined[1] == 1)
        {
            a1 = cordonnee[0][1];
            a2 = cordonnee[1][1];
        }
        else
        {
            a1 = cordonnee[2][1];
            a2 = cordonnee[3][1];
        }
        *dy = abs(CENTREIMAGEY - (a1 + a2) / 2);

        // traitement sur x
        if (hirondelle_defined[0] == 1 && hirondelle_defined[2] == 1)
        {
            a1 = cordonnee[0][0];
            a2 = cordonnee[2][0];
        }
        else
        {
            a1 = cordonnee[1][0];
            a2 = cordonnee[3][0];
        }
        *dx = abs(CENTREIMAGEX - (a1 + a2) / 2);
    }

    if (nb_hirondelle_valide == 2)
    {
        // on cherche les hirondelle définies :

        if (hirondelle_defined[0] == 1 && hirondelle_defined[1] == 1)
        {
            a1 = 0;
            a2 = 1;
        }
        if (hirondelle_defined[0] == 1 && hirondelle_defined[1] == 1)
        {
            a1 = 0;
            a2 = 2;
        }
        if (hirondelle_defined[1] == 1 && hirondelle_defined[3] == 1)
        {
            a1 = 1;
            a2 = 3;
        }
        if (hirondelle_defined[2] == 1 && hirondelle_defined[3] == 1)
        {
            a1 = 2;
            a2 = 3;
        }
        *dx = abs(CENTREIMAGEX - (cordonnee[a1][0] + cordonnee[a2][0]) / 2);
        *dy = abs(CENTREIMAGEY - (cordonnee[a1][1] + cordonnee[a2][1]) / 2);
    }
}

int analyseInterpretation_x_y(int **cordonnees)
{
    float dx, dy;
    // -------------------------------------- traitement de strafer et monter decendre  ----------------------------------------------------------------------------
    current_state_y(cordonnees); // estimation de la position sur x
    current_state_x(cordonnees); // estimation de la position sur y

    // pour chaque axe ; regarder si on est dans la meme etat DONC on doit EVALUER le déplacement sinon on renvoit le nouvel ETAT
    calcule_dx_dy(cordonnees, &dx, &dy); // on calcule les nouvelle distances
    
    if (dx_precedent == 0 && dy_precedent == 0)
    { // initialiser dx_precedent et dy_precedent avec les premieres valeurs dx et dy dans la nouvelle etat
        dx_precedent = dx;
        dy_precedent = dy;
    }
    if (tab_Sestimatin[index_historique].matrice[STRAFER][POS_INTENSITE] == tab_Sestimatin[index_courant].matrice[STRAFER][POS_INTENSITE] && tab_Sestimatin[index_courant].matrice[STRAFER][POS_INTENSITE] != AXE)
    { // on regarde si on est tjr dans le meme etat
        if (dy < dy_precedent)
        {
            tab_Sestimatin[index_courant].matrice[STRAFER][EVALUATION] = GOOD;
        }
        else
        {
            if (dy > dy_precedent)
            {   
                tab_Sestimatin[index_courant].matrice[STRAFER][EVALUATION] = BAD;
            }
            else
            {
                tab_Sestimatin[index_courant].matrice[STRAFER][EVALUATION] = AXE;
            }
        }
    }
    else
    {
        //sortie[STRAFER][POS_INTENSITE]=vecteur[STRAFER][POS_INTENSITE];
        tab_Sestimatin[index_courant].matrice[STRAFER][EVALUATION] = 0;
    }

    if (tab_Sestimatin[index_historique].matrice[MONTER_DESCENDRE][POS_INTENSITE] == tab_Sestimatin[index_courant].matrice[MONTER_DESCENDRE][POS_INTENSITE] && tab_Sestimatin[index_courant].matrice[MONTER_DESCENDRE][POS_INTENSITE] != AXE)
    { // on regarde si on est tjr dans la mm zone
        if (dx < dx_precedent)
        {
            tab_Sestimatin[index_courant].matrice[MONTER_DESCENDRE][EVALUATION] = GOOD;
        }
        else
        {
            if (dx > dx_precedent)
            {
                tab_Sestimatin[index_courant].matrice[MONTER_DESCENDRE][EVALUATION] = BAD;
            }
            else
            {
                tab_Sestimatin[index_courant].matrice[MONTER_DESCENDRE][EVALUATION] = AXE;
            }
        }
    }
    else
    {
        // sortie[MONTER_DESCENDRE][POS_INTENSITE]=vecteur[MONTER_DESCENDRE][POS_INTENSITE];
        tab_Sestimatin[index_courant].matrice[MONTER_DESCENDRE][EVALUATION] = 0;
    }
    dx_precedent = dx; //mettre ajour les distance d'Evaluation
    dy_precedent = dy;
    
    return (tab_Sestimatin[index_courant].matrice[STRAFER][POS_INTENSITE] == AXE && tab_Sestimatin[index_courant].matrice[MONTER_DESCENDRE][POS_INTENSITE] == AXE);
}

float calcule_dr(int **cordonnees)
{
    if (hirondelle_defined[0] == 1 && hirondelle_defined[1] == 1)
    {
        return abs(cordonnees[0][1] - cordonnees[1][1]);
    }
    else
    {
        if (hirondelle_defined[2] == 1 && hirondelle_defined[3] == 1)
        {
            return abs(cordonnees[2][1] - cordonnees[3][1]);
        }
    }
}

int analyseInterpretation_rotation(int **cordonnees){
    current_state_rotation(cordonnees); // estimation de la rotation
    int dr = calcule_dr(cordonnees);
    if (dr_precedent == 0 )
    { 
        dr_precedent = dr;
    }
    if(tab_Sestimatin[index_historique].matrice[ROTATION][POS_INTENSITE] == tab_Sestimatin[index_courant].matrice[ROTATION][POS_INTENSITE]&&tab_Sestimatin[index_courant].matrice[ROTATION][POS_INTENSITE]!=AXE){ // on regarde si on est tjr dans la mm zone
        if(dr < dr_precedent){
            tab_Sestimatin[index_courant].matrice[ROTATION][EVALUATION] = GOOD;
        }
        else{
            if(dr > dr_precedent){
                tab_Sestimatin[index_courant].matrice[ROTATION][EVALUATION] = BAD;
            }
            else{
                tab_Sestimatin[index_courant].matrice[ROTATION][EVALUATION] = AXE;
            }
        }
    }
    dr_precedent = dr;
    return (tab_Sestimatin[index_courant].matrice[ROTATION][POS_INTENSITE]==AXE); // si on dans l'AXE meme pour la ROTATION on peut alors avancer ou reculer
}

int analyseInterpretation_z(int **cordonnees){
    current_state_z(cordonnees);
    int dz = get_nb_pixel(cordonnees);

    if (dz_precedent == 0 )
    { 
        dz_precedent = dz;
    }
    if((tab_Sestimatin[index_historique].matrice[AVANT_ARRIERE][POS_INTENSITE]==tab_Sestimatin[index_courant].matrice[AVANT_ARRIERE][POS_INTENSITE]) && tab_Sestimatin[index_courant].matrice[AVANT_ARRIERE][POS_INTENSITE] != AXE ){//&& (tab_Sestimatin[index_courant].matrice[AVANT_ARRIERE][POS_INTENSITE]!=AXE)){ // on regarde si on est tjr dans la mm zone
        if(dz < dz_precedent){
            if(tab_Sestimatin[index_courant].matrice[AVANT_ARRIERE][POS_INTENSITE]<0){ // SI ON EST DEJA EN ARRIRE  si la distance augmente on avance bien si elle diminue on avance mal
                tab_Sestimatin[index_courant].matrice[AVANT_ARRIERE][EVALUATION] = BAD;
            }else{
                tab_Sestimatin[index_courant].matrice[AVANT_ARRIERE][EVALUATION] = GOOD;
            }

        }
        else{
            if(dz > dz_precedent){
                if(tab_Sestimatin[index_courant].matrice[AVANT_ARRIERE][POS_INTENSITE]<0){
                    tab_Sestimatin[index_courant].matrice[AVANT_ARRIERE][EVALUATION] = GOOD;
                }else{
                    tab_Sestimatin[index_courant].matrice[AVANT_ARRIERE][EVALUATION] = BAD;
                }
            }else{
                tab_Sestimatin[index_courant].matrice[AVANT_ARRIERE][EVALUATION] = AXE;
            }
        }
    }
    else{
        tab_Sestimatin[index_courant].matrice[AVANT_ARRIERE][EVALUATION]=0;

    }
    dz_precedent = dz;
    return (tab_Sestimatin[index_courant].matrice[AVANT_ARRIERE][POS_INTENSITE]==AXE);
}

void analyseInterpretation(int **cordonnees)
{

    /*
        Cette fonction renvoie la commande à transmettre selon l'état du drone 
        vecteur : le résultat de la partie 
        cordonnees: les cordonnées des hirondelles 
    */

    // les variables:

    // creation d'un fichier de résultat
    FILE* fichier = fopen("test_decision.txt", "a");

    //on écrit les cordonnées données par la partie imagerie
    fprintf(fichier," les coordonnées reçues \n");
    for(int i=0; i<4; i++){
        for (int k=0; k<2;k++){
            //printf("[%d]",cordonnees[i][k]);
            fprintf(fichier,"[%d] ",cordonnees[i][k]);
            //printf("[%d] ",cordonnees[i][k]);
        }
        fprintf(fichier,"\n");
        //printf("\n");
        //printf("\n");
    }
    fprintf(fichier,"\n");


    // ___________________________________________________________________________________________________________________
    // la traitement sur les coordonnées recues
    for (int i = 0; i < TAILLE_SORTIE; i++)
    {
        for (int j = 0; j < INFO_SORTIE; j++)
        {
            tab_Sestimatin[index_courant].matrice[i][j] = 0;
        }
    }

    isDefine(cordonnees, hirondelle_defined, &nb_hirondelle_valide);

    if (nb_hirondelle_valide == 0){
        compteur_indefined--; 
        if(compteur_indefined == 0){
            compteur_indefined= 10; 
            index_courant = 0;
            index_historique = 0;
            dx_precedent = 0;
            dy_precedent = 0; 
            dz_precedent = 0;
            dr_precedent = 0;
            // mettre l'historique à 0 
            for(int k=0; k<TAILLE_SEQ; k++){
                for (int i = 0; i < TAILLE_SORTIE; i++){
                    for (int j = 0; j < INFO_SORTIE; j++){
                        tab_Sestimatin[k].matrice[i][j] = 0;
                    }
                }
            }
            

            callbackPilote(index_courant,STOP); // on s'arrête un instant 
           //fprintf(fichier," on est perdu \n");

            
        }
        else{
            callbackPilote(index_historique,1);
            fprintf(fichier,"le résulat d'analyse \n"); 
            for(int i=0; i<TAILLE_SORTIE; i++){
                fprintf(fichier,"sortie[%d]\n",i);
                for (int k=0; k<INFO_SORTIE;k++){
                    fprintf(fichier,"%d ___",tab_Sestimatin[index_historique].matrice[i][k]);
                }
                fprintf(fichier,"\n");

            }
        }


    }
    else
    {
        if (nb_hirondelle_valide == 1 ||nb_hirondelle_valide == 2||nb_hirondelle_valide == 3)
        { // deja on sais avec que ca qu'on est à l'extrémité
        }
        else
        {
            if (analyseInterpretation_x_y(cordonnees))
            { // si on est dans l'AXE sur les axes x et y on peut faire la rotation
               
                if(analyseInterpretation_z(cordonnees)){
                    int res_R = analyseInterpretation_rotation(cordonnees);  // estimation de la position sur z                                                 
                }
            }
        }
        

       callbackPilote(index_courant,1); // pour l'instant c'est 0
        fprintf(fichier,"le résulat d'analyse \n"); 
        for(int i=0; i<TAILLE_SORTIE; i++){
            fprintf(fichier,"sortie[%d]\n",i);
            for (int k=0; k<INFO_SORTIE;k++){
                fprintf(fichier,"%d ___",tab_Sestimatin[index_courant].matrice[i][k]);
            }
            fprintf(fichier,"\n");

        }
        // for(int i=0; i<TAILLE_SORTIE; i++){

        //     printf("sortie[%d]\n",i);

        //     for (int k=0; k<INFO_SORTIE;k++){
        //         printf("%d ___",tab_Sestimatin[index_courant].matrice[i][k]);

        //     }

        //     printf("\n");   
        // }
        
 
        // l'affichage de l'historique 

        // printf(" \n ici c'est l historique \n ");
        // for(int i=0; i<TAILLE_SORTIE; i++){

        //     printf("sortie[%d]\n",i);

        //     for (int k=0; k<INFO_SORTIE;k++){
        //         printf("%d ___",tab_Sestimatin[index_historique].matrice[i][k]);

        //     }

        //     printf("\n");   
        // }



        if (index_courant == 0 && index_historique == 0){
            index_courant++;
        }
        else{
            if (index_courant < TAILLE_SEQ-1)
            {
                index_courant++;
            }
            else
            {
                index_courant = 0;
            }
            if (index_historique < TAILLE_SEQ-1)
            {
                index_historique++;
            }
            else
            {
                index_historique = 0;
            }
        }
    }



}