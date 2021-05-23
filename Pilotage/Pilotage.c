#include "Pilotage.h"
#include "../commun.h"
#include <curses.h>

static void cmdBatteryStateChangedRcv(ARCONTROLLER_Device_t *deviceController, ARCONTROLLER_DICTIONARY_ELEMENT_t *elementDictionary);

static void cmdSensorStateListChangedRcv(ARCONTROLLER_Device_t *deviceController, ARCONTROLLER_DICTIONARY_ELEMENT_t *elementDictionary);


/*--------------Variable globales---------------*/
//Vars globales Parrot
int failed = 0;
static char fifo_dir[] = FIFO_DIR_PATTERN;
int choice;
char gErrorStr[ERROR_STR_LENGTH];
ARCONTROLLER_Device_t *deviceController = NULL;
ARSAL_Sem_t stateSem;
ARDISCOVERY_Device_t *device = NULL;
eARCONTROLLER_ERROR error = ARCONTROLLER_OK;
eARCONTROLLER_DEVICE_STATE deviceState = ARCONTROLLER_DEVICE_STATE_MAX;

//Vars globales watchdog 
struct timeval counter, watch;
pthread_t videoThread;
pthread_t dogThread;
char toPrint[100];

//Tableau 2D contenant les valeurs d'amplitudes de mouvements pour chaque déplacements 
int tabPrc[4][4]={
    {0, LOW_ROLL_ANGLE, MID_ROLL_ANGLE, HIGH_ROLL_ANGLE},
    {0, LOW_PITCH_ANGLE, MID_PITCH_ANGLE, HIGH_PITCH_ANGLE},
    {0, LOW_GAZ_SPEED, MID_GAZ_SPEED, HIGH_GAZ_SPEED},
    {0, LOW_ROT_SPEED, MID_ROT_SPEED, HIGH_ROT_SPEED}
};
/*-------------------------------------------------*/
int StateZero=0;
ARCONTROLLER_Stream_Codec_t H264codec;

/*****************************************
 *
 *             implementation :
 *
 *****************************************/

// Afficher les infos supplementaires, activer avec IFPRINT
void myPrint(char *toPrint){
    if(IFPRINT) printf("%s",toPrint);
}

void watchdog(){
    while(endProgState < ENDING){

        usleep(CYCLE); //Lancer watchdog chaque CYCLE secondes

        if(counter.tv_sec!=0){ //Commencer a verifier apres le counter a ete modifie

            gettimeofday(&watch, NULL); //Recuperer le temps reel

            if(((watch.tv_sec - counter.tv_sec) * 1000000 + watch.tv_usec - counter.tv_usec)>TIMEOUT){

                myPrint("WATCHDOG\n"); //S'il y a TIMEOUT secondes de decalage, endProg
                sprintf(toPrint,"watch: %lus %lums, counter: %lus %lums, diff: %lums\n",watch.tv_sec,watch.tv_usec, counter.tv_sec,counter.tv_usec, (watch.tv_sec - counter.tv_sec)*1000000+ watch.tv_usec - counter.tv_usec);
                myPrint(toPrint);
                
                if(endProgState < ENDING) endProg();
                break;
            }
        }
    }
}

// Attraper tous les signaux sauf control-Z
void catchSig(int sig){
    sprintf(toPrint,"CAUGHT %d\n",sig);
    myPrint(toPrint);

    if(endProgState < ENDING) endProg();
}

int main_Pilotage (void * (*functionPtr)(const char*))
{
    fifo_name[128] = "";
    videoOut = NULL;

    //Initialisation de enProg
    endProgState = RUNNING;
    start=0;
    // catch signaux
    int i;
    for(i = 1; i <=SIGRTMIN ; i++){
        if(i != SIGTSTP) signal(i,catchSig);
    }

    /*---------Choix paramètre programme------------*/
    choiceParams();
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
 
    if (!failed)
    {
        if(choice==0) pthread_create(&videoThread, NULL, functionPtr, fifo_name);
        videoOut = fopen(fifo_name, "w");
    }


/*****************************************
 *
 *       Connection au drone et création
 *          de l'interface de control
 *
 *****************************************/

discoverDevice();

controlDevice();

    
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
        
        if(fly) takeOff();
        myPrint("TAKEN OFF\n");

        //pthread_create(&dogThread, NULL, watchdog,NULL);
        start=1;
        watchdog();

        //pthread_join(dogThread, NULL);

        //join threads
        void * status;
        pthread_join(videoThread,&status);
    }
    
    
