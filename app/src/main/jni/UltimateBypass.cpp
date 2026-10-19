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

// Physically delete VPhoneGaGa trace files that ACE scans
static void cleanVMTraces() {
    const char* vm_paths[] = {
        "/system/lib/libvboxsf.so",
        "/system/lib64/libvboxsf.so",
        "/system/lib/libqemu-props.so",
        "/system/lib64/libqemu-props.so",
        "/system/bin/qemud",
        "/system/bin/vbox",
        "/system/bin/vphonegaga",
        "/data/local/tmp/vphonegaga"
    };
    
    for (int i = 0; i < 8; i++) {
        if (access(vm_paths[i], F_OK) != -1) {
            char cmd[256];
            // Use root to delete the file
            snprintf(cmd, sizeof(cmd), OBFUSCATE("su -c 'rm -rf %s'"), vm_paths[i]);
            system(cmd);
            LOGI(OBFUSCATE("[VM-CLEANER] Deleted: %s"), vm_paths[i]);
        }
    }
}

// Hide the "qemu" strings from ACE by clearing the property files
static void cleanProperties() {
    char cmd[256];
    snprintf(cmd, sizeof(cmd), OBFUSCATE("su -c 'resetprop ro.kernel.qemu 0'"));
    system(cmd);
    snprintf(cmd, sizeof(cmd), OBFUSCATE("su -c 'resetprop ro.vphonegaga 0'"));
    system(cmd);
    snprintf(cmd, sizeof(cmd), OBFUSCATE("su -c 'resetprop ro.product.model SM-G998B'"));
    system(cmd);
    snprintf(cmd, sizeof(cmd), OBFUSCATE("su -c 'resetprop ro.product.manufacturer samsung'"));
    system(cmd);
    LOGI(OBFUSCATE("[VM-CLEANER] Properties spoofed."));
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
    // Wait for VM to fully boot
    sleep(2); 
    cleanVMTraces();
    cleanProperties();
    nukeOfflineBan();
    
    while (true) {
        sleep(5); 
    }
    return nullptr;
}

__attribute__((constructor))
void bypass_init() {
    pthread_t tid;
    pthread_create(&tid, NULL, guard_thread, NULL);
}
