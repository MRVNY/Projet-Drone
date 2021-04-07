extern "C" {
#include "Pilotage.h"
}
#include "../Bas_niveau/bas_niveau.hpp"

int main()
{   
    void (*functionPtr)(const char*);
    functionPtr=&video_reader_process;
    main_Pilotage(functionPtr);
    return 0;
}
