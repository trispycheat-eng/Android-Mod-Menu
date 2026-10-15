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
// TRISPY CHEATS - t.me/TrispyCheats
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

extern void applyPatches();

// ============================================================
// ACTION FUNCTIONS (for buttons)
// ============================================================
static void doClearReports() {
    LOGI(OBFUSCATE("clearing reports..."));
    // kill any report writers, delete report/crash folders in our process space
    system("rm -rf /data/data/com.dts.freefireth/files/report* 2>/dev/null");
    system("rm -rf /data/data/com.dts.freefireth/files/crash* 2>/dev/null");
    system("rm -rf /data/data/com.dts.freefiremax/files/report* 2>/dev/null");
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
    LOGI(OBFUSCATE("stop & wipe - killing process"));
    doWipeCache();
    doClearReports();
    // kill our own process (in virtual space, this IS the game)
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
// FEATURE LIST - with descriptions
// ============================================================
jobjectArray GetFeatureList(JNIEnv *env, jobject context) {
    jobjectArray ret;

    const char *features[] = {
        // compact header
        OBFUSCATE("RichWebView_<html><head><meta name='viewport' content='width=device-width,initial-scale=1'><style>*{box-sizing:border-box;}body{margin:0;padding:6px 4px;background:linear-gradient(180deg,#0d0d0d,#1a0a0a);font-family:sans-serif;text-align:center;}h1{margin:0;font-size:16px;letter-spacing:3px;color:#ff4500;text-shadow:0 0 10px #ff2200,0 0 20px #ff4500;font-weight:700;}p{margin:2px 0 0;font-size:9px;color:#ff9966;letter-spacing:2px;}i{display:inline-block;width:6px;height:6px;background:#ff2200;border-radius:50%;box-shadow:0 0 6px #ff4500;margin:0 4px;}</style></head><body><h1>TRISPY</h1><p><i></i>CHEATS<i></i></p></body></html>"),

        // ---- AIM ----
        OBFUSCATE("Category_⚡ AIM"),
        OBFUSCATE("Toggle_Aimlock Head"),
        OBFUSCATE("TextView_▶ Use: LOBBY or IN-GAME"),
        OBFUSCATE("TextView_▶ Locks crosshair to enemy head"),
        OBFUSCATE("SeekBar_Aim Smooth_1_20"),
        OBFUSCATE("TextView_▶ 1 = instant, 20 = slow"),
        OBFUSCATE("SeekBar_Aim FOV_1_360"),
        OBFUSCATE("TextView_▶ Detection range (degrees)"),
        OBFUSCATE("Toggle_Ignore Knocked"),
        OBFUSCATE("TextView_▶ Skip downed enemies"),
        OBFUSCATE("Toggle_Visible Check Only"),
        OBFUSCATE("TextView_▶ Only aim at visible targets"),

        // ---- VISUAL ----
        OBFUSCATE("Category_👁 VISUAL"),
        OBFUSCATE("Toggle_ESP Enemy"),
        OBFUSCATE("TextView_▶ Use: IN-GAME only"),
        OBFUSCATE("TextView_▶ See enemies through walls"),
        OBFUSCATE("Toggle_ESP Glow"),
        OBFUSCATE("TextView_▶ Highlights enemies with glow"),
        OBFUSCATE("Toggle_ESP Line"),
        OBFUSCATE("TextView_▶ Draws line to each enemy"),
        OBFUSCATE("Toggle_ESP Box"),
        OBFUSCATE("TextView_▶ Box outline around enemies"),
        OBFUSCATE("SeekBar_ESP Range_1_1000"),
        OBFUSCATE("TextView_▶ Max distance for ESP"),

        // ---- SYSTEM ----
        OBFUSCATE("Category_⚙ SYSTEM"),
        OBFUSCATE("Toggle_Anti-Cheat Bypass"),
        OBFUSCATE("TextView_▶ Turn ON in LOBBY before match"),
        OBFUSCATE("TextView_▶ Keeps patches hidden from ACE"),
        OBFUSCATE("Button_Clear Reports"),
        OBFUSCATE("TextView_▶ Run anytime — deletes logs"),
        OBFUSCATE("Button_Wipe Cache"),
        OBFUSCATE("TextView_▶ Run anytime — clears cached data"),
        OBFUSCATE("Button_STOP & WIPE"),
        OBFUSCATE("TextView_▶ Kills game + wipes everything"),
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
// BUTTON HANDLER — featNum indexes match feature array above
// ============================================================
void Changes(JNIEnv *env, jclass clazz, jobject obj,
             jint featNum, jstring featName, jint value,
             jlong lValue, jboolean boolean, jstring jstr) {
    switch (featNum) {

        // ---- AIM ----
        case 3:  aimlockHead   = boolean; break;
        case 5:  aimSmooth     = value;   break;
        case 7:  aimFov        = value;   break;
        case 9:  ignoreKnocked = boolean; break;
        case 11: visibleCheck  = boolean; break;

        // ---- VISUAL ----
        case 14: espEnemy = boolean; break;
        case 17: espGlow  = boolean; break;
        case 20: espLine  = boolean; break;
        case 23: espBox   = boolean; break;
        case 25: espRange = value;   break;

        // ---- SYSTEM ----
        case 28: antiCheat = boolean; break;
        case 31: doClearReports();    break;   // Clear Reports button
        case 34: doWipeCache();       break;   // Wipe Cache button
        case 37: doStopAndWipe();     break;   // STOP & WIPE button

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
    LOGI(OBFUSCATE("trispy hook thread started"));
    do { sleep(1); } while (!isLibraryLoaded(targetLibName));
    LOGI(OBFUSCATE("%s loaded"), (const char *) targetLibName);
#ifdef __aarch64__
    uintptr_t il2cppBase = g_il2cppELF.base();
#endif
    LOGI(OBFUSCATE("trispy hook thread done"));
    return nullptr;
}

__attribute__((constructor))
void lib_main() {
    pthread_t tid;
    pthread_create(&tid, NULL, hack_thread, NULL);
}
