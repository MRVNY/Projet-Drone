#include "Pilotage.h"
#include "../commun.h"
#include <curses.h>

static void cmdBatteryStateChangedRcv(ARCONTROLLER_Device_t *deviceController, ARCONTROLLER_DICTIONARY_ELEMENT_t *elementDictionary);

static void cmdSensorStateListChangedRcv(ARCONTROLLER_Device_t *deviceController, ARCONTROLLER_DICTIONARY_ELEMENT_t *elementDictionary);


/*--------------Variable globales---------------*/
//Vars globl Parrot
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

//Vars globl watchdog 
#define TIMEOUT = 1000000;
struct timeval counter, watch;
pid_t child = 0;
pthread_t threads;

/*****************************************
 *
 *             implementation :
 *
 *****************************************/

static void signal_handler(int signal)
{
    gIHMRun = 0;
}

void *watch_dog(){
    while(1){
        usleep(125000);
        if(counter.tv_sec!=0){
            gettimeofday(&watch, NULL);
            printf("watch: %lus %lums, counter: %lus %lums, diff: %lums\n",watch.tv_sec,watch.tv_usec, counter.tv_sec,counter.tv_usec, (watch.tv_sec - counter.tv_sec)*1000000+ watch.tv_usec - counter.tv_usec); 
            if(((watch.tv_sec - counter.tv_sec) * 1000000 + watch.tv_usec - counter.tv_usec)>TIMEOUT){
                printf("WATCHDOG\n");
                endProg();
                break;
            }
        }
    }
    return 0;
}

void catchSig(){
    printf("CAUGHT\n");
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

    
    
    // Watch Dog
    //pthread_create(&threads, NULL, watch_dog, NULL);

   // catch signaux
    int i;
    for(i = 1; i <=SIGRTMIN ; i++){
        if(i != SIGTSTP) signal(i,catchSig);
    }

    pthread_create(&threads, NULL, watch_dog, NULL);

    /*-----Test du bouchon sans drone ni simu (affichages)-------*/
    /*printf("Début du test\n");
    (*functionPtr)("/home/johan/Parrot/packages/Samples/Unix/Projet-Drone-b/Data/Coords/coord1.txt");
    */
    // MPLAYER ou FFMPEG
   
    printf("\nrien (0), mplayer(1) ou ffmpeg(2)?\n");
    if(scanf("%d",&choice)==0 || (choice!=2 && choice!=1 && choice!=0)){
        printf("Entree non connue, mplayer par defaut\n");
        choice = 1;
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
        printf("rien\n");
        sleep(1);
    } 


    /* Set signal handlers */
    struct sigaction sig_action = {
        .sa_handler = signal_handler,
    };

    int ret = sigaction(SIGINT, &sig_action, NULL);
    if (ret < 0)
    {
        ARSAL_PRINT(ARSAL_PRINT_ERROR, "ERROR", "Unable to set SIGINT handler : %d(%s)",
                    errno, strerror(errno));
        return 1;
    }
    ret = sigaction(SIGPIPE, &sig_action, NULL);
    if (ret < 0)
    {
        ARSAL_PRINT(ARSAL_PRINT_ERROR, "ERROR", "Unable to set SIGPIPE handler : %d(%s)",
                    errno, strerror(errno));
        return 1;
    }

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
        if (DISPLAY_WITH_MPLAYER && choice!=0)
        {
            // fork the process to launch mplayer
            if ((child = fork()) == 0)
            {
                if(choice==2){
                    char str[5];
                    sprintf(str,"%d",fps);
                    execlp("ffmpeg", "ffmpeg", "-f", "h264", "-i", fifo_name, "-vf", "scale=-1:720", "-r",str, "outputs/%04d.jpeg", NULL);
                }
                else{
                    execlp("xterm", "xterm", "-e", "mplayer", "-demuxer",  "h264es", fifo_name, "-benchmark", "-really-quiet", NULL);
                    ARSAL_PRINT(ARSAL_PRINT_ERROR, TAG, "Missing mplayer, you will not see the video. Please install mplayer and xterm.");
                }
                return -1;
            }
        }

        if (DISPLAY_WITH_MPLAYER && choice!=0)
        {
            videoOut = fopen(fifo_name, "w");
        }
    }


    

/*****************************************
 *
 *             Connection au drone :
 *
 *****************************************/
// create a discovery device
    if (!failed)
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

/*****************************************
 *
 *             Création de l'interface
 *             de controle du :
 *
 *****************************************/
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
        
        takeOff(deviceController);
        sleep(5);

        //Appel de la partie imagerie avec la référence au flux vidéo (ici bouchon: tableau de coordonées)
        printf("Début du test\n");
        (*functionPtr)("/home/johan/Parrot/packages/Samples/Unix/Projet-Drone-b/Data/Coords/coord1.txt");        sleep(5);
        
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

void callback(int **state,int ifStop){
    //printf("callback\n");
    //Arrêt de la commande en cour
    stop(deviceController);

    //Erreur dans les traitements précédents, mise en sécurité de l'appareil 
    if(ifStop==STOP){
        //Gerer d'autre signaux pour les autres parties ?
        printf("Stop");
        endProg();
        return;
    }

    if(state){
        //Parcour des différents mouvements
        for(int i=STRAFER; i<=STRAFER; i++) {

            //Test de l'évaluation
            if(state[i][EVALUATION]==GOOD){

                int angleAmp;
                int speedAmp;
                int sign=state[i][EVALUATION]/abs(state[i][EVALUATION]);

                //Switch sur la position
                switch (abs(state[i][POS_INTENSITE]))
                {
                case AXE:
                    //printf("Centré\n");
                    angleAmp=0;
                    speedAmp=0;
                    break;
                case CLOSE:
                    //printf("Près\n");
                    angleAmp=LOW_ANGLE;
                    speedAmp=LOW_SPEED;
                    break;
                case FAR:
                    //printf("Loin\n");
                    angleAmp=MID_ANGLE;
                    speedAmp=MID_SPEED;
                    break;
                case EXTREME:
                    //printf("Très Loin\n");
                    angleAmp=HIGH_ANGLE;
                    speedAmp=HIGH_SPEED;
                    break;
                default:
                    break;
                }

                speedAmp=speedAmp*sign;
                angleAmp=angleAmp*sign;

                //Switch sur le mouvement
                switch (i)
                {
                case STRAFER:
                    //printf("Straffer\n");
                    roll(deviceController,-angleAmp);
                    break;
                case AVANT_ARRIERE:
                    pitch(deviceController,angleAmp);
                    break;
                case MONTER_DESCENDRE:
                    gaz(deviceController,speedAmp);
                    break;
                case ROTATION:
                    yaw(deviceController,speedAmp);
                    break;
                default:
                    stop(deviceController);
                    break;
                }
            }
        }
    }
    gettimeofday(&counter, NULL);
}



void endProg(){
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

        if (DISPLAY_WITH_MPLAYER && choice!=0)
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
            if (DISPLAY_WITH_MPLAYER && choice!=0)
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
            if (DISPLAY_WITH_MPLAYER && choice!=0)
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
