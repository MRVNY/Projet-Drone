ifeq ("$(TARGET_OS_FLAVOUR)","native")

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := ncurses
LOCAL_EXPORT_LDLIBS := -lncurses
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := ffmpegs
LOCAL_EXPORT_C_INCLUDES := $(shell pkg-config --cflags libavcodec libavformat libswscale libavutil libavfilter)
LOCAL_EXPORT_LDLIBS := $(shell pkg-config --libs libavcodec libavformat libswscale libavutil libavfilter)
include $(BUILD_PREBUILT)

include $(CLEAR_VARS) 
LOCAL_MODULE := opencv2
LOCAL_EXPORT_C_INCLUDES := $(shell pkg-config --cflags opencv4)
LOCAL_EXPORT_LDLIBS := $(shell pkg-config --libs opencv4)
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)

LOCAL_CATEGORY_PATH := samples
LOCAL_MODULE := Projet-Drone-b
LOCAL_DESCRIPTION := Projet-Drone-b

LOCAL_LIBRARIES := \
	libARSAL \
	libARController \
	libARDataTransfer \
	libARUtils \
	libARCommands \
	libARNetwork \
	libARNetworkAL \
	libARDiscovery \
	libARStream \
	libARStream2 \
	ncurses\
	ffmpegs\
	opencv2

LOCAL_SRC_FILES := \
	$(call all-cpp-files-under,./Pilotage)\
	$(call all-c-files-under,./Pilotage)\
	$(call all-cpp-files-under,./Bas_niveau)\
	$(call all-c-files-under,./Decision)\
	$(call all-c-files-under,./DecodeManager)\


	
include $(BUILD_EXECUTABLE)

endif

