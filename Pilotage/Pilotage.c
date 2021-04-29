#include "Pilotage.h"
#include "../commun.h"
#include <curses.h>

static void cmdBatteryStateChangedRcv(ARCONTROLLER_Device_t *deviceController, ARCONTROLLER_DICTIONARY_ELEMENT_t *elementDictionary);

static void cmdSensorStateListChangedRcv(ARCONTROLLER_Device_t *deviceController, ARCONTROLLER_DICTIONARY_ELEMENT_t *elementDictionary);


/*--------------Variable globales---------------*/
//Vars globales Parrot
static char fifo_dir[] = FIFO_DIR_PATTERN;
static char fifo_name[128] = "";
int gIHMRun = 1;
int choice;
char gErrorStr[ERROR_STR_LENGTH];
FILE *videoOut = NULL;
ARCONTROLLER_Device_t *deviceController = NULL;
ARSAL_Sem_t stateSem;
ARDISCOVERY_Device_t *device = NULL;
eARCONTROLLER_ERROR error = ARCONTROLLER_OK;
eARCONTROLLER_DEVICE_STATE deviceState = ARCONTROLLER_DEVICE_STATE_MAX;

//Vars globales watchdog 
struct timeval counter, watch;
pid_t child = 0;
pthread_t threads;
char toPrint[50];

//Tableau 2D contenant les valeurs d'amplitudes de mouvements pour chaque déplacements 
int tabPrc[4][4]={
    {0, LOW_ROLL_ANGLE, MID_ROLL_ANGLE, HIGH_ROLL_ANGLE},
    {0, LOW_PITCH_ANGLE, MID_PITCH_ANGLE, HIGH_PITCH_ANGLE},
    {0, LOW_GAZ_SPEED, MID_GAZ_SPEED, HIGH_GAZ_SPEED},
    {0, LOW_ROT_SPEED, MID_ROT_SPEED, HIGH_ROT_SPEED}
};
/*-------------------------------------------------*/
int StateZero=0;


/*****************************************
 *
 *             implementation :
 *
 *****************************************/

// Afficher les infos supplementaires, activer avec IFPRINT
void myPrint(char *toPrint){
    if(IFPRINT) printf("%s",toPrint);
}

void *watch_dog(){
    while(1){
        usleep(CYCLE); //Lancer watchdog chaque CYCLE secondes
        if(counter.tv_sec!=0){ //Commencer a verifier apres le counter a ete modifie
            gettimeofday(&watch, NULL); //Recuperer le temps reel
            sprintf(toPrint,"watch: %lus %lums, counter: %lus %lums, diff: %lums\n",watch.tv_sec,watch.tv_usec, counter.tv_sec,counter.tv_usec, (watch.tv_sec - counter.tv_sec)*1000000+ watch.tv_usec - counter.tv_usec);
            myPrint(toPrint);
            if(((watch.tv_sec - counter.tv_sec) * 1000000 + watch.tv_usec - counter.tv_usec)>TIMEOUT){
                myPrint("WATCHDOG\n"); //S'il y a TIMEOUT secondes de decalage, endProg
                //MODIF
                if(deviceController!=NULL){
                    stop(deviceController);
                }
                //MODIF
                //endProg();
                break;
            }
        }
    }
    return 0;
}

// Attraper tous les signaux sauf control-Z
void catchSig(int sig){
    sprintf(toPrint,"CAUGHT %d\n",sig);
    myPrint(toPrint);
    endProg();
    return 0;
}

