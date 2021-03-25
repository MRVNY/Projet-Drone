#ifndef _PILOTAGE_H_
#define _PILOTAGE_H_

#define TAG "Protocom"

#define ERROR_STR_LENGTH 2048

#define BEBOP_IP_ADDRESS "192.168.42.1"
#define BEBOP_DISCOVERY_PORT 44444

#define DISPLAY_WITH_MPLAYER 1

#define FIFO_DIR_PATTERN "/tmp/arsdk_XXXXXX"
#define FIFO_NAME "arsdk_fifo"

#define LOW_ANGLE 10
#define HIGH_ANGLE 30

#define HIGH_SPEED 10
#define LOW_SPEED 30

static char fifo_dir[] = FIFO_DIR_PATTERN;
static char fifo_name[128] = "";

int gIHMRun = 1;
char gErrorStr[ERROR_STR_LENGTH];
//IHM_t *ihm = NULL;

FILE *videoOut = NULL;
int frameNb = 0;
ARSAL_Sem_t stateSem;
int isBebop2 = 1;


void stateChanged (eARCONTROLLER_DEVICE_STATE newState, eARCONTROLLER_ERROR error, void *customData);

void commandReceived (eARCONTROLLER_DICTIONARY_KEY commandKey, ARCONTROLLER_DICTIONARY_ELEMENT_t *elementDictionary, void *customData);

void batteryStateChanged (uint8_t percent);

eARCONTROLLER_ERROR didReceiveFrameCallback (ARCONTROLLER_Frame_t *frame, void *customData);

eARCONTROLLER_ERROR decoderConfigCallback (ARCONTROLLER_Stream_Codec_t codec, void *customData);

int customPrintCallback (eARSAL_PRINT_LEVEL level, const char *tag, const char *format, va_list va);

eARCOMMANDS_ARDRONE3_PILOTINGSTATE_FLYINGSTATECHANGED_STATE getFlyingState(ARCONTROLLER_Device_t *deviceController);

void takeOff(ARCONTROLLER_Device_t *deviceController);

void land(ARCONTROLLER_Device_t *deviceController);

void gaz(ARCONTROLLER_Device_t *deviceController,int valeur);

void yaw(ARCONTROLLER_Device_t *deviceController,int valeur);

void roll(ARCONTROLLER_Device_t *deviceController,int valeur);

void pitch(ARCONTROLLER_Device_t *deviceController,int valeur);

void stop(ARCONTROLLER_Device_t *deviceController);

void setMaxVerticalSpeed(ARCONTROLLER_Device_t *deviceController,int valeur);

void setMaxRotationSpeed(ARCONTROLLER_Device_t *deviceController,int valeur);

#endif 
