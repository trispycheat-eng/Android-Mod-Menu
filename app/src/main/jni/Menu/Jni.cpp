#include "Jni.hpp"
#include <list>
#include <vector>
#include <cstring>
#include <string>
#include <pthread.h>
#include <thread>
#include <unistd.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <dlfcn.h>
#include "Includes/obfuscate.h"
#include "Includes/get_device_api_level_inlines.h"
#include "Menu/Jni.hpp"
#include "Includes/Logger.h"

// forward declaration of Changes() in Menu.cpp
extern void Changes(JNIEnv *env, jclass clazz, jobject obj,
                    jint featNum, jstring featName, jint value,
                    jlong lValue, jboolean boolean, jstring jstr);

// ============================================================
// JNI BRIDGE — Java Menu.Changes() → C++ Changes()
// ============================================================
extern "C" JNIEXPORT void JNICALL
Java_com_android_support_Menu_Changes(JNIEnv *env, jclass clazz,
                                       jobject ctx, jstring featName,
                                       jint featNum, jlong lValue,
                                       jboolean boolean, jstring str) {
    if (env == nullptr) return;
    Changes(env, clazz, ctx, featNum, featName, 0, lValue, boolean, str);
}

// ============================================================
// UI HELPERS
// ============================================================
void setDialog(jobject ctx, JNIEnv *env, const char *title, const char *msg) {
    if (env == nullptr || ctx == nullptr) return;

    jclass Alert = env->FindClass(OBFUSCATE("android/app/AlertDialog$Builder"));
    if (Alert == nullptr) return;

    jmethodID AlertCons = env->GetMethodID(Alert, OBFUSCATE("<init>"),
                                           OBFUSCATE("(Landroid/content/Context;)V"));
    if (AlertCons == nullptr) return;

    jobject MainAlert = env->NewObject(Alert, AlertCons, ctx);
    if (MainAlert == nullptr) return;

    jmethodID setTitle = env->GetMethodID(Alert, OBFUSCATE("setTitle"),
                                          OBFUSCATE("(Ljava/lang/CharSequence;)Landroid/app/AlertDialog$Builder;"));
    env->CallObjectMethod(MainAlert, setTitle, env->NewStringUTF(title));

    jmethodID setMsg = env->GetMethodID(Alert, OBFUSCATE("setMessage"),
                                        OBFUSCATE("(Ljava/lang/CharSequence;)Landroid/app/AlertDialog$Builder;"));
    env->CallObjectMethod(MainAlert, setMsg, env->NewStringUTF(msg));

    jmethodID setCa = env->GetMethodID(Alert, OBFUSCATE("setCancelable"),
                                       OBFUSCATE("(Z)Landroid/app/AlertDialog$Builder;"));
    env->CallObjectMethod(MainAlert, setCa, false);

    jmethodID setPB = env->GetMethodID(Alert, OBFUSCATE("setPositiveButton"),
                                       OBFUSCATE("(Ljava/lang/CharSequence;Landroid/content/DialogInterface$OnClickListener;)Landroid/app/AlertDialog$Builder;"));
    env->CallObjectMethod(MainAlert, setPB, env->NewStringUTF("Ok"), static_cast<jobject>(NULL));

    jmethodID create = env->GetMethodID(Alert, OBFUSCATE("create"),
                                        OBFUSCATE("()Landroid/app/AlertDialog;"));
    if (create == nullptr) return;
    jobject creatob = env->CallObjectMethod(MainAlert, create);
    if (creatob == nullptr) return;

    jclass AlertN = env->FindClass(OBFUSCATE("android/app/AlertDialog"));
    if (AlertN == nullptr) return;
    jmethodID show = env->GetMethodID(AlertN, OBFUSCATE("show"), OBFUSCATE("()V"));
    env->CallVoidMethod(creatob, show);
}

void Toast(JNIEnv *env, jobject thiz, const char *text, int length) {
    if (env == nullptr || thiz == nullptr) return;
    jstring jstr = env->NewStringUTF(text);
    jclass toast = env->FindClass(OBFUSCATE("android/widget/Toast"));
    if (toast == nullptr) return;
    jmethodID methodMakeText = env->GetStaticMethodID(toast, OBFUSCATE("makeText"),
        OBFUSCATE("(Landroid/content/Context;Ljava/lang/CharSequence;I)Landroid/widget/Toast;"));
    if (methodMakeText == nullptr) return;
    jobject toastobj = env->CallStaticObjectMethod(toast, methodMakeText, thiz, jstr, length);
    if (toastobj == nullptr) return;
    jmethodID methodShow = env->GetMethodID(toast, OBFUSCATE("show"), OBFUSCATE("()V"));
    env->CallVoidMethod(toastobj, methodShow);
}

