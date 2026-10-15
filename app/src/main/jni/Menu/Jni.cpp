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

// forward declaration of the Changes() function in Menu.cpp
extern void Changes(JNIEnv *env, jclass clazz, jobject obj,
                    jint featNum, jstring featName, jint value,
                    jlong lValue, jboolean boolean, jstring jstr);

// JNI bridge for our Java Menu.Changes() method
extern "C" JNIEXPORT void JNICALL
Java_com_android_support_Menu_Changes(JNIEnv *env, jclass clazz,
                                       jobject ctx, jstring featName,
                                       jint featNum, jlong lValue,
                                       jboolean boolean, jstring str) {
    Changes(env, clazz, ctx, featNum, featName, 0, lValue, boolean, str);
}

// jni stuff from MrDarkRX https://github.com/MrDarkRX/DarkMod-Floating
void setDialog(jobject ctx, JNIEnv *env, const char *title, const char *msg)
{
    jclass Alert = env->FindClass(OBFUSCATE("android/app/AlertDialog$Builder"));
    jmethodID AlertCons = env->GetMethodID(Alert, OBFUSCATE("<init>"), OBFUSCATE("(Landroid/content/Context;)V"));

    jobject MainAlert = env->NewObject(Alert, AlertCons, ctx);

    jmethodID setTitle = env->GetMethodID(Alert, OBFUSCATE("setTitle"), OBFUSCATE("(Ljava/lang/CharSequence;)Landroid/app/AlertDialog$Builder;"));
    env->CallObjectMethod(MainAlert, setTitle, env->NewStringUTF(title));

    jmethodID setMsg = env->GetMethodID(Alert, OBFUSCATE("setMessage"), OBFUSCATE("(Ljava/lang/CharSequence;)Landroid/app/AlertDialog$Builder;"));
    env->CallObjectMethod(MainAlert, setMsg, env->NewStringUTF(msg));

    jmethodID setCa = env->GetMethodID(Alert, OBFUSCATE("setCancelable"), OBFUSCATE("(Z)Landroid/app/AlertDialog$Builder;"));
    env->CallObjectMethod(MainAlert, setCa, false);

    jmethodID setPB = env->GetMethodID(Alert, OBFUSCATE("setPositiveButton"), OBFUSCATE("(Ljava/lang/CharSequence;Landroid/content/DialogInterface$OnClickListener;)Landroid/app/AlertDialog$Builder;"));
    env->CallObjectMethod(MainAlert, setPB, env->NewStringUTF("Ok"), static_cast<jobject>(NULL));

    jmethodID create = env->GetMethodID(Alert, OBFUSCATE("create"), OBFUSCATE("()Landroid/app/AlertDialog;"));
    jobject creatob = env->CallObjectMethod(MainAlert, create);
    jclass AlertN = env->FindClass(OBFUSCATE("android/app/AlertDialog"));
    jmethodID show = env->GetMethodID(AlertN, OBFUSCATE("show"), OBFUSCATE("()V"));
    env->CallVoidMethod(creatob, show);
}

void Toast(JNIEnv *env, jobject thiz, const char *text, int length) {
    jstring jstr = env->NewStringUTF(text);
    jclass toast = env->FindClass(OBFUSCATE("android/widget/Toast"));
    jmethodID methodMakeText = env->GetStaticMethodID(toast, OBFUSCATE("makeText"), OBFUSCATE("(Landroid/content/Context;Ljava/lang/CharSequence;I)Landroid/widget/Toast;"));
    jobject toastobj = env->CallStaticObjectMethod(toast, methodMakeText, thiz, jstr, length);
    jmethodID methodShow = env->GetMethodID(toast, OBFUSCATE("show"), OBFUSCATE("()V"));
    env->CallVoidMethod(toastobj, methodShow);
}

void startService(JNIEnv *env, jobject ctx){
    jclass native_context = env->GetObjectClass(ctx);
    jclass intentClass = env->FindClass(OBFUSCATE("android/content/Intent"));
    jclass actionString = env->FindClass(OBFUSCATE("com/android/support/launcher"));
    jmethodID newIntent = env->GetMethodID(intentClass, OBFUSCATE("<init>"), OBFUSCATE("(Landroid/content/Context;Ljava/lang/Class;)V"));
    jobject intent = env->NewObject(intentClass,newIntent,ctx,actionString);
    jmethodID startActivityMethodId = env->GetMethodID(native_context, OBFUSCATE("startService"), OBFUSCATE("(Landroid/content/Intent;)Landroid/content/ComponentName;"));
    env->CallObjectMethod(ctx, startActivityMethodId, intent);
}

void *exit_thread(void *) {
    sleep(5);
    exit(0);
}

void startActivityPermisson(JNIEnv *env, jobject ctx){
    jclass native_context = env->GetObjectClass(ctx);
    jmethodID startActivity = env->GetMethodID(native_context, OBFUSCATE("startActivity"), OBFUSCATE("(Landroid/content/Intent;)V"));

    jmethodID id = env->GetMethodID(native_context, OBFUSCATE("getPackageName"), OBFUSCATE("()Ljava/lang/String;"));
    jstring packageName = static_cast<jstring>(env->CallObjectMethod(ctx, id));

    const char *pkg = env->GetStringUTFChars(packageName, 0);

    std::stringstream strpkg;
    strpkg << OBFUSCATE("package:");
    strpkg << pkg;
    std::string pkg = strpkg.str();

    jclass uri = env->FindClass(OBFUSCATE("android/net/Uri"));
    jmethodID Parce = env->GetStaticMethodID(uri, OBFUSCATE("parse"), OBFUSCATE("(Ljava/lang/String;)Landroid/net/Uri;"));
    jobject UriMethod = env->CallStaticObjectMethod(uri, Parce, env->NewStringUTF(pkg.c_str()));

    jclass intentClass = env->FindClass(OBFUSCATE("android/content/Intent"));
    jmethodID newIntent = env->GetMethodID(intentClass, OBFUSCATE("<init>"), OBFUSCATE("(Ljava/lang/String;Landroid/net/Uri;)V"));
    jobject intent = env->NewObject(intentClass,newIntent,env->NewStringUTF(OBFUSCATE("android.settings.action.MANAGE_OVERLAY_PERMISSION")), UriMethod);

    env->CallVoidMethod(ctx, startActivity, intent);
}

// Needed jclass parameter because this is a static java method
void CheckOverlayPermission(JNIEnv *env, jclass thiz, jobject ctx)
{
    // If overlay permission option is greyed out, make sure to add android.permission.SYSTEM_ALERT_WINDOW in manifest

    LOGI(OBFUSCATE("Check overlay permission"));

    int sdkVer = api_level();
    if (sdkVer >= 23) { // Android 6.0
        jclass settings = env->FindClass(OBFUSCATE("android/provider/Settings"));
        jmethodID canDraw = env->GetStaticMethodID(settings, OBFUSCATE("canDrawOverlays"), OBFUSCATE("(Landroid/content/Context;)Z"));
        if (!(env->CallStaticBooleanMethod(settings, canDraw, ctx)))
        {
            Toast(env, ctx, OBFUSCATE("Overlay permission is required in order to show mod menu."), 1);
            Toast(env, ctx, OBFUSCATE("Overlay permission is required in order to show mod menu."), 1);
            startActivityPermisson(env, ctx);

            pthread_t ptid;
            pthread_create(&ptid, NULL, exit_thread, NULL);
            return;
        }
    }

    LOGI(OBFUSCATE("Start service"));

    // Start Mod Normal
    startService(env, ctx);
}