int main_Pilotage (int (*functionPtr)(const char*))
{
    //Local declaration
    int failed = 0;
    int fps;
    int frameNb = 0;
    int isBebop2 = 1;

    //Initialisation de enProg
    endProgState=0;
    start=0;
   // catch signaux
    int i;
    for(i = 1; i <=SIGRTMIN ; i++){
        if(i != SIGTSTP) signal(i,catchSig);
    }

    // Watch Dog
    pthread_create(&threads, NULL, watch_dog, NULL);

    /*---------Choix paramètre programme------------*/
    printf("\n Fly: oui(1), non(0)\n");
    if(scanf("%d",&choice)==0 || (choice!=1 && choice!=0)){
        printf("Entree non connue, no Fly par defaut\n");
        choice = 0;
        sleep(1);
    }
    if (choice==1)
    {   
        printf("Vol: oui");
        fly=1;
    }
    else if(choice==0)
    {
        printf("Vol: non");
        fly=0;
    }

    printf("\n Affichage caméra: oui(1), non(0)\n");
    if(scanf("%d",&choice)==0 || (choice!=1 && choice!=0)){
        printf("Entree non connue,  pas d'affichage par defaut\n");
        choice = 0;
        sleep(1);
    }
    if (choice==1)
    {   
        printf("Affichage caméra: oui\n\n");
        display=1;
    }
    else if(choice==0)
    {
        printf("Affichage caméra: non\n\n");
        display=0;
    }
    
    printf("\nVideoCapture (0), mplayer(1) ou ffmpeg(2)?\n");
    if(scanf("%d",&choice)==0 || (choice!=2 && choice!=1 && choice!=0)){
        printf("Entree non connue, VideoCapture par defaut\n");
        choice = 0;
        sleep(1);
    }
    else if(choice==1){
        printf("mplayer\n");
        sleep(1);
    }
    if(choice==2){
        printf("FPS(1-24)?\n");
        if(scanf("%d",&fps)==0 || fps>24 || fps<1){
            printf("Entree non connue, 2 fps par defaut\n");
            fps = 2;
            sleep(1);
        }
    } 
    if(choice==0){
        myPrint("VideoCapture\n");
        sleep(1);
    } 
    /*------------------------------------------------------*/

    if (mkdtemp(fifo_dir) == NULL)
    {
        ARSAL_PRINT(ARSAL_PRINT_ERROR, "ERROR", "Mkdtemp failed.");
        return 1;
    }
    snprintf(fifo_name, sizeof(fifo_name), "%s/%s", fifo_dir, FIFO_NAME);

    if(mkfifo(fifo_name, 0666) < 0)
    {
        ARSAL_PRINT(ARSAL_PRINT_ERROR, "ERROR", "Mkfifo failed: %d, %s", errno, strerror(errno));
        return 1;
    }

    ARSAL_Sem_Init (&(stateSem), 0, 0);

    //ARSAL_PRINT(ARSAL_PRINT_INFO, TAG, "-- Bebop 2 Test TakeOff and Landing --");
 
    if (!failed)
    {
        if (DISPLAY_WITH_MPLAYER)
        {
            if(choice==0) pthread_create(&threads, NULL, functionPtr, fifo_name);

            // fork the process to launch mplayer
            else if (choice!=0 && (child = fork()) == 0)
            {
                if(choice==2){
                    char str[5];
                    sprintf(str,"%d",fps);
                    execlp("ffmpeg", "ffmpeg", "-f", "h264", "-i", fifo_name, "-vf", "scale=-1:720", "-r",str, "outputs/%04d.jpeg", NULL);
                    //execlp("ffmpeg", "ffmpeg", "-f", "h264", "-i", fifo_name,"-vcodec","copy","-vf", "scale=-1:720", "-acodec","none", "/tmp/test.mp4", NULL);
                }
                if(choice==1){
                    execlp("xterm", "xterm", "-e", "mplayer", "-demuxer",  "h264es", fifo_name, "-benchmark", "-really-quiet", NULL);
                    ARSAL_PRINT(ARSAL_PRINT_ERROR, TAG, "Missing mplayer, you will not see the video. Please install mplayer and xterm.");
                }
                return -1;
                //(*functionPtr)(fifo_name);
            }
        }
        if (DISPLAY_WITH_MPLAYER)
        {
            videoOut = fopen(fifo_name, "w");
        }
    }


/*****************************************
 *
 *       Connection au drone et création
 *          de l'interface de control
 *
 *****************************************/

discoverDevice(&failed,isBebop2);

controlDevice(&failed);

    
/*****************************************
 *
 *             Démarrage des instruction 
 *             de pilotage :
 *
 *****************************************/
    
    if (!failed){
       
        //On définit la vitesse max de rotation et  vitesse max verticale (85 °/s et 1 m/s)
        deviceController->aRDrone3->sendSpeedSettingsMaxVerticalSpeed(deviceController->aRDrone3,1 );
        deviceController->aRDrone3->sendSpeedSettingsMaxRotationSpeed(deviceController->aRDrone3, 85);
        
        if(fly)
        {
            takeOff(deviceController);
        }
        start=1;

        //sleep(5);
        //roll(deviceController,20);
        
        //Test catchSig
        //sleep(1000);

        //Test Watchdog
        /*
        for(i=0;i<200;i++){
            usleep(125000); //3/24
            callback(NULL,1);
        }
        while(1){
            //wait to be killed
        }*/

        pthread_join(threads, NULL);
    }
    
    
/*****************************************
 *
 *             Fin des instruction 
 *             de pilotage :
 *
 *****************************************/

// we are here because of a disconnection or user has quit IHM, so safely delete everything
    endProg();
    return EXIT_SUCCESS;
}


