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
#include <sys/ptrace.h>
#include <sys/syscall.h>
#include <fcntl.h>
#include <dlfcn.h>
#include <sys/system_properties.h>

#include "Includes/Logger.h"
#include "Includes/obfuscate.h"
#include "Dobby/dobby.h"

// =====================================================================
// 1. VPHONEGAGA SPOOFER (Hide VM from ACE)
// =====================================================================
static int (*orig_system_property_get)(const char* name, char* value) = nullptr;

int my_system_property_get(const char* name, char* value) {
    if (name != nullptr) {
        // ACE checks these to see if you are in an emulator/VM
        if (strstr(name, OBFUSCATE("ro.hardware")) || 
            strstr(name, OBFUSCATE("ro.product.model")) ||
            strstr(name, OBFUSCATE("ro.product.brand"))) {
            
            // Feed it fake Samsung S21 Ultra properties
            strcpy(value, OBFUSCATE("SM-G998B"));
            return strlen(value);
        }
        if (strstr(name, OBFUSCATE("ro.product.manufacturer"))) {
            strcpy(value, OBFUSCATE("samsung"));
            return strlen(value);
        }
    }
    return orig_system_property_get(name, value);
}

// =====================================================================
// 2. SYSCALL HOOKS (Hide files from ACE)
// =====================================================================
static int (*orig_ptrace)(int request, pid_t pid, void *addr, void *data) = nullptr;
static int (*orig_openat)(int dirfd, const char *pathname, int flags, mode_t mode) = nullptr;

int my_ptrace(int request, pid_t pid, void *addr, void *data) {
    if (request == PTRACE_ATTACH || request == PTRACE_SEIZE) {
        return 0; // Lie to ACE
    }
    return orig_ptrace(request, pid, addr, data);
}

int my_openat(int dirfd, const char *pathname, int flags, mode_t mode) {
    if (pathname != nullptr) {
        // Hide VM files, LSPatch files, and our payload
        if (strstr(pathname, OBFUSCATE("vphonegaga")) || 
            strstr(pathname, OBFUSCATE("vphone")) || 
            strstr(pathname, OBFUSCATE("lspatch")) ||
            strstr(pathname, OBFUSCATE("loader.dex")) ||
            strstr(pathname, OBFUSCATE("libcrypto")) || 
            strstr(pathname, OBFUSCATE("magisk")) ||
            strstr(pathname, OBFUSCATE("frida"))) {
            return -1; // File not found
        }
    }
    return orig_openat(dirfd, pathname, flags, mode);
}

// =====================================================================
// 3. OFFLINE BAN NUKER (24/7 Protection)
// =====================================================================
static void nukeOfflineBan() {
    // Delete FF's local ban cache and config files
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
                    LOGI(OBFUSCATE("[OFFLINE-NUKE] Erased ban trace: %s"), full_path);
                }
            }
            closedir(d);
        }
    }
}

// =====================================================================
// 4. ACE THREAD FREEZER
// =====================================================================
static void suspendACEThreads() {
    DIR* d = opendir(OBFUSCATE("/proc/self/task"));
    if (!d) return;
    struct dirent* e;
    while ((e = readdir(d)) != NULL) {
        if (e->d_name[0] < '0' || e->d_name[0] > '9') continue;
        char path[256];
        snprintf(path, sizeof(path), OBFUSCATE("/proc/self/task/%s/comm"), e->d_name);
        FILE* f = fopen(path, "r");
        if (f) {
            char comm[64] = {0};
            if (fgets(comm, sizeof(comm), f)) {
                if (strstr(comm, OBFUSCATE("ACE")) || strstr(comm, OBFUSCATE("terrors")) || strstr(comm, OBFUSCATE("TP"))) {
                    int tid = atoi(e->d_name);
                    syscall(SYS_tgkill, getpid(), tid, SIGSTOP); 
                }
            }
            fclose(f);
        }
    }
    closedir(d);
}

// =====================================================================
// 5. WATCHDOG
// =====================================================================
static void* guard_thread(void*) {
    LOGI(OBFUSCATE("[VM-BYPASS] God Tier VPhoneGaGa protection active."));
    nukeOfflineBan();

    void* handle = dlopen("libc.so", RTLD_NOW);
    if (handle) {
        void* ptrace_sym = dlsym(handle, OBFUSCATE("ptrace"));
        void* openat_sym = dlsym(handle, OBFUSCATE("openat"));
        void* prop_sym = dlsym(handle, OBFUSCATE("__system_property_get"));
        
        if (ptrace_sym) DobbyHook(ptrace_sym, (void*)my_ptrace, (void**)&orig_ptrace);
        if (openat_sym) DobbyHook(openat_sym, (void*)my_openat, (void**)&orig_openat);
        if (prop_sym)   DobbyHook(prop_sym, (void*)my_system_property_get, (void**)&orig_system_property_get);
        
        LOGI(OBFUSCATE("[VM-BYPASS] Hooks applied."));
        dlclose(handle);
    }

    while (true) {
        suspendACEThreads();
        sleep(1); 
    }
    return nullptr;
}

__attribute__((constructor))
void bypass_init() {
    pthread_t tid;
    pthread_create(&tid, NULL, guard_thread, NULL);
}thread, NULL);
}
