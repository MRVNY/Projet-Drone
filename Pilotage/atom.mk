ifeq ("$(TARGET_OS_FLAVOUR)","native")

LOCAL_PATH := $(call \Proto)

include $(CLEAR_VARS)

LOCAL_CATEGORY_PATH := samples
LOCAL_MODULE := Proto_commun
LOCAL_DESCRIPTION := Proto_commun

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
	ncurses

LOCAL_SRC_FILES := \
	$(call all-c-files-under,.)

include $(BUILD_EXECUTABLE)

endif