/*Définitions des fonctions de pilotage*/

//compteur pour ignorer un certain nombre de frames
int cpt =0;
void callbackPilote(int index,int ifStop){
    
    int NullError=0;
    int (*state)[4] = tab_Sestimatin[index].matrice;
    if (state==NULL)
    {
        NullError=1;
        myPrint("Erreur matrice nulle\n");
    }
    if(1){

        if(deviceController != NULL && !NullError){
            //Affichage de la matrice dedécision
            
            printf("STATE:\n");
            int i,j;
            for(i=0;i<4;i++){
                printf("[%d , %d]\n",state[i][0],state[i][1]);
            }
            
            //Arrêt de la commande en cour
            stop(deviceController);

            //Erreur dans les traitements précédents, mise en sécurité de l'appareil 
            if(ifStop==STOP){
                myPrint("Stop");
                //MAJ de la partie décision, le ifstop==STOP ne termine pas le programme
                //endProg();
                return;
            }

            //Tableau de la composition des mouvements
            int composition[4]={0,0,0,0};    

            if(state){
                //Parcour des différents mouvements
                for(int i=STRAFER; i<=STRAFER; i++) { //MODIF STRAFF
                    
                    //Test de l'évaluation
                    if(state[i][EVALUATION]==GOOD||state[i][EVALUATION]==0){
                    //if(1){
                        //Signe des déplacement (cf. common.h)
                        int sign=state[i][POS_INTENSITE]/abs(state[i][POS_INTENSITE]);
                        sign=sign*-1;
                        //On va définir l'amplitude de mouvement a appliquer pour chaque mvmts
                        switch (i)
                        {
                        case STRAFER:
                            //Modification pour ne prendre en compte que le STRAFF 
                            if (state[i][POS_INTENSITE]==AXE)
                            {   
                                StateZero++;
                                if (StateZero>10)
                                {
                                    stop(deviceController);
                                    land(deviceController);
                                    //endProgState=1;
                                    //sleep(2);
                                    myPrint("Fin\n");

                                    //pthread_cancel(threads);
                                    //endProg(); //MODIF STRAFF
                                    break;
                                }
                             
                            }
                          
                            else if(state[i][EVALUATION]==GOOD||state[i][EVALUATION]==0){
                                StateZero=0;
                                composition[STRAFER]=sign*choixPourcentage(state[i][POS_INTENSITE],STRAFER);
                            }
                            break;
                        case AVANT_ARRIERE:
                            composition[AVANT_ARRIERE]=sign*choixPourcentage(state[i][POS_INTENSITE],AVANT_ARRIERE);
                            break;
                        case MONTER_DESCENDRE:
                            composition[MONTER_DESCENDRE]=sign*choixPourcentage(state[i][POS_INTENSITE],MONTER_DESCENDRE);
                            break;
                        case ROTATION:
                            composition[ROTATION]=sign*choixPourcentage(state[i][POS_INTENSITE],ROTATION);
                            break;
                        default:
                            break;
                        }
                    }
                }
            }

            //On compose les mouvement que l'on envoie au drone
            roll(deviceController,composition[STRAFER]);

            /*-------TEST AXE X (uniquement straffer)---------
            pitch(deviceController,composition[AVANT_ARRIERE]); //MODIF STRAFF
            gaz(deviceController,composition[MONTER_DESCENDRE]);
            yaw(deviceController,composition[ROTATION]);
            */
            gettimeofday(&counter, NULL);
        }
    }
    cpt++;
}

