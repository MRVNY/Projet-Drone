ifeq ("$(TARGET_OS_FLAVOUR)","native")

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_CATEGORY_PATH := samples
LOCAL_MODULE := Projet-Drone
LOCAL_DESCRIPTION := Projet-Drone

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
	

LOCAL_SRC_FILES := \
	$(call all-c-files-under,.)\
	$(call all-c-files-under,../Décision)\
	$(call all-c-files-under,../Image)

include $(BUILD_EXECUTABLE)

endif
