
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
//#define BEBOP_IP_ADDRESS "10.202.0.1"
#define BEBOP_DISCOVERY_PORT 44444
#define FIFO_DIR_PATTERN "/tmp/arsdk_XXXXXX"
#define FIFO_NAME "arsdk_fifo"
/*---------------------------------------*/

/*---Convention sur les amplitudes de déplacement---*/
#define LOW_PITCH_ANGLE 3
#define MID_PITCH_ANGLE 7
#define HIGH_PITCH_ANGLE 10
                            //% d'Angle max (Roll et Pitch)
#define LOW_ROLL_ANGLE 3
#define MID_ROLL_ANGLE 7 
#define HIGH_ROLL_ANGLE 10
                            //GAZ= MONTER/DESCENDRE , ROLL=GAUCHE/DROITE , PITCH=AVANT/ARRIERE , ROT=ROTATION
#define HIGH_GAZ_SPEED 10
#define MID_GAZ_SPEED 20 
#define LOW_GAZ_SPEED 30
                            //% de vitesse max (Gaz et rotation)
#define HIGH_ROT_SPEED 10
#define MID_ROT_SPEED 20 
#define LOW_ROT_SPEED 30
/*--------------------------------------------------*/

/*---Watchdog et signaux---*/
#define IFPRINT 1
#define TIMEOUT 2000000
#define CYCLE 83000
/*-------------------------*/



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
int main_Pilotage (void * (*functionPtr)(const char*));
 
#if defined (__cplusplus)
}
#endif

/*****************************************
 *
 *             Methodes :
 *
 *****************************************/

/*-----------------Commande déplacements ----------------------*/
/*
PARAMETRES:

ARCONTROLLER_Device_t *deviceController -> interface de communication avec le drone, cet objet représente le "drone"
et les instruction on passée a travers celui-ci.

int valeur ->  Entier représentant le % de vitesse/angle max de la commande de déplacement assocsiée
*/

void takeOff();

void land();

void gaz(int valeur); //UP-DOWN

void yaw(int valeur); //ROTATION

void roll(int valeur); //LEFT-RIGHT

void pitch(int valeur); //FRONT-BACK

void stop();
/*--------------------------------------------------------------*/

/*---------------------UI------------------*/
void choiceParams(); //Selection des paramètre du programme (alumage des moteur/affichage caméra)
/*-----------------------------------------*/

/*-----------Gestion de la liaison avec le drone---------*/
void endProg(); // Suite d'instructions (arrêt, atterrissage, déconection ...) terminant le programme
void discoverDevice(int *failed); //Connection au dronen (Parrot)
void controlDevice(int *failed); //Création de l'interface de control du drone (Parrot)
/*-------------------------------------------------------*/


/*----------Setter des vitesses de rotation/translation--------------*/
void setMaxVerticalSpeed(int valeur);
void setMaxRotationSpeed(int valeur);

//int choixPourcentage(int pos_intensite, int type); //Définit selon le type de déplacement l'intensité a assoscié
/* PARAMETRES:  int pos_intensité -> distance a la mire (cf commun.h)
                int type -> type de déplacement (cf commun.h)
/*-------------------------------------------------------------------------*/

/*----------WATCHDOG et SIGNAUX--------------*/
void watchdog(); // Lancer watchdog chaque CYCLE et verifier si le decalage entre le counter et le temps reel est < TIMEOUT, sinon arreter le programme
void catchSig(); // Attraper tous les signaux sauf control-Z
void myPrint(char *toPrint); // Afficher les infos supplementaires (pour debug)
/*-------------------------------------------*/

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
