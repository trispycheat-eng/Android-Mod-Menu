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
#include <sys/stat.h>
#include "Includes/Logger.h"
#include "Includes/obfuscate.h"

static void killSelf() {
    LOGI(OBFUSCATE("integrity fail -- self-terminating"));
    kill(getpid(), SIGKILL);
}

static bool checkTracerPidSelf() {
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

static bool checkTracerPidAllThreads() {
    DIR* d = opendir(OBFUSCATE("/proc/self/task"));
    if (!d) return false;
    struct dirent* e;
    bool traced = false;
    while ((e = readdir(d)) != NULL) {
        if (e->d_name[0] < '0' || e->d_name[0] > '9') continue;
        char path[256];
        snprintf(path, sizeof(path), OBFUSCATE("/proc/self/task/%s/status"), e->d_name);
        FILE* f = fopen(path, "r");
        if (!f) continue;
        char line[256];
        while (fgets(line, sizeof(line), f)) {
            if (strncmp(line, OBFUSCATE("TracerPid:"), 10) == 0) {
                int pid = atoi(line + 10);
                if (pid != 0) { traced = true; break; }
            }
        }
        fclose(f);
        if (traced) break;
    }
    closedir(d);
    return traced;
}

static bool checkFridaPorts() {
    FILE* f = fopen(OBFUSCATE("/proc/net/tcp"), "r");
    if (!f) return false;
    char line[512];
    bool found = false;
    while (fgets(line, sizeof(line), f)) {
        if (strstr(line, OBFUSCATE(":69A2")) ||   // 27042
            strstr(line, OBFUSCATE(":69A3")) ||   // 27043
            strstr(line, OBFUSCATE(":21FC"))) {   // 8700 JDWP
            found = true;
            break;
        }
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
            strstr(line, OBFUSCATE("linjector")) ||
            strstr(line, OBFUSCATE("gdbserver")) ||
            strstr(line, OBFUSCATE("android_server"))) {
            found = true;
            break;
        }
    }
    fclose(f);
    return found;
}

static bool checkSuspiciousPaths() {
    const char* paths[] = {
        "/data/local/tmp/frida-server",
        "/data/local/tmp/re.frida.server",
        "/system/bin/frida-server",
        "/system/bin/gdbserver",
        "/system/bin/android_server",
        "/data/local/tmp/gdbserver",
        "/data/local/tmp/android_server"
    };
    for (int i = 0; i < 7; i++) {
        struct stat st;
        if (stat(paths[i], &st) == 0) return true;
    }
    return false;
}

static bool checkSuspiciousProcs() {
    DIR* d = opendir(OBFUSCATE("/proc"));
    if (!d) return false;
    struct dirent* e;
    bool found = false;
    while ((e = readdir(d)) != NULL) {
        if (e->d_name[0] < '0' || e->d_name[0] > '9') continue;
        char path[256];
        snprintf(path, sizeof(path), OBFUSCATE("/proc/%s/cmdline"), e->d_name);
        FILE* f = fopen(path, "r");
        if (!f) continue;
        char cmd[256] = {0};
        size_t n = fread(cmd, 1, sizeof(cmd) - 1, f);
        fclose(f);
        if (n == 0) continue;
        if (strstr(cmd, OBFUSCATE("frida")) ||
            strstr(cmd, OBFUSCATE("gdbserver")) ||
            strstr(cmd, OBFUSCATE("android_server")) ||
            strstr(cmd, OBFUSCATE("magisk")) ||
            strstr(cmd, OBFUSCATE("zygisk"))) {
            found = true;
            break;
        }
    }
    closedir(d);
    return found;
}

static void* guard_thread(void*) {
    LOGI(OBFUSCATE("guard thread started"));
    sleep(3);
    while (true) {
        if (checkTracerPidSelf())       { killSelf(); }
        if (checkTracerPidAllThreads()) { killSelf(); }
        if (checkFridaPorts())          { killSelf(); }
        if (checkFridaMaps())           { killSelf(); }
        if (checkSuspiciousPaths())     { killSelf(); }
        if (checkSuspiciousProcs())     { killSelf(); }
        sleep(5);
    }
    return nullptr;
}

__attribute__((constructor))
void guard_init() {
    pthread_t tid;
    pthread_create(&tid, NULL, guard_thread, NULL);
}
