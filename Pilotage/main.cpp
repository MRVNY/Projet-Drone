#define EXTERN

extern "C" {
#include "Pilotage.h"
}
#include "../Bas_niveau/bas_niveau.hpp"

int main()
{   
    //Pointeur de fonction pointant sur la fonction de "Bas_niveau" utilisant opencv 
    int (*functionPtr)(const char*);
    functionPtr=&video_reader_process;

    //On le passe au main de la partie pilotage (C)
    main_Pilotage(functionPtr);
    return 0;
}
