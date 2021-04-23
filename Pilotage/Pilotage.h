
#ifndef _PILOTAGE_H_
#define _PILOTAGE_H_

#define TAG "Projet-Drone"

/*****************************************
 *
 *             Constantes :
 *
 *****************************************/

/*--------------PARROT------------------*/
#define ERROR_STR_LENGTH 2048
#define BEBOP_IP_ADDRESS "192.168.42.1"
#define BEBOP_DISCOVERY_PORT 44444
#define DISPLAY_WITH_MPLAYER 1
#define FIFO_DIR_PATTERN "/tmp/arsdk_XXXXXX"
#define FIFO_NAME "arsdk_fifo"
/*---------------------------------------*/

/*---Convention sur les amplitudes de déplacement---*/
#define LOW_PITCH_ANGLE 10
#define MID_PITCH_ANGLE 20  
#define HIGH_PITCH_ANGLE 30
                            //% d'Angle max (Roll et Pitch)
#define LOW_ROLL_ANGLE 10
#define MID_ROLL_ANGLE 20  
#define HIGH_ROLL_ANGLE 30

#define HIGH_GAZ_SPEED 10
#define MID_GAZ_SPEED 20 
#define LOW_GAZ_SPEED 30
                            //% de vitesse max (Gaz et rotation)
#define HIGH_ROT_SPEED 10
#define MID_ROT_SPEED 20 
#define LOW_ROT_SPEED 30

/*--------------------------------------------------*/



/*****************************************
 *
 *             Librairies :
 *
 *****************************************/

/*--------------PARROT---------------------*/
#include <libARSAL/ARSAL.h>
#include <libARController/ARController.h>
#include <libARDiscovery/ARDiscovery.h>

/*------------------------------------------*/

/*--------------UTILITAIRES---------------------*/
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <pthread.h>
#include <time.h>
#include <signal.h>
#include <math.h>
/*------------------------------------------*/

/*****************************************
 *
 *             Main et Interface C++ :
 *
 *****************************************/
#if defined (__cplusplus)
extern "C" {
#endif
//Fonction appelée depuis main.cpp
/*
functionPtr -> pointeur sur la fonction vidéo_reader_process en C++ de Bas_niveau
*/
int main_Pilotage (int (*functionPtr)(const char*));
 
#if defined (__cplusplus)
}
#endif

/*****************************************
 *
 *             Methodes :
 *
 *****************************************/
/*
PARAMETRES:

ARCONTROLLER_Device_t *deviceController -> interface de communication avec le drone, cet objet représente le "drone"
et les instruction on passée a travers celui-ci.

int valeur ->  Entier représentant le % de vitesse/angle max de la commande de déplacement assocsiée
*/

/*-----------------Commande déplacements ----------------------*/
void takeOff(ARCONTROLLER_Device_t *deviceController);

void land(ARCONTROLLER_Device_t *deviceController);

void gaz(ARCONTROLLER_Device_t *deviceController,int valeur);

void yaw(ARCONTROLLER_Device_t *deviceController,int valeur);

void roll(ARCONTROLLER_Device_t *deviceController,int valeur);

void pitch(ARCONTROLLER_Device_t *deviceController,int valeur);

void stop(ARCONTROLLER_Device_t *deviceController);
/*--------------------------------------------------------------*/

/*-----------Gestion de la liaison avec le drone---------*/
void endProg();
void discoverDevice(int *failed,int isBebop2);
/*-------------------------------------------------------*/


/*----------Setter des vitesses de rotation/translation max--------------*/
void setMaxVerticalSpeed(ARCONTROLLER_Device_t *deviceController,int valeur);

void setMaxRotationSpeed(ARCONTROLLER_Device_t *deviceController,int valeur);

int choixPourcentage(int pos_intensite, int type);
/*-------------------------------------------------------------------------*/

/*----------WATCHDOG et SIGNAUX--------------*/
#define TIMEOUT 1000000
#define CYCLE 125000
void *watch_dog();
void catchSig();
void myPrint(char *toPrint);
/*---------------------------------*/

/*------------------------------------------------PARROT-----------------------------------------------*/

void stateChanged (eARCONTROLLER_DEVICE_STATE newState, eARCONTROLLER_ERROR error, void *customData);

void commandReceived (eARCONTROLLER_DICTIONARY_KEY commandKey, ARCONTROLLER_DICTIONARY_ELEMENT_t *elementDictionary, void *customData);

void batteryStateChanged (uint8_t percent);

eARCONTROLLER_ERROR didReceiveFrameCallback (ARCONTROLLER_Frame_t *frame, void *customData);

eARCONTROLLER_ERROR decoderConfigCallback (ARCONTROLLER_Stream_Codec_t codec, void *customData);

int customPrintCallback (eARSAL_PRINT_LEVEL level, const char *tag, const char *format, va_list va);

eARCOMMANDS_ARDRONE3_PILOTINGSTATE_FLYINGSTATECHANGED_STATE getFlyingState(ARCONTROLLER_Device_t *deviceController);

/*--------------------------------------------------------------------------------------------------------*/


#endif 
