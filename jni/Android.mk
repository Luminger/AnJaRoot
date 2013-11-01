LOCAL_PATH := $(call my-dir)

ANJAROOTDAEMON_LOGTAG := AnJaRootDaemon
ANJAROOTNATIVE_LOGTAG := AnJaRootNative
ANJAROOTINSTALLER_LOGTAG := AnJaRootInstaller


include $(CLEAR_VARS)
LOCAL_MODULE := anjarootd
LOCAL_SRC_FILES := anjarootd/main.cpp \
				   anjarootd/trace.cpp \
				   anjarootd/anjarootdaemon.cpp \
				   anjarootd/packages.cpp \
				   anjarootd/hook.cpp \
				   anjarootd/arch-$(TARGET_ARCH)/hook.cpp \
				   shared/util.cpp \
				   shared/version.cpp
LOCAL_LDLIBS := -llog
LOCAL_CPP_FEATURES := exceptions
LOCAL_CPPFLAGS := -DANJAROOT_LOGTAG="\"$(ANJAROOTDAEMON_LOGTAG)\"" \
				  -std=c++11 -Wall
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_MODULE := anjaroot
LOCAL_SRC_FILES :=	lib/wrapper.cpp \
					lib/exceptions.cpp \
					lib/helper.cpp \
					lib/syscallfix.cpp \
					lib/arch-$(TARGET_ARCH)/local_getresuid.S \
				   	lib/arch-$(TARGET_ARCH)/local_getresgid.S \
					shared/util.cpp \
					shared/version.cpp
LOCAL_LDLIBS := -llog -ldl
LOCAL_CPP_FEATURES := exceptions
LOCAL_CPPFLAGS := -DANJAROOT_LOGTAG="\"$(ANJAROOTNATIVE_LOGTAG)\"" \
				  -std=c++11 -Wall
include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := minizip
LOCAL_SRC_FILES := minizip/ioapi.c \
				   minizip/unzip.c \
				   minizip/zip.c
# clang doesn't like how zlib uses parentheses, disable the warning
LOCAL_CFLAGS := -Wno-parentheses-equality
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
				  -std=c++11 -Wall
include $(BUILD_EXECUTABLE)
