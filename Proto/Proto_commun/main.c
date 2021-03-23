
#include <stdlib.h>
#include <curses.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>

#include <libARSAL/ARSAL.h>
#include <libARController/ARController.h>
#include <libARDiscovery/ARDiscovery.h>
#include "Pilotage.h"



int main (int argc, char *argv[])

{
    // MPLAYER ou FFMPEG
    int choice;
    int fps;
    printf("\nmplayer(1) ou ffmpeg(2)?\n");
    if(scanf("%d",&choice)==0 || (choice!=2 && choice!=1)){
        printf("Entree non connue, mplayer par defaut\n");
        choice = 1;
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

    //Selection des variable de déplacement
    int temps; //ms
    int prcAngle; 
    int prcVitt;
    printf("Durée impulsion (ms)\n");
    scanf("%d",&temps);
    printf("Pourcentage angle max\n");
    scanf("%d",&prcAngle);
    printf("Pourcentage vitesse max\n");
    scanf("%d",&prcVitt);


    // local declarations
    int failed = 0;
    ARDISCOVERY_Device_t *device = NULL;
    ARCONTROLLER_Device_t *deviceController = NULL;
    eARCONTROLLER_ERROR error = ARCONTROLLER_OK;
    eARCONTROLLER_DEVICE_STATE deviceState = ARCONTROLLER_DEVICE_STATE_MAX;
    pid_t child = 0;

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

    ARSAL_PRINT(ARSAL_PRINT_INFO, TAG, "-- Bebop 2 Test TakeOff and Landing --");
    
 
    if (!failed)
    {
        if (DISPLAY_WITH_MPLAYER)
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

        if (DISPLAY_WITH_MPLAYER)
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
        while (getFlyingState(deviceController)!=ARCOMMANDS_ARDRONE3_PILOTINGSTATE_FLYINGSTATECHANGED_STATE_HOVERING)
        {   
            //On attend tant que le drone n'est pas en vol stationaire
        }
        sleep(1);
        
    
    char c;
    char bff;
    int it;
    //sleep(5);
    do
    {   
        deviceController->aRDrone3->setPilotingPCMD(deviceController->aRDrone3, 0, 0, 0, 0, 0, 0);
        printf("Commande a(avancer), r(reculer),g(gauche),d(droit),h(haut),b(bas),t(takeoff),l(land),q(rotgauche),s(rotdroit)\n");
        scanf("%c",&bff);
        scanf("%c",&c);

        if(c=='e'|| c=='l'|| c=='t'){
            it=1;
        }
        else{
            printf("Nombre itération\n");
            scanf("%d",&it);
        }
       
        
        for (int i = 0; i < it; i++)
        {   
            //Arrêt du déplacement
            deviceController->aRDrone3->setPilotingPCMD(deviceController->aRDrone3, 0, 0, 0, 0, 0, 0);

            switch (c)
            {
            case 'a':
                deviceController->aRDrone3->setPilotingPCMDFlag(deviceController->aRDrone3, 1);
                deviceController->aRDrone3->setPilotingPCMDPitch(deviceController->aRDrone3, prcAngle);
                break;
            case 'r':
                deviceController->aRDrone3->setPilotingPCMDFlag(deviceController->aRDrone3, 1);
                deviceController->aRDrone3->setPilotingPCMDPitch(deviceController->aRDrone3, -prcAngle);                
                break;
            case 'g':
                deviceController->aRDrone3->setPilotingPCMDFlag(deviceController->aRDrone3, 1);
                deviceController->aRDrone3->setPilotingPCMDRoll(deviceController->aRDrone3, -prcAngle);                
                break;
            case 'd':
                deviceController->aRDrone3->setPilotingPCMDFlag(deviceController->aRDrone3, 1);
                deviceController->aRDrone3->setPilotingPCMDRoll(deviceController->aRDrone3, prcAngle);                    
                break;
            case 'h':
                deviceController->aRDrone3->setPilotingPCMDGaz(deviceController->aRDrone3, prcVitt);
                break;
            case 'b':
                deviceController->aRDrone3->setPilotingPCMDGaz(deviceController->aRDrone3, -prcVitt);
            case 'q':
                deviceController->aRDrone3->setPilotingPCMDYaw(deviceController->aRDrone3, -prcVitt);
                break;
            case 's':
                deviceController->aRDrone3->setPilotingPCMDYaw(deviceController->aRDrone3, prcVitt);
                break;
            case 'l':
                land(deviceController);
                break;
            case 't':
                takeOff(deviceController);
                break;
            
            default:
            deviceController->aRDrone3->setPilotingPCMD(deviceController->aRDrone3, 0, 0, 0, 0, 0, 0);

                break;
            }
            //Temps du reste du traitement 
            usleep(1000*temps);
        }
        

    } while (c!='e');
    
    sleep(2);
    land(deviceController);   
    
    
    

        /*PITCH / ROLL
        1/24s/10 -> 17 cm/s
        1/24s/30 -> 60 cm/s
        1/24s/50 -> 1 m/s
        */
       

             
    }
    
    
/*****************************************
 *
 *             Fin des instruction 
 *             de pilotage :
 *
 *****************************************/

// we are here because of a disconnection or user has quit IHM, so safely delete everything
    if (deviceController != NULL)
    {


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

    return EXIT_SUCCESS;
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