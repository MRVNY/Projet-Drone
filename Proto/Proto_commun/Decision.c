
#include "Decision.h"
#include <stdio.h>

int n=0;
int m=2;
int **tableauCoord(char* nomfichier){

    FILE* f=fopen(nomfichier,"r");
    char chr = getc(f);
    
    int k=0;
    int i;
    int j;

    while (chr != EOF)
    {
        //printf("%d_",chr);
        if (chr == '\n')
        {
            //printf("ici");
            n = n + 1;
        }
        chr = getc(f);
    }
  //int coordTab[n][2];
  int **coordTab = malloc(sizeof(int*) * n);
  for(i = 0; i < n; i++) {
      coordTab[i] = malloc(sizeof(int*) * 2);
  }
  fseek(f, 0, SEEK_SET);

  while ((fscanf (f, "%d",&i)!=EOF)&&(fscanf(f,"%d" ,&j)!= EOF))
  {
    int tab[2];
    coordTab[k][0]=i;
    coordTab[k][1]=j;
    k++;
  }
      
    /*for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < 2; j++)
        {   
            printf("%d\n",coordTab[i][j]);
        }
        
    }*/

    fclose(f);
    return coordTab;
}

//Code déplacement:
/*
1->avant
-1->arrière
2->droite
-2->gauche
  */
int *bouchDecisionTab1(int tab[],int ref,int max,int min){
  int *tabDec=malloc(sizeof(int*)*2);
  
  if(tab[0]-ref<0)
  {
    tabDec[0]=-1;
    if(tab[0]<(min+ref)/2){
      tabDec[1]=1;
    }
    else{
      tabDec[1]=2;
    }
  }
  else{
    tabDec[0]=1;
    if(tab[0]<(max-ref)/2){
      tabDec[1]=1;
    }
    else{
      tabDec[1]=2;
    }

  }

  return tabDec;
}