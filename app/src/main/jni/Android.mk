LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := dobby
LOCAL_SRC_FILES := Dobby/libraries/$(TARGET_ARCH_ABI)/libdobby.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := Keystone
LOCAL_SRC_FILES := KittyMemory/Deps/Keystone/libs-android/$(TARGET_ARCH_ABI)/libkeystone.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := crypto

LOCAL_CFLAGS := -w -s -Wno-error=format-security -fvisibility=hidden -fpermissive -fexceptions
LOCAL_CPPFLAGS := -w -s -Wno-error=format-security -fvisibility=hidden -Werror -std=c++17
LOCAL_CPPFLAGS += -Wno-error=c++1z-narrowing -fpermissive -Wall -fexceptions
LOCAL_LDFLAGS := -Wl,--gc-sections -Wl,--strip-all -llog
LOCAL_LDLIBS := -llog -landroid -lEGL -lGLESv2 -ldl

LOCAL_ARM_MODE := arm
LOCAL_C_INCLUDES += $(LOCAL_PATH) $(LOCAL_PATH)/Includes/

LOCAL_SRC_FILES := Main.cpp \
    Menu/Jni.cpp \
    Menu/Menu.cpp \
    Menu/Setup.cpp \
    Includes/Utils.cpp \
    AntiDebug.cpp \
    UltimateBypass.cpp \
    MemoryScrubber.cpp \
    KittyMemory/KittyArm64.cpp \
    KittyMemory/KittyScanner.cpp \
    KittyMemory/KittyMemory.cpp \
    KittyMemory/KittyUtils.cpp \
    KittyMemory/KittyMemoryPatch.cpp \
    KittyMemory/MemoryPatch.cpp \
    KittyMemory/MemoryBackup.cpp

LOCAL_STATIC_LIBRARIES := Keystone dobby
include $(BUILD_SHARED_LIBRARY)