//Retourne la valeur de pourcentage d'angle/vitt selon le type de mouvement (cf Pilotage.h)
int choixPourcentage(int pos_intensite, int type){
    switch (abs(pos_intensite))
                {
                case AXE:
                    return tabPrc[type][AXE];
                    break;
                case CLOSE:
                    return tabPrc[type][CLOSE];
                    break;
                case FAR:
                    return tabPrc[type][FAR];
                    break;
                case EXTREME:
                    return tabPrc[type][EXTREME];
                    break;
                default:
                    return 0;
                    break;
                }
}

void controlDevice(int *failed){
    if (!*failed)
    {
        deviceController = ARCONTROLLER_Device_New (device, &error);

        if (error != ARCONTROLLER_OK)
        {
            ARSAL_PRINT (ARSAL_PRINT_ERROR, TAG, "Creation of deviceController failed.");
            failed = 1;
        }
       
    }

    if (!*failed)
    {
        ARSAL_PRINT(ARSAL_PRINT_INFO, TAG, "- delete discovey device ... ");
        ARDISCOVERY_Device_Delete (&device);
    }

    // add the state change callback to be informed when the device controller starts, stops...
    if (!*failed)
    {
        error = ARCONTROLLER_Device_AddStateChangedCallback (deviceController, stateChanged, deviceController);

        if (error != ARCONTROLLER_OK)
        {
            ARSAL_PRINT (ARSAL_PRINT_ERROR, TAG, "add State callback failed.");
            failed = 1;
        }
    }

    // add the command received callback to be informed when a command has been received from the device
    if (!*failed)
    {
        error = ARCONTROLLER_Device_AddCommandReceivedCallback (deviceController, commandReceived, deviceController);

        if (error != ARCONTROLLER_OK)
        {
            ARSAL_PRINT (ARSAL_PRINT_ERROR, TAG, "add callback failed.");
            failed = 1;
        }
    }

    // add the frame received callback to be informed when a streaming frame has been received from the device
    if (!*failed)
    {
        ARSAL_PRINT(ARSAL_PRINT_INFO, TAG, "- set Video callback ... ");
        error = ARCONTROLLER_Device_SetVideoStreamCallbacks (deviceController, decoderConfigCallback, didReceiveFrameCallback, NULL , NULL);

        if (error != ARCONTROLLER_OK)
        {
            failed = 1;
            ARSAL_PRINT(ARSAL_PRINT_ERROR, TAG, "- error: %s", ARCONTROLLER_Error_ToString(error));
        }
    }

    if (!*failed)
    {
        ARSAL_PRINT(ARSAL_PRINT_INFO, TAG, "Connecting ...");
        error = ARCONTROLLER_Device_Start (deviceController);

        if (error != ARCONTROLLER_OK)
        {
            failed = 1;
            ARSAL_PRINT(ARSAL_PRINT_ERROR, TAG, "- error :%s", ARCONTROLLER_Error_ToString(error));
        }
    }

    if (!*failed)
    {
        // wait state update update
        ARSAL_Sem_Wait (&(stateSem));

        deviceState = ARCONTROLLER_Device_GetState (deviceController, &error);

        if ((error != ARCONTROLLER_OK) || (deviceState != ARCONTROLLER_DEVICE_STATE_RUNNING))
        {
            failed = 1;
            ARSAL_PRINT(ARSAL_PRINT_ERROR, TAG, "- deviceState :%d", deviceState);
            ARSAL_PRINT(ARSAL_PRINT_ERROR, TAG, "- error :%s", ARCONTROLLER_Error_ToString(error));
        }
    }

    // send the command that tells to the Bebop to begin its streaming
    if (!*failed)
    {
        ARSAL_PRINT(ARSAL_PRINT_INFO, TAG, "- send StreamingVideoEnable ... ");
        error = deviceController->aRDrone3->sendMediaStreamingVideoEnable (deviceController->aRDrone3, 1);
        if (error != ARCONTROLLER_OK)
        {
            ARSAL_PRINT(ARSAL_PRINT_ERROR, TAG, "- error :%s", ARCONTROLLER_Error_ToString(error));
            failed = 1;
        }
    }
}

