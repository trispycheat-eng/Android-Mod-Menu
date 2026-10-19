#include <jni.h>
#include <string>
#include <unistd.h>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <dirent.h>
#include <pthread.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "Includes/Logger.h"
#include "Includes/obfuscate.h"

static void runSuCommand(const char* cmd) {
    char full_cmd[512];
    snprintf(full_cmd, sizeof(full_cmd), "su -c '%s'", cmd);
    system(full_cmd);
}

static void hideVM() {
    // 1. Spoof Hardware & Model (PC Logo Bypass)
    runSuCommand("resetprop ro.product.model SM-G998B");
    runSuCommand("resetprop ro.product.brand samsung");
    runSuCommand("resetprop ro.product.manufacturer samsung");
    runSuCommand("resetprop ro.hardware exynos2100");
    
    // 2. Hide QEMU / VM Traces from ACE
    runSuCommand("resetprop ro.kernel.qemu 0");
    runSuCommand("resetprop ro.kernel.qemu.gles 0");
    runSuCommand("resetprop ro.boot.qemu 0");
    runSuCommand("resetprop ro.vphonegaga 0");
    runSuCommand("resetprop ro.vbox 0");
    
    // 3. Delete obvious VM files (if they exist)
    runSuCommand("rm -rf /system/lib/libvboxsf.so");
    runSuCommand("rm -rf /system/lib64/libvboxsf.so");
    runSuCommand("rm -rf /system/bin/qemud");
    runSuCommand("rm -rf /system/bin/vbox");
    
    // 4. Kill the ACE processes (but not the game)
    runSuCommand("pkill -f terrors");
    runSuCommand("pkill -f ACE");
}

static void nukeOfflineBan() {
    const char* paths[] = {
        "/data/data/com.dts.freefiremax/files",
        "/data/data/com.dts.freefiremax/shared_prefs"
    };
    for (int i = 0; i < 2; i++) {
        DIR* d = opendir(paths[i]);
        if (d) {
            struct dirent* e;
            while ((e = readdir(d)) != NULL) {
                if (strstr(e->d_name, OBFUSCATE("ban")) || 
                    strstr(e->d_name, OBFUSCATE("config")) || 
                    strstr(e->d_name, OBFUSCATE("device"))) {
                    char full_path[256];
                    snprintf(full_path, sizeof(full_path), "%s/%s", paths[i], e->d_name);
                    unlink(full_path);
                }
            }
            closedir(d);
        }
    }
}

static void* guard_thread(void*) {
    sleep(2); 
    hideVM();
    nukeOfflineBan();
    
    // Keep hiding the VM every 10 seconds just in case ACE reloads it
    while (true) {
        hideVM();
        sleep(10); 
    }
    return nullptr;
}

__attribute__((constructor))
void bypass_init() {
    pthread_t tid;
    pthread_create(&tid, NULL, guard_thread, NULL);
}
