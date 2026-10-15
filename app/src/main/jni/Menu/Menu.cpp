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
#include <signal.h>
#include <sys/types.h>
#include "Includes/Logger.h"
#include "Includes/obfuscate.h"
#include "Includes/Utils.hpp"
#include "Menu/Menu.hpp"
#include "Menu/Jni.hpp"
#include "Includes/Macros.h"

// ============================================================
// TRISPY CHEATS
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
bool matchMode = false;

extern void applyPatches();

// ============================================================
// ACTION FUNCTIONS
// ============================================================
static void doClearReports() {
    LOGI(OBFUSCATE("clearing reports..."));
    system("rm -rf /data/data/com.dts.freefireth/files/report* 2>/dev/null");
    system("rm -rf /data/data/com.dts.freefireth/files/crash* 2>/dev/null");
    system("rm -rf /data/data/com.dts.freefiremax/files/report* 2>/dev/null");
    system("rm -rf /data/data/com.dts.freefiremax/files/crash* 2>/dev/null");
    system("rm -rf /sdcard/Android/data/com.dts.freefireth/files/report* 2>/dev/null");
    system("rm -rf /sdcard/Android/data/com.dts.freefiremax/files/report* 2>/dev/null");
    LOGI(OBFUSCATE("reports cleared"));
}

static void doWipeCache() {
    LOGI(OBFUSCATE("wiping cache..."));
    system("rm -rf /data/data/com.dts.freefireth/cache/* 2>/dev/null");
    system("rm -rf /data/data/com.dts.freefiremax/cache/* 2>/dev/null");
    system("rm -rf /sdcard/Android/data/com.dts.freefireth/cache/* 2>/dev/null");
    system("rm -rf /sdcard/Android/data/com.dts.freefiremax/cache/* 2>/dev/null");
    LOGI(OBFUSCATE("cache wiped"));
}

static void doStopAndWipe() {
    LOGI(OBFUSCATE("stop & wipe"));
    doWipeCache();
    doClearReports();
    kill(getpid(), SIGKILL);
}

// ============================================================
// REQUIRED FRAMEWORK FUNCTIONS
// ============================================================
void setText(JNIEnv *env, jobject obj, const char *text) { return; }
jstring Icon(JNIEnv *env, jobject obj) { return env->NewStringUTF(""); }
jstring IconWebViewData(JNIEnv *env, jobject obj) { return env->NewStringUTF(""); }
jobjectArray SettingsList(JNIEnv *env, jobject obj) { return nullptr; }

// ============================================================
// FEATURE LIST — descriptions baked into titles
// ============================================================
jobjectArray GetFeatureList(JNIEnv *env, jobject context) {
    jobjectArray ret;

    const char *features[] = {
        OBFUSCATE("RichWebView_<html><head><meta name='viewport' content='width=device-width,initial-scale=1'><style>*{box-sizing:border-box;}body{margin:0;padding:6px 4px;background:linear-gradient(180deg,#0d0d0d,#1a0a0a);font-family:sans-serif;text-align:center;}h1{margin:0;font-size:16px;letter-spacing:3px;color:#ff4500;text-shadow:0 0 10px #ff2200,0 0 20px #ff4500;font-weight:700;}p{margin:2px 0 0;font-size:9px;color:#ff9966;letter-spacing:2px;}</style></head><body><h1>TRISPY</h1><p>CHEATS PANEL</p></body></html>"),

        // ---- AIM ----
        OBFUSCATE("Category_⚡ AIM"),
        OBFUSCATE("Toggle_Aimlock Head  (use: in-match)"),
        OBFUSCATE("SeekBar_Aim Smooth  (1=instant 20=slow)_1_20"),
        OBFUSCATE("SeekBar_Aim FOV  (range in degrees)_1_360"),
        OBFUSCATE("Toggle_Ignore Knocked  (skip downed)"),
        OBFUSCATE("Toggle_Visible Check Only  (only visible)"),

        // ---- VISUAL ----
        OBFUSCATE("Category_👁 VISUAL"),
        OBFUSCATE("Toggle_ESP Enemy  (use: in-match)"),
        OBFUSCATE("Toggle_ESP Glow  (highlight enemies)"),
        OBFUSCATE("Toggle_ESP Line  (line to each enemy)"),
        OBFUSCATE("Toggle_ESP Box  (box around enemies)"),
        OBFUSCATE("SeekBar_ESP Range  (max distance)_1_1000"),

        // ---- SYSTEM ----
        OBFUSCATE("Category_⚙ SYSTEM"),
        OBFUSCATE("Toggle_Anti-Cheat Bypass  (turn ON in lobby)"),
        OBFUSCATE("Button_MATCH MODE  (hide panel for 5 min)"),
        OBFUSCATE("Button_Clear Reports  (deletes logs)"),
        OBFUSCATE("Button_Wipe Cache  (clears cached data)"),
        OBFUSCATE("Button_STOP & WIPE  (kill + wipe)"),
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
// BUTTON HANDLER — indices match feature array exactly
// ============================================================
void Changes(JNIEnv *env, jclass clazz, jobject obj,
             jint featNum, jstring featName, jint value,
             jlong lValue, jboolean boolean, jstring jstr) {
    switch (featNum) {
        // AIM
        case 2:  aimlockHead   = boolean; break;
        case 3:  aimSmooth     = value;   break;
        case 4:  aimFov        = value;   break;
        case 5:  ignoreKnocked = boolean; break;
        case 6:  visibleCheck  = boolean; break;

        // VISUAL
        case 8:  espEnemy = boolean; break;
        case 9:  espGlow  = boolean; break;
        case 10: espLine  = boolean; break;
        case 11: espBox   = boolean; break;
        case 12: espRange = value;   break;

        // SYSTEM
        case 14: antiCheat = boolean; break;
        case 15: matchMode = true;    break;  // Java handles hide
        case 16: doClearReports();    break;
        case 17: doWipeCache();       break;
        case 18: doStopAndWipe();     break;

        default: break;
    }

    applyPatches();
}

// ============================================================
// HOOK THREAD
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
    LOGI(OBFUSCATE("trispy thread started"));
    do { sleep(1); } while (!isLibraryLoaded(targetLibName));
    LOGI(OBFUSCATE("%s loaded"), (const char *) targetLibName);
#ifdef __aarch64__
    uintptr_t il2cppBase = g_il2cppELF.base();
#endif
    LOGI(OBFUSCATE("trispy thread done"));
    return nullptr;
}

__attribute__((constructor))
void lib_main() {
    pthread_t tid;
    pthread_create(&tid, NULL, hack_thread, NULL);
}