void startService(JNIEnv *env, jobject ctx) {
    if (env == nullptr || ctx == nullptr) return;

    jclass native_context = env->GetObjectClass(ctx);
    jclass intentClass = env->FindClass(OBFUSCATE("android/content/Intent"));
    jclass actionString = env->FindClass(OBFUSCATE("com/android/support/launcher"));
    if (intentClass == nullptr || actionString == nullptr) return;

    jmethodID newIntent = env->GetMethodID(intentClass, OBFUSCATE("<init>"),
                                            OBFUSCATE("(Landroid/content/Context;Ljava/lang/Class;)V"));
    if (newIntent == nullptr) return;

    jobject intent = env->NewObject(intentClass, newIntent, ctx, actionString);
    if (intent == nullptr) return;

    jmethodID startServiceMethodId = env->GetMethodID(native_context, OBFUSCATE("startService"),
                                                       OBFUSCATE("(Landroid/content/Intent;)Landroid/content/ComponentName;"));
    if (startServiceMethodId == nullptr) return;
    env->CallObjectMethod(ctx, startServiceMethodId, intent);
}

void *exit_thread(void *) {
    sleep(5);
    exit(0);
}

void startActivityPermisson(JNIEnv *env, jobject ctx) {
    if (env == nullptr || ctx == nullptr) return;

    jclass native_context = env->GetObjectClass(ctx);
    jmethodID startActivity = env->GetMethodID(native_context, OBFUSCATE("startActivity"),
                                               OBFUSCATE("(Landroid/content/Intent;)V"));
    if (startActivity == nullptr) return;

    jmethodID getId = env->GetMethodID(native_context, OBFUSCATE("getPackageName"),
                                       OBFUSCATE("()Ljava/lang/String;"));
    if (getId == nullptr) return;

    jstring packageName = static_cast<jstring>(env->CallObjectMethod(ctx, getId));
    if (packageName == nullptr) return;

    const char *pkgRaw = env->GetStringUTFChars(packageName, 0);
    if (pkgRaw == nullptr) return;

    std::stringstream strpkg;
    strpkg << OBFUSCATE("package:") << pkgRaw;
    std::string pkgStr = strpkg.str();

    env->ReleaseStringUTFChars(packageName, pkgRaw);

    jclass uri = env->FindClass(OBFUSCATE("android/net/Uri"));
    if (uri == nullptr) return;
    jmethodID Parce = env->GetStaticMethodID(uri, OBFUSCATE("parse"),
                                             OBFUSCATE("(Ljava/lang/String;)Landroid/net/Uri;"));
    if (Parce == nullptr) return;
    jobject UriMethod = env->CallStaticObjectMethod(uri, Parce, env->NewStringUTF(pkgStr.c_str()));
    if (UriMethod == nullptr) return;

    jclass intentClass = env->FindClass(OBFUSCATE("android/content/Intent"));
    if (intentClass == nullptr) return;
    jmethodID newIntent = env->GetMethodID(intentClass, OBFUSCATE("<init>"),
                                            OBFUSCATE("(Ljava/lang/String;Landroid/net/Uri;)V"));
    if (newIntent == nullptr) return;
    jobject intent = env->NewObject(intentClass, newIntent,
                                    env->NewStringUTF(OBFUSCATE("android.settings.action.MANAGE_OVERLAY_PERMISSION")),
                                    UriMethod);
    if (intent == nullptr) return;
    env->CallVoidMethod(ctx, startActivity, intent);
}

// Needed jclass parameter because this is a static java method
void CheckOverlayPermission(JNIEnv *env, jclass thiz, jobject ctx) {
    if (env == nullptr || ctx == nullptr) return;

    LOGI(OBFUSCATE("Check overlay permission"));

    int sdkVer = api_level();
    if (sdkVer >= 23) { // Android 6.0+
        jclass settings = env->FindClass(OBFUSCATE("android/provider/Settings"));
        if (settings == nullptr) return;
        jmethodID canDraw = env->GetStaticMethodID(settings, OBFUSCATE("canDrawOverlays"),
                                                    OBFUSCATE("(Landroid/content/Context;)Z"));
        if (canDraw == nullptr) return;
        if (!(env->CallStaticBooleanMethod(settings, canDraw, ctx))) {
            Toast(env, ctx, OBFUSCATE("Overlay permission is required in order to show mod menu."), 1);
            startActivityPermisson(env, ctx);
            pthread_t ptid;
            pthread_create(&ptid, NULL, exit_thread, NULL);
            return;
        }
    }

    LOGI(OBFUSCATE("Start service"));
    startService(env, ctx);
}
