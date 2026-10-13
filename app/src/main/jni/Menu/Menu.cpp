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

bool noDeath;
int scoreMul = 1, coinsMul = 1;

struct MemPatches {
    MemoryPatch noDeath;
} gPatches;

// ============================================================
//  TRISPY CHEATS · t.me/TrispyCheats
// ============================================================
jobjectArray GetFeatureList(JNIEnv *env, jobject context) {
    jobjectArray ret;

    const char *features[] = {

        // ── ANIMATED HEADER ──
        OBFUSCATE("RichWebView_html=<html><head><style>"
            "@keyframes glow{0%{text-shadow:0 0 8px #ff2200,0 0 16px #ff4500;}50%{text-shadow:0 0 22px #ff6600,0 0 44px #ff4500;}100%{text-shadow:0 0 8px #ff2200,0 0 16px #ff4500;}}"
            "@keyframes flick{0%,100%{opacity:1;}47%{opacity:1;}50%{opacity:0.55;}53%{opacity:1;}}"
            "body{background:#0a0000;margin:0;padding:14px 8px;font-family:sans-serif;}"
            ".h{color:#ff4500;font-size:24px;font-weight:bold;text-align:center;animation:glow 1.8s infinite,flick 3.2s infinite;letter-spacing:4px;}"
            ".sub{color:#cc9966;font-size:11px;text-align:center;margin-top:6px;letter-spacing:2px;}"
            ".line{height:1px;background:linear-gradient(90deg,transparent,#ff4500,transparent);margin:10px 0;}"
            "</style></head><body>"
            "<div class='h'>&#9733; TRISPY &#9733;</div>"
            "<div class='line'></div>"
            "<div class='sub'>by TrispyCheats</div>"
            "<div class='sub'>t.me/TrispyCheats</div>"
            "</body></html>"),

        // ── AIM ──
        OBFUSCATE("Category_AIM"),
        OBFUSCATE("Toggle_Aimlock Head"),
        OBFUSCATE("Toggle_Aimlock Smooth"),
        OBFUSCATE("SeekBar_Aim FOV_1_360"),
        OBFUSCATE("Toggle_Ignore Knocked"),
        OBFUSCATE("Toggle_Visible Check"),

        // ── VISUAL ──
        OBFUSCATE("Category_VISUAL"),
        OBFUSCATE("Toggle_ESP Enemy"),
        OBFUSCATE("Toggle_ESP Glow"),
        OBFUSCATE("Toggle_ESP Line"),
        OBFUSCATE("Toggle_ESP Box"),
        OBFUSCATE("SeekBar_ESP Range_1_500"),

        // ── SYSTEM ──
        OBFUSCATE("Category_SYSTEM"),
        OBFUSCATE("Button_Clear Reports"),
        OBFUSCATE("Button_Wipe Cache"),
        OBFUSCATE("Toggle_Anti-Cheat Bypass"),
        OBFUSCATE("Button_STOP & WIPE"),

        // ── FOOTER ──
        OBFUSCATE("RichTextView_&#9733; t.me/TrispyCheats &#9733;"),
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
//  BUTTON LOGIC — signature MUST match Setup.cpp
//  (JNIEnv*, jclass, jobject, jint, jstring, jint, jlong, jboolean, jstring)
// ============================================================
bool btnPressed = false;

void Changes(JNIEnv *env, jclass clazz, jobject obj,
             jint featNum, jstring featName, jint value,
             jlong jl, jboolean boolean, jstring jstr) {

    switch (featNum) {
        // ── header RichWebView is index 0, ignored ──
        case 1:  // Category_AIM (not interactive)
        case 6:  // Category_VISUAL
        case 12: // Category_SYSTEM
            break;

        case 2:  // Aimlock Head
            if (boolean) { /* hook aim to head */ }
            break;
        case 3:  // Aimlock Smooth
            break;
        case 4:  // Aim FOV slider
            // value = 1..360
            break;
        case 5:  // Ignore Knocked
            break;

        case 7:  // ESP Enemy
            break;
        case 8:  // ESP Glow
            break;
        case 9:  // ESP Line
            break;
        case 10: // ESP Box
            break;
        case 11: // ESP Range slider
            break;

        case 13: // Clear Reports
            if (boolean || btnPressed) { /* clear */ }
            break;
        case 14: // Wipe Cache
            break;
        case 15: // Anti-Cheat Bypass
            break;
        case 16: // STOP & WIPE
            btnPressed = true;
            break;

        default:
            break;
    }
}

// ============================================================
//  NATIVE HOOKS (fill offsets later)
// ============================================================
void (*old_Update)(void *instance);
void Update(void *instance) {
    if (instance != nullptr) {
        if (btnPressed) btnPressed = false;
    }
    return old_Update(instance);
}

void (*old_AddScore)(void *instance, int score);
void AddScore(void *instance, int score) {
    if (instance != nullptr && scoreMul != 1)
        return old_AddScore(instance, score * scoreMul);
    return old_AddScore(instance, score);
}

void (*old_AddCoins)(void *instance, int count);
void AddCoins(void *instance, int count) {
    if (instance != nullptr && coinsMul != 1)
        return old_AddCoins(instance, count * coinsMul);
    return old_AddCoins(instance, count);
}

// ============================================================
//  HOOK THREAD
// ============================================================
#define targetLibName OBFUSCATE("libil2cpp.so")
ElfScanner g_il2cppELF;

void *hack_thread(void *) {
    LOGI(OBFUSCATE("trispy hook thread started"));

    do { sleep(1); } while (!isLibraryLoaded(targetLibName));

    LOGI(OBFUSCATE("%s has been loaded"), (const char *) targetLibName);

#if defined(__aarch64__)
    uintptr_t il2cppBase = g_il2cppELF.base();
    // HOOK(targetLibName, str2offset(OBFUSCATE("0xOFFSET")), AddScore, old_AddScore);
#endif

    LOGI(OBFUSCATE("trispy hook thread done"));
    return nullptr;
}

__attribute__((constructor))
void lib_main() {
    pthread_t ptid;
    pthread_create(&ptid, NULL, hack_thread, NULL);
}
