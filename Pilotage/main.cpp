extern "C" {
#include "Pilotage.h"
}
#include "../Bas_niveau/bas_niveau.hpp"

int main()
{   
    int** (*functionPtr)(const char*);
    functionPtr=&video_reader_process;
    main_Pilotage(functionPtr);
    return 0;
}