void discoverDevice(int *failed,int isBebop2){
    if (!*failed)
    {
        ARSAL_PRINT(ARSAL_PRINT_INFO, TAG, "- init discovey device ... ");
        eARDISCOVERY_ERROR errorDiscovery = ARDISCOVERY_OK;

        device = ARDISCOVERY_Device_New (&errorDiscovery);

        if (errorDiscovery == ARDISCOVERY_OK)
        {
            ARSAL_PRINT(ARSAL_PRINT_INFO, TAG, "    - ARDISCOVERY_Device_InitWifi ...");
            // create a Bebop drone discovery device (ARDISCOVERY_PRODUCT_ARDRONE)

            if(isBebop2)
            {
                errorDiscovery = ARDISCOVERY_Device_InitWifi (device, ARDISCOVERY_PRODUCT_BEBOP_2, "bebop2", BEBOP_IP_ADDRESS, BEBOP_DISCOVERY_PORT);
            }
            else
            {
                errorDiscovery = ARDISCOVERY_Device_InitWifi (device, ARDISCOVERY_PRODUCT_ARDRONE, "bebop", BEBOP_IP_ADDRESS, BEBOP_DISCOVERY_PORT);
            }

            if (errorDiscovery != ARDISCOVERY_OK)
            {
                failed = 1;
                ARSAL_PRINT(ARSAL_PRINT_ERROR, TAG, "Discovery error :%s", ARDISCOVERY_Error_ToString(errorDiscovery));
            }
        }
        else
        {
            ARSAL_PRINT(ARSAL_PRINT_ERROR, TAG, "Discovery error :%s", ARDISCOVERY_Error_ToString(errorDiscovery));
            failed = 1;
        }
    }
}

void endProg(){
    /*------LOGS-------*/
    FILE *fp1 = fopen("Logs.csv", "w");// création du
    fprintf(fp1, "%s%s%s%s%s","Bas_nivea",",","Pilotage_decision",",","FrameCapture");
    fprintf(fp1,"\n");

    for (int i = 0; i<NB_VALS_LOGS;i++){
        if(tab_Logs.bas_niveau[i]!=0&&tab_Logs.pilotage_decsion[i]!=0&&tab_Logs.capture[i]!=0){
            fprintf(fp1, "%f%s%f%s%f",tab_Logs.bas_niveau[i],",",tab_Logs.pilotage_decsion[i],",",tab_Logs.capture[i]);
            fprintf(fp1,"\n");
        }
    }
    fclose(fp1);
    /*-----------------*/
    if (deviceController != NULL)
    {
        stop(deviceController);
        sleep(2);
        land(deviceController);

        deviceState = ARCONTROLLER_Device_GetState (deviceController, &error);
        if ((error == ARCONTROLLER_OK) && (deviceState != ARCONTROLLER_DEVICE_STATE_STOPPED))
        {
            ARSAL_PRINT(ARSAL_PRINT_INFO, TAG, "Disconnecting ...");

            error = ARCONTROLLER_Device_Stop (deviceController);

            if (error == ARCONTROLLER_OK)
            {
                // wait state update update
                ARSAL_Sem_Wait (&(stateSem));
            }
        }

        ARSAL_PRINT(ARSAL_PRINT_INFO, TAG, "ARCONTROLLER_Device_Delete ...");
        ARCONTROLLER_Device_Delete (&deviceController);

        if (DISPLAY_WITH_MPLAYER)
        {
            fflush (videoOut);
            fclose (videoOut);

            if (child > 0)
            {
                kill(child, SIGKILL);
            }
        }
    }

    ARSAL_Sem_Destroy (&(stateSem));

    unlink(fifo_name);
    rmdir(fifo_dir);

    ARSAL_PRINT(ARSAL_PRINT_INFO, TAG, "-- END --");

    //END Watch Dog
    //pthread_join(threads, NULL);
    exit(0);
}