/*****************************************
 *
 *             Fin des instruction 
 *             de pilotage :
 *
 *****************************************/
    //while (!endProgState){}
    //myPrint("TO_END\n");
    
    if(endProgState < ENDING) endProg();
    return EXIT_SUCCESS;
}


/*Définitions des fonctions de pilotage*/

//compteur pour ignorer un certain nombre de frames
void callbackPilote(int index,int ifStop){
    
    if(!endProgState == ENDING) return;

    if(index==-1){
        stop();
        return;
    }

    int (*state)[4] = tab_Sestimatin[index].matrice;
    if (state==NULL){
        myPrint("Erreur matrice nulle\n");
        return;
    }


    if(deviceController != NULL && state != NULL){
        
        int i, sign;
        int composition[4] = {0,0,0,0}; //Tableau de la composition des mouvements
        int sum = 0;

        //Affichage de la matrice dedécision
        if(IFPRINT){
            printf("STATE:\n");
            for(i=0;i<4;i++)
                printf("[%d , %d]\n",state[i][0],state[i][1]);
            printf("\n");
        }
        
        //Arrêt de la commande en cours
        stop();

        //Erreur dans les traitements précédents, mise en sécurité de l'appareil 
        if(ifStop==STOP){
            myPrint("Stop\n");
            //MAJ de la partie décision, le ifstop==STOP ne termine pas le programme
            //endProg();
            return;
        }

        //Parcour des différents mouvements
        for(i=STRAFER; i<=AVANT_ARRIERE; i++) {
            composition[i] = 0;
            
            sum += abs(state[i][POS_INTENSITE]);
            //Test de l'évaluation
            if(state[i][EVALUATION]==GOOD/*||state[i][EVALUATION]==0*/){

                //On va définir l'amplitude de mouvement a appliquer pour chaque mvmts
                    if(state[i][POS_INTENSITE]!=0) 
                        sign = -state[i][POS_INTENSITE] / abs(state[i][POS_INTENSITE]);
                    else sign = 0;
                composition[i] = sign * tabPrc[i][abs(state[i][POS_INTENSITE])];
            }
        }

        //Condition attérissage et fin de programme
        if(sum==0){
            StateZero++;
            if(StateZero>20){
                land();
                endProgState = TO_END;
                return;
            }
        }
        else StateZero = 0;

        //On compose les mouvement que l'on envoie au drone
        roll(composition[STRAFER]);
        pitch(-composition[AVANT_ARRIERE]);
        /*gaz(composition[MONTER_DESCENDRE]);
        yaw(composition[ROTATION]);*/
        
        gettimeofday(&counter, NULL);
    }

}

void endProg(){
    endProgState = ENDING;
    myPrint("ENDING\n");

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

    }
    fflush (videoOut);
    fclose (videoOut);

    ARSAL_Sem_Destroy (&(stateSem));

    unlink(fifo_name);
    rmdir(fifo_dir);

    ARSAL_PRINT(ARSAL_PRINT_INFO, TAG, "-- END --");

    endProgState = ENDED;
    myPrint("ENDED\n");
}

void choiceParams(){
    /*
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
    }*/
    
    fly = 1;
    display = 0;
    choice = 0; //VideoCapture
}

