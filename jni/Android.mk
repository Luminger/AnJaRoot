LOCAL_PATH := $(call my-dir)

ANJAROOTDAEMON_LOGTAG := AnJaRootDaemon
ANJAROOTNATIVE_LOGTAG := AnJaRootNative
ANJAROOTINSTALLER_LOGTAG := AnJaRootInstaller


include $(CLEAR_VARS)
LOCAL_MODULE := anjarootd
LOCAL_SRC_FILES := anjarootd/main.cpp \
				   anjarootd/trace.cpp \
				   shared/util.cpp
LOCAL_LDLIBS := -llog
LOCAL_CPP_FEATURES := exceptions
LOCAL_CPPFLAGS := -DANJAROOT_LOGTAG="\"$(ANJAROOTDAEMON_LOGTAG)\"" \
				  -std=c++0x -Wall
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_MODULE := anjaroot
LOCAL_SRC_FILES :=	lib/wrapper.cpp \
					lib/exceptions.cpp \
					lib/hook.cpp \
					lib/packages.cpp \
					lib/helper.cpp \
					lib/syscallfix.cpp \
					shared/util.cpp \
					shared/version.cpp

ifeq ($(TARGET_ARCH),arm)
LOCAL_SRC_FILES += lib/arch-arm/local_getresuid.S \
				   lib/arch-arm/local_getresgid.S
endif

ifeq ($(TARGET_ARCH),x86)
LOCAL_SRC_FILES += lib/arch-x86/local_getresuid.S \
				   lib/arch-x86/local_getresgid.S
endif

ifeq ($(TARGET_ARCH),mips)
LOCAL_SRC_FILES += lib/arch-mips/local_getresuid.S \
				   lib/arch-mips/local_getresgid.S
endif

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