static void cmdBatteryStateChangedRcv(ARCONTROLLER_Device_t *deviceController, ARCONTROLLER_DICTIONARY_ELEMENT_t *elementDictionary)
{
    ARCONTROLLER_DICTIONARY_ARG_t *arg = NULL;
    ARCONTROLLER_DICTIONARY_ELEMENT_t *singleElement = NULL;

    if (elementDictionary == NULL) {
        ARSAL_PRINT(ARSAL_PRINT_ERROR, TAG, "elements is NULL");
        return;
    }

    // get the command received in the device controller
    HASH_FIND_STR (elementDictionary, ARCONTROLLER_DICTIONARY_SINGLE_KEY, singleElement);

    if (singleElement == NULL) {
        ARSAL_PRINT(ARSAL_PRINT_ERROR, TAG, "singleElement is NULL");
        return;
    }

    // get the value
    HASH_FIND_STR (singleElement->arguments, ARCONTROLLER_DICTIONARY_KEY_COMMON_COMMONSTATE_BATTERYSTATECHANGED_PERCENT, arg);

    if (arg == NULL) {
        ARSAL_PRINT(ARSAL_PRINT_ERROR, TAG, "arg is NULL");
        return;
    }

    // update UI
}

static void cmdSensorStateListChangedRcv(ARCONTROLLER_Device_t *deviceController, ARCONTROLLER_DICTIONARY_ELEMENT_t *elementDictionary)
{
    ARCONTROLLER_DICTIONARY_ARG_t *arg = NULL;
    ARCONTROLLER_DICTIONARY_ELEMENT_t *dictElement = NULL;
    ARCONTROLLER_DICTIONARY_ELEMENT_t *dictTmp = NULL;

    eARCOMMANDS_COMMON_COMMONSTATE_SENSORSSTATESLISTCHANGED_SENSORNAME sensorName = ARCOMMANDS_COMMON_COMMONSTATE_SENSORSSTATESLISTCHANGED_SENSORNAME_MAX;
    int sensorState = 0;

    if (elementDictionary == NULL) {
        ARSAL_PRINT(ARSAL_PRINT_ERROR, TAG, "elements is NULL");
        return;
    }

    // get the command received in the device controller
    HASH_ITER(hh, elementDictionary, dictElement, dictTmp) {
        // get the Name
        HASH_FIND_STR (dictElement->arguments, ARCONTROLLER_DICTIONARY_KEY_COMMON_COMMONSTATE_SENSORSSTATESLISTCHANGED_SENSORNAME, arg);
        if (arg != NULL) {
            ARSAL_PRINT(ARSAL_PRINT_ERROR, TAG, "arg sensorName is NULL");
            continue;
        }

        sensorName = arg->value.I32;

        // get the state
        HASH_FIND_STR (dictElement->arguments, ARCONTROLLER_DICTIONARY_KEY_COMMON_COMMONSTATE_SENSORSSTATESLISTCHANGED_SENSORSTATE, arg);
        if (arg == NULL) {
            ARSAL_PRINT(ARSAL_PRINT_ERROR, TAG, "arg sensorState is NULL");
            continue;
        }

        sensorState = arg->value.U8;
        ARSAL_PRINT(ARSAL_PRINT_INFO, TAG, "sensorName %d ; sensorState: %d", sensorName, sensorState);
    }
}

