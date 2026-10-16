#include <jni.h>
#include <string>
#include <unistd.h>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <dirent.h>
#include <pthread.h>
#include <signal.h>
#include <sys/types.h>
#include "Includes/Logger.h"
#include "Includes/obfuscate.h"

static void killSelf() {
    LOGI(OBFUSCATE("integrity fail -- self-terminating"));
    kill(getpid(), SIGKILL);
}

static bool checkTracerPid() {
    FILE* f = fopen(OBFUSCATE("/proc/self/status"), "r");
    if (!f) return false;
    char line[256];
    bool traced = false;
    while (fgets(line, sizeof(line), f)) {
        if (strncmp(line, OBFUSCATE("TracerPid:"), 10) == 0) {
            int pid = atoi(line + 10);
            if (pid != 0) traced = true;
            break;
        }
    }
    fclose(f);
    return traced;
}

static bool checkFridaPort() {
    FILE* f = fopen(OBFUSCATE("/proc/net/tcp"), "r");
    if (!f) return false;
    char line[512];
    bool found = false;
    while (fgets(line, sizeof(line), f)) {
        if (strstr(line, OBFUSCATE(":69A2"))) { found = true; break; }
    }
    fclose(f);
    return found;
}

static bool checkFridaMaps() {
    FILE* f = fopen(OBFUSCATE("/proc/self/maps"), "r");
    if (!f) return false;
    char line[512];
    bool found = false;
    while (fgets(line, sizeof(line), f)) {
        if (strstr(line, OBFUSCATE("frida")) ||
            strstr(line, OBFUSCATE("gum-js")) ||
            strstr(line, OBFUSCATE("gadget")) ||
            strstr(line, OBFUSCATE("linjector"))) {
            found = true;
            break;
        }
    }
    fclose(f);
    return found;
}

static void* guard_thread(void*) {
    LOGI(OBFUSCATE("guard thread started"));
    sleep(3);
    while (true) {
        if (checkTracerPid())  { killSelf(); }
        if (checkFridaPort())  { killSelf(); }
        if (checkFridaMaps())  { killSelf(); }
        sleep(5);
    }
    return nullptr;
}

__attribute__((constructor))
void guard_init() {
    pthread_t tid;
    pthread_create(&tid, NULL, guard_thread, NULL);
}