void controlDevice(){
    if (!failed)
    {
        deviceController = ARCONTROLLER_Device_New (device, &error);

        if (error != ARCONTROLLER_OK)
        {
            ARSAL_PRINT (ARSAL_PRINT_ERROR, TAG, "Creation of deviceController failed.");
            failed = 1;
        }
       
    }

    if (!failed)
    {
        ARSAL_PRINT(ARSAL_PRINT_INFO, TAG, "- delete discovey device ... ");
        ARDISCOVERY_Device_Delete (&device);
    }

    // add the state change callback to be informed when the device controller starts, stops...
    if (!failed)
    {
        error = ARCONTROLLER_Device_AddStateChangedCallback (deviceController, stateChanged, deviceController);

        if (error != ARCONTROLLER_OK)
        {
            ARSAL_PRINT (ARSAL_PRINT_ERROR, TAG, "add State callback failed.");
            failed = 1;
        }
    }

    // add the command received callback to be informed when a command has been received from the device
    if (!failed)
    {
        error = ARCONTROLLER_Device_AddCommandReceivedCallback (deviceController, commandReceived, deviceController);

        if (error != ARCONTROLLER_OK)
        {
            ARSAL_PRINT (ARSAL_PRINT_ERROR, TAG, "add callback failed.");
            failed = 1;
        }
    }

    // add the frame received callback to be informed when a streaming frame has been received from the device
    if (!failed)
    {
        ARSAL_PRINT(ARSAL_PRINT_INFO, TAG, "- set Video callback ... ");
        error = ARCONTROLLER_Device_SetVideoStreamCallbacks (deviceController, decoderConfigCallback, didReceiveFrameCallback, NULL , NULL);

        if (error != ARCONTROLLER_OK)
        {
            failed = 1;
            ARSAL_PRINT(ARSAL_PRINT_ERROR, TAG, "- error: %s", ARCONTROLLER_Error_ToString(error));
        }
    }

    if (!failed)
    {
        ARSAL_PRINT(ARSAL_PRINT_INFO, TAG, "Connecting ...");
        error = ARCONTROLLER_Device_Start (deviceController);

        if (error != ARCONTROLLER_OK)
        {
            failed = 1;
            ARSAL_PRINT(ARSAL_PRINT_ERROR, TAG, "- error :%s", ARCONTROLLER_Error_ToString(error));
        }
    }

    if (!failed)
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
    if (!failed)
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

void discoverDevice(){
    if (!failed)
    {
        ARSAL_PRINT(ARSAL_PRINT_INFO, TAG, "- init discovey device ... ");
        eARDISCOVERY_ERROR errorDiscovery = ARDISCOVERY_OK;

        device = ARDISCOVERY_Device_New (&errorDiscovery);

        if (errorDiscovery == ARDISCOVERY_OK)
        {
            ARSAL_PRINT(ARSAL_PRINT_INFO, TAG, "    - ARDISCOVERY_Device_InitWifi ...");
            // create a Bebop drone discovery device (ARDISCOVERY_PRODUCT_ARDRONE)

            errorDiscovery = ARDISCOVERY_Device_InitWifi (device, ARDISCOVERY_PRODUCT_BEBOP_2, "bebop2", BEBOP_IP_ADDRESS, BEBOP_DISCOVERY_PORT);

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
    H264codec=codec;
    if (videoOut != NULL)
    {
        if (codec.type == ARCONTROLLER_STREAM_CODEC_TYPE_H264)
        {
            fwrite(codec.parameters.h264parameters.spsBuffer, codec.parameters.h264parameters.spsSize, 1, videoOut);
            fwrite(codec.parameters.h264parameters.ppsBuffer, codec.parameters.h264parameters.ppsSize, 1, videoOut);
            fflush (videoOut);
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
            fwrite(frame->data, frame->used, 1, videoOut);
            fflush (videoOut);
        }
        else
        {
            ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "frame is NULL.");
        }
    }
    else
    {
        ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "videoOut is NULL.");
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

void takeOff()
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

void land()
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

void gaz(int valeur){
    
    deviceController->aRDrone3->setPilotingPCMDGaz(deviceController->aRDrone3, valeur);
}

void yaw(int valeur){
    
    deviceController->aRDrone3->setPilotingPCMDYaw(deviceController->aRDrone3, valeur);    
}

void roll(int valeur){
    
    deviceController->aRDrone3->setPilotingPCMDFlag(deviceController->aRDrone3, 1);
    deviceController->aRDrone3->setPilotingPCMDRoll(deviceController->aRDrone3, valeur);
}

void pitch(int valeur){

    deviceController->aRDrone3->setPilotingPCMDFlag(deviceController->aRDrone3, 1);
    deviceController->aRDrone3->setPilotingPCMDPitch(deviceController->aRDrone3, valeur);
}

void stop(){
    deviceController->aRDrone3->setPilotingPCMD(deviceController->aRDrone3, 0, 0, 0, 0, 0, 0);
}

void setMaxVerticalSpeed(int valeur){
    deviceController->aRDrone3->sendSpeedSettingsMaxVerticalSpeed(deviceController->aRDrone3,valeur);
}

void setMaxRotationSpeed(int valeur){
    deviceController->aRDrone3->sendSpeedSettingsMaxRotationSpeed(deviceController->aRDrone3, valeur); 
}