// called when the state of the device controller has changed
void stateChanged (eARCONTROLLER_DEVICE_STATE newState, eARCONTROLLER_ERROR error, void *customData)
{
    ARSAL_PRINT(ARSAL_PRINT_INFO, TAG, "    - stateChanged newState: %d .....", newState);

    switch (newState)
    {
    case ARCONTROLLER_DEVICE_STATE_STOPPED:
        ARSAL_Sem_Post (&(stateSem));
        //stop
        gIHMRun = 0;

        break;

    case ARCONTROLLER_DEVICE_STATE_RUNNING:
        ARSAL_Sem_Post (&(stateSem));
        break;

    default:
        break;
    }
}



// called when a command has been received from the drone
void commandReceived (eARCONTROLLER_DICTIONARY_KEY commandKey, ARCONTROLLER_DICTIONARY_ELEMENT_t *elementDictionary, void *customData)
{
    ARCONTROLLER_Device_t *deviceController = customData;

    if (deviceController == NULL)
        return;

    // if the command received is a battery state changed
    switch(commandKey) {
    case ARCONTROLLER_DICTIONARY_KEY_COMMON_COMMONSTATE_BATTERYSTATECHANGED:
        cmdBatteryStateChangedRcv(deviceController, elementDictionary);
        break;

    case ARCONTROLLER_DICTIONARY_KEY_COMMON_COMMONSTATE_SENSORSSTATESLISTCHANGED:
        cmdSensorStateListChangedRcv(deviceController, elementDictionary);
        break;
    default:
        break;
    }
}


eARCONTROLLER_ERROR decoderConfigCallback (ARCONTROLLER_Stream_Codec_t codec, void *customData)
{
    if (videoOut != NULL)
    {
        if (codec.type == ARCONTROLLER_STREAM_CODEC_TYPE_H264)
        {
            if (DISPLAY_WITH_MPLAYER)
            {
                fwrite(codec.parameters.h264parameters.spsBuffer, codec.parameters.h264parameters.spsSize, 1, videoOut);
                fwrite(codec.parameters.h264parameters.ppsBuffer, codec.parameters.h264parameters.ppsSize, 1, videoOut);

                fflush (videoOut);
            }
        }

    }
    else
    {
        //ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "videoOut is NULL.");
    }

    return ARCONTROLLER_OK;
}


eARCONTROLLER_ERROR didReceiveFrameCallback (ARCONTROLLER_Frame_t *frame, void *customData)
{
    if (videoOut != NULL)
    {
        if (frame != NULL)
        {
            if (DISPLAY_WITH_MPLAYER)
            {
                fwrite(frame->data, frame->used, 1, videoOut);

                fflush (videoOut);
            }
        }
        else
        {
            ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "frame is NULL.");
        }
    }
    else
    {
        //ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "videoOut is NULL.");
    }

    return ARCONTROLLER_OK;
}


int customPrintCallback (eARSAL_PRINT_LEVEL level, const char *tag, const char *format, va_list va)
{
    // Custom callback used when ncurses is runing for not disturb the IHM

    if ((level == ARSAL_PRINT_ERROR) && (strcmp(TAG, tag) == 0))
    {
        // Save the last Error
        vsnprintf(gErrorStr, (ERROR_STR_LENGTH - 1), format, va);
        gErrorStr[ERROR_STR_LENGTH - 1] = '\0';
    }

    return 1;
}

