LOCAL_PATH := $(call my-dir)

ANJAROOTNATIVE_LOGTAG := AnJaRootNative
ANJAROOTINSTALLER_LOGTAG := AnJaRootInstaller


include $(CLEAR_VARS)
LOCAL_MODULE := anjaroot
LOCAL_SRC_FILES :=	lib/wrapper.cpp \
					lib/exceptions.cpp \
					lib/hook.cpp \
					lib/packages.cpp \
					lib/helper.cpp \
					shared/util.cpp \
					shared/version.cpp
LOCAL_LDLIBS := -llog -ldl
LOCAL_CPP_FEATURES := exceptions
LOCAL_CPPFLAGS := -DANJAROOT_LOGTAG="\"$(ANJAROOTNATIVE_LOGTAG)\"" \
				  -std=c++0x -Wall
include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := minizip
LOCAL_SRC_FILES := minizip/ioapi.c \
				   minizip/unzip.c \
				   minizip/zip.c
include $(BUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := anjarootinstaller
LOCAL_SRC_FILES := installer/installer.cpp \
				   installer/operations.cpp \
				   installer/hash.cpp \
				   installer/modes.cpp \
				   installer/mark.cpp \
				   installer/config.cpp \
				   installer/compression.cpp \
				   shared/util.cpp \
				   shared/version.cpp
LOCAL_LDLIBS := -llog -ldl -lz
LOCAL_STATIC_LIBRARIES := minizip
LOCAL_CPP_FEATURES := exceptions
LOCAL_CPPFLAGS := -DANJAROOT_LOGTAG="\"$(ANJAROOTINSTALLER_LOGTAG)\"" \
				  -std=c++0x -Wall
include $(BUILD_EXECUTABLE)
