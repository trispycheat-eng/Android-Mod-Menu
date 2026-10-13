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
//  Main menu — floating panel over Free Fire
// ============================================================

bool noDeath = false;
bool aimlockHead = false;
bool aimlockSmooth = false;
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

struct MemPatches {
    MemoryPatch noDeathPatch;
} gPatches;

// ============================================================
//  FEATURE LIST — everything you see in the menu
// ============================================================
jobjectArray GetFeatureList(JNIEnv *env, jobject context) {
    jobjectArray ret;

    const char *features[] = {

        // ── HEADER ──
        OBFUSCATE("RichWebView_html=<html><head><meta name='viewport' content='width=device-width,initial-scale=1'>"
            "<style>"
            "@keyframes glow{0%{text-shadow:0 0 8px #ff2200,0 0 16px #ff4500;}50%{text-shadow:0 0 22px #ff6600,0 0 44px #ff4500;}100%{text-shadow:0 0 8px #ff2200,0 0 16px #ff4500;}}"
            "@keyframes flick{0%,100%{opacity:1;}47%{opacity:1;}50%{opacity:0.55;}53%{opacity:1;}}"
            "body{background:#0a0000;margin:0;padding:14px 8px;font-family:sans-serif;}"
            ".h{color:#ff4500;font-size:22px;font-weight:bold;text-align:center;animation:glow 1.8s infinite,flick 3.2s infinite;letter-spacing:3px;}"
            ".sub{color:#cc9966;font-size:10px;text-align:center;margin-top:5px;letter-spacing:2px;}"
            ".line{height:1px;background:linear-gradient(90deg,transparent,#ff4500,transparent);margin:8px 0;}"
            "</style></head><body>"
            "<div class='h'>&#9733; TRISPY &#9733;</div>"
            "<div class='line'></div>"
            "<div class='sub'>by TrispyCheats</div>"
            "<div class='sub'>t.me/TrispyCheats</div>"
            "</body></html>"),

        // ── AIM ──
        OBFUSCATE("Category_AIM"),
        OBFUSCATE("Toggle_Aimlock Head"),
        OBFUSCATE("SeekBar_Aim Smooth_1_20"),
        OBFUSCATE("SeekBar_Aim FOV_1_360"),
        OBFUSCATE("Toggle_Ignore Knocked"),
        OBFUSCATE("Toggle_Visible Check Only"),

        // ── VISUAL ──
        OBFUSCATE("Category_VISUAL"),
        OBFUSCATE("Toggle_ESP Enemy"),
        OBFUSCATE("Toggle_ESP Glow"),
        OBFUSCATE("Toggle_ESP Line"),
        OBFUSCATE("Toggle_ESP Box"),
        OBFUSCATE("SeekBar_ESP Range_1_1000"),

        // ── SYSTEM ──
        OBFUSCATE("Category_SYSTEM"),
        OBFUSCATE("Toggle_Anti-Cheat Bypass"),
        OBFUSCATE("Button_Clear Reports"),
        OBFUSCATE("Button_Wipe Cache"),
        OBFUSCATE("Button_STOP & WIPE"),

        // ── FOOTER ──
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
//  Exact signature Setup.cpp calls — DO NOT CHANGE
// ============================================================
bool btnPressed = false;

void Changes(JNIEnv *env, jclass clazz, jobject obj,
             jint featNum, jstring featName, jint value,
             jlong Lvalue, jboolean boolean, jstring jstr) {

    switch (featNum) {
        // 0 = header (not interactive)
        case 0: break;

        // 1 = Category_AIM (not interactive)
        case 1: break;

        // 2 = Aimlock Head
        case 2:
            aimlockHead = boolean;
            break;

        // 3 = Aim Smooth slider
        case 3:
            aimSmooth = value;
            break;

        // 4 = Aim FOV slider
        case 4:
            aimFov = value;
            break;

        // 5 = Ignore Knocked
        case 5:
            ignoreKnocked = boolean;
            break;

        // 6 = Visible Check Only
        case 6:
            visibleCheck = boolean;
            break;

        // 7 = Category_VISUAL
        case 7: break;

        // 8 = ESP Enemy
        case 8:
            espEnemy = boolean;
            break;

        // 9 = ESP Glow
        case 9:
            espGlow = boolean;
            break;

        // 10 = ESP Line
        case 10:
            espLine = boolean;
            break;

        // 11 = ESP Box
        case 11:
            espBox = boolean;
            break;

        // 12 = ESP Range slider
        case 12:
            espRange = value;
            break;

        // 13 = Category_SYSTEM
        case 13: break;

        // 14 = Anti-Cheat Bypass
        case 14:
            antiCheat = boolean;
            break;

        // 15 = Clear Reports
        case 15:
            if (boolean) {
                // wipes FF cache via native side (stub)
            }
            break;

        // 16 = Wipe Cache
        case 16:
            if (boolean) {
                // wipes session files (stub)
            }
            break;

        // 17 = STOP & WIPE
        case 17:
            if (boolean) {
                btnPressed = true;
            }
            break;

        // 18 = footer (not interactive)
        case 18: break;

        default:
            break;
    }
}

// ============================================================
//  HOOKS — attach to libil2cpp.so
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
    // Hook offsets go here once you find them with il2cpp dumper
    // HOOK(targetLibName, str2offset(OBFUSCATE("0xOFFSET")), Update, old_Update);
#endif

    LOGI(OBFUSCATE("trispy hook thread done"));
    return nullptr;
}

__attribute__((constructor))
void lib_main() {
    pthread_t ptid;
    pthread_create(&ptid, NULL, hack_thread, NULL);
}