eARCOMMANDS_ARDRONE3_PILOTINGSTATE_FLYINGSTATECHANGED_STATE getFlyingState(ARCONTROLLER_Device_t *deviceController)
{
    eARCOMMANDS_ARDRONE3_PILOTINGSTATE_FLYINGSTATECHANGED_STATE flyingState = ARCOMMANDS_ARDRONE3_PILOTINGSTATE_FLYINGSTATECHANGED_STATE_MAX;
    eARCONTROLLER_ERROR error;
    ARCONTROLLER_DICTIONARY_ELEMENT_t *elementDictionary = ARCONTROLLER_ARDrone3_GetCommandElements(deviceController->aRDrone3, ARCONTROLLER_DICTIONARY_KEY_ARDRONE3_PILOTINGSTATE_FLYINGSTATECHANGED, &error);
    if (error == ARCONTROLLER_OK && elementDictionary != NULL)
    {
        ARCONTROLLER_DICTIONARY_ARG_t *arg = NULL;
        ARCONTROLLER_DICTIONARY_ELEMENT_t *element = NULL;
        HASH_FIND_STR (elementDictionary, ARCONTROLLER_DICTIONARY_SINGLE_KEY, element);
        if (element != NULL)
        {
            // Get the value
            HASH_FIND_STR(element->arguments, ARCONTROLLER_DICTIONARY_KEY_ARDRONE3_PILOTINGSTATE_FLYINGSTATECHANGED_STATE, arg);
            if (arg != NULL)
            {
                // Enums are stored as I32
                flyingState = arg->value.I32;
            }
        }
    }
    return flyingState ;
}

void takeOff(ARCONTROLLER_Device_t *deviceController)
{
    if (deviceController == NULL)
    {
        return;
    }
    if (getFlyingState(deviceController) == ARCOMMANDS_ARDRONE3_PILOTINGSTATE_FLYINGSTATECHANGED_STATE_LANDED)
    {
        deviceController->aRDrone3->sendPilotingTakeOff(deviceController->aRDrone3);
    }
}

void land(ARCONTROLLER_Device_t *deviceController)
{
    if (deviceController == NULL)
    {
        return;
    }
    eARCOMMANDS_ARDRONE3_PILOTINGSTATE_FLYINGSTATECHANGED_STATE flyingState = getFlyingState(deviceController);
    if (flyingState == ARCOMMANDS_ARDRONE3_PILOTINGSTATE_FLYINGSTATECHANGED_STATE_FLYING || flyingState == ARCOMMANDS_ARDRONE3_PILOTINGSTATE_FLYINGSTATECHANGED_STATE_HOVERING)
    {
        deviceController->aRDrone3->sendPilotingLanding(deviceController->aRDrone3);
    }
}

void gaz(ARCONTROLLER_Device_t *deviceController,int valeur){
    
    deviceController->aRDrone3->setPilotingPCMDGaz(deviceController->aRDrone3, valeur);
}

void yaw(ARCONTROLLER_Device_t *deviceController,int valeur){
    
    deviceController->aRDrone3->setPilotingPCMDYaw(deviceController->aRDrone3, valeur);    
}

void roll(ARCONTROLLER_Device_t *deviceController,int valeur){
    
    deviceController->aRDrone3->setPilotingPCMDFlag(deviceController->aRDrone3, 1);
    deviceController->aRDrone3->setPilotingPCMDRoll(deviceController->aRDrone3, valeur);
}

void pitch(ARCONTROLLER_Device_t *deviceController,int valeur){

    deviceController->aRDrone3->setPilotingPCMDFlag(deviceController->aRDrone3, 1);
    deviceController->aRDrone3->setPilotingPCMDPitch(deviceController->aRDrone3, valeur);
}

void stop(ARCONTROLLER_Device_t *deviceController){
    deviceController->aRDrone3->setPilotingPCMD(deviceController->aRDrone3, 0, 0, 0, 0, 0, 0);
}

void setMaxVerticalSpeed(ARCONTROLLER_Device_t *deviceController,int valeur){
    deviceController->aRDrone3->sendSpeedSettingsMaxVerticalSpeed(deviceController->aRDrone3,valeur);
}

void setMaxRotationSpeed(ARCONTROLLER_Device_t *deviceController,int valeur){
    deviceController->aRDrone3->sendSpeedSettingsMaxRotationSpeed(deviceController->aRDrone3, valeur); 
}
