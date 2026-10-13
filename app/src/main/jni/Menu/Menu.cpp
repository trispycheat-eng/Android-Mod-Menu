#include <list>
#include <vector>
#include <string>
#include <pthread.h>
#include <thread>
#include <cstring>
#include <jni.h>
#include <unistd.h>
#include <fstream>
#include <iostream>
#include <dlfcn.h>
#include "Includes/Logger.h"
#include "Includes/obfuscate.h"
#include "Includes/Utils.hpp"
#include "Menu/Menu.hpp"
#include "Menu/Jni.hpp"
#include "Includes/Macros.h"

// ============================================================
//  TRISPY CHEATS · t.me/TrispyCheats
// ============================================================

bool aimlockHead = false;
bool ignoreKnocked = false;
bool visibleCheck = false;
bool espEnemy = false;
bool espGlow = false;
bool espLine = false;
bool espBox = false;
bool antiCheat = false;
int aimFov = 30;
int espRange = 200;
int aimSmooth = 5;
bool btnPressed = false;

// ============================================================
//  REQUIRED FRAMEWORK FUNCTIONS — exact signatures
// ============================================================
jobjectArray setText(JNIEnv *env, jobject obj, const char *text) {
    return nullptr;
}

jstring Icon(JNIEnv *env, jobject obj) {
    return env->NewStringUTF("");
}

jstring IconWebViewData(JNIEnv *env, jobject obj) {
    return env->NewStringUTF("");
}

jobjectArray SettingsList(JNIEnv *env, jobject obj) {
    return nullptr;
}

void RegisterMenu(JNIEnv *env, jobject context) {
}

// ============================================================
//  FEATURE LIST
// ============================================================
jobjectArray GetFeatureList(JNIEnv *env, jobject context) {
    jobjectArray ret;

    const char *features[] = {

        OBFUSCATE("RichWebView_html=<html><head><meta name='viewport' content='width=device-width,initial-scale=1'><style>@keyframes glow{0%{text-shadow:0 0 8px #ff2200,0 0 16px #ff4500;}50%{text-shadow:0 0 22px #ff6600,0 0 44px #ff4500;}100%{text-shadow:0 0 8px #ff2200,0 0 16px #ff4500;}}@keyframes flick{0%,100%{opacity:1;}47%{opacity:1;}50%{opacity:0.55;}53%{opacity:1;}}body{background:#0a0000;margin:0;padding:14px 8px;font-family:sans-serif;}.h{color:#ff4500;font-size:22px;font-weight:bold;text-align:center;animation:glow 1.8s infinite,flick 3.2s infinite;letter-spacing:3px;}.sub{color:#cc9966;font-size:10px;text-align:center;margin-top:5px;letter-spacing:2px;}.line{height:1px;background:linear-gradient(90deg,transparent,#ff4500,transparent);margin:8px 0;}</style></head><body><div class='h'>&#9733; TRISPY &#9733;</div><div class='line'></div><div class='sub'>by TrispyCheats</div><div class='sub'>t.me/TrispyCheats</div></body></html>"),

        OBFUSCATE("Category_AIM"),
        OBFUSCATE("Toggle_Aimlock Head"),
        OBFUSCATE("SeekBar_Aim Smooth_1_20"),
        OBFUSCATE("SeekBar_Aim FOV_1_360"),
        OBFUSCATE("Toggle_Ignore Knocked"),
        OBFUSCATE("Toggle_Visible Check Only"),

        OBFUSCATE("Category_VISUAL"),
        OBFUSCATE("Toggle_ESP Enemy"),
        OBFUSCATE("Toggle_ESP Glow"),
        OBFUSCATE("Toggle_ESP Line"),
        OBFUSCATE("Toggle_ESP Box"),
        OBFUSCATE("SeekBar_ESP Range_1_1000"),

        OBFUSCATE("Category_SYSTEM"),
        OBFUSCATE("Toggle_Anti-Cheat Bypass"),
        OBFUSCATE("Button_Clear Reports"),
        OBFUSCATE("Button_Wipe Cache"),
        OBFUSCATE("Button_STOP & WIPE"),

        OBFUSCATE("RichTextView_<div style='text-align:center;color:#cc9966;font-size:11px;letter-spacing:2px;'>v1.0 · t.me/TrispyCheats</div>"),
    };

    int Total_Feature = (sizeof features / sizeof features[0]);
    ret = (jobjectArray)
        env->NewObjectArray(Total_Feature, env->FindClass(OBFUSCATE("java/lang/String")),
                            env->NewStringUTF(""));
    for (int i = 0; i < Total_Feature; i++)
        env->SetObjectArrayElement(ret, i, env->NewStringUTF(features[i]));
    return ret;
}

// ============================================================
//  BUTTON HANDLER
// ============================================================
void Changes(JNIEnv *env, jclass clazz, jobject obj,
             jint featNum, jstring featName, jint value,
             jlong Lvalue, jboolean boolean, jstring jstr) {
    switch (featNum) {
        case 0: break;
        case 1: break;
        case 2: aimlockHead = boolean; break;
        case 3: aimSmooth = value; break;
        case 4: aimFov = value; break;
        case 5: ignoreKnocked = boolean; break;
        case 6: visibleCheck = boolean; break;
        case 7: break;
        case 8:  espEnemy = boolean; break;
        case 9:  espGlow = boolean; break;
        case 10: espLine = boolean; break;
        case 11: espBox = boolean; break;
        case 12: espRange = value; break;
        case 13: break;
        case 14: antiCheat = boolean; break;
        case 15: break;
        case 16: break;
        case 17: btnPressed = boolean; break;
        case 18: break;
        default: break;
    }
}

// ============================================================
//  HOOK THREAD
// ============================================================
void (*old_Update)(void *instance);
void Update(void *instance) {
    if (instance != nullptr) {
        if (btnPressed) btnPressed = false;
    }
    return old_Update(instance);
}

#define targetLibName OBFUSCATE("libil2cpp.so")
ElfScanner g_il2cppELF;

void *hack_thread(void *) {
    LOGI(OBFUSCATE("trispy hook thread started"));
    do { sleep(1); } while (!isLibraryLoaded(targetLibName));
    LOGI(OBFUSCATE("%s loaded"), (const char *) targetLibName);
#if defined(__aarch64__)
    uintptr_t il2cppBase = g_il2cppELF.base();
#endif
    LOGI(OBFUSCATE("trispy hook thread done"));
    return nullptr;
}

__attribute__((constructor))
void lib_main() {
    pthread_t ptid;
    pthread_create(&ptid, NULL, hack_thread, NULL);
}
