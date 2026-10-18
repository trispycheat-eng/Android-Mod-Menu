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
#include <map>
#include <string>

#include "Includes/Logger.h"
#include "Includes/obfuscate.h"
#include "Dobby/dobby.h"

// ============================================================================
// 1. KERNEL PROPERTY SPOOFER (PC LOGO / EMULATOR BYPASS)
// ============================================================================

// Original function pointers
static int (*orig_system_property_get)(const char* name, char* value) = nullptr;
static const prop_info* (*orig_system_property_find)(const char* name) = nullptr;
static void (*orig_system_property_read_callback)(const prop_info* pi, void (*callback)(void* cookie, const char* name, const char* value, uint32_t serial), void* cookie) = nullptr;

// Fake Samsung Galaxy S21 Ultra properties
struct FakeProp {
    const char* name;
    const char* value;
};

static FakeProp fake_props[] = {
    {"ro.product.model", "SM-G998B"},
    {"ro.product.brand", "samsung"},
    {"ro.product.manufacturer", "samsung"},
    {"ro.product.device", "o1s"},
    {"ro.product.board", "exynos2100"},
    {"ro.product.name", "o1sxeea"},
    {"ro.hardware", "exynos2100"},
    {"ro.board.platform", "exynos2100"},
    {"ro.arch", "arm64-v8a"},
    {"ro.kernel.qemu", "0"},
    {"ro.kernel.qemu.gles", "0"},
    {"ro.boot.qemu", "0"},
    {"ro.boot.hardware", "exynos2100"},
    {"ro.hardware.chipname", "Exynos 2100"},
    {"ro.build.fingerprint", "samsung/o1sxeea/o1s:13/TP1A.220624.014/G998BXXU5CVDD:user/release-keys"},
    {"ro.build.version.sdk", "33"},
    {"ro.build.version.release", "13"},
    {"ro.build.type", "user"},
    {"ro.build.tags", "release-keys"},
    {"ro.debuggable", "0"},
    {"ro.secure", "1"},
    {"ro.adb.secure", "1"},
    {"ro.dalvik.vm.native.bridge", "0"},
    {"ro.enable.native.bridge.exec", "0"},
    {"ro.vendor.vboxsf", "0"},
    {"ro.vendor.vboxguest", "0"},
    {"ro.vphonegaga", "0"},
    {"ro.vbox", "0"},
    {"ro.vm", "0"},
    {"sys.boot_completed", "1"},
    {"sys.usb.config", "mtp,adb"},
    {"sys.usb.state", "mtp,adb"},
    {"init.svc.qemud", "stopped"},
    {"init.svc.qemu-props", "stopped"},
    {"init.svc.vbox", "stopped"}
};

int my_system_property_get(const char* name, char* value) {
    if (name != nullptr) {
        for (int i = 0; i < sizeof(fake_props)/sizeof(fake_props[0]); i++) {
            if (strcmp(name, fake_props[i].name) == 0) {
                strcpy(value, fake_props[i].value);
                return strlen(value);
            }
        }
    }
    return orig_system_property_get(name, value);
}

const prop_info* my_system_property_find(const char* name) {
    if (name != nullptr) {
        for (int i = 0; i < sizeof(fake_props)/sizeof(fake_props[0]); i++) {
            if (strcmp(name, fake_props[i].name) == 0) {
                return orig_system_property_find(name); // Return real prop if exists, or we can fake it later
            }
        }
    }
    return orig_system_property_find(name);
}

// ============================================================================
// 2. FILE SYSTEM HOOKS (HIDE VM, MAGISK, LSPATCH)
// ============================================================================

static int (*orig_openat)(int dirfd, const char *pathname, int flags, mode_t mode) = nullptr;
static FILE* (*orig_fopen)(const char *pathname, const char *mode) = nullptr;
static int (*orig_stat)(const char *pathname, struct stat *statbuf) = nullptr;
static int (*orig_lstat)(const char *pathname, struct stat *statbuf) = nullptr;
static int (*orig_access)(const char *pathname, int mode) = nullptr;

bool isSuspiciousPath(const char* path) {
    if (path == nullptr) return false;
    
    // VM detection paths
    if (strstr(path, "vphonegaga") || strstr(path, "vbox") || 
        strstr(path, "qemu") || strstr(path, "genymotion") ||
        strstr(path, "bluestacks") || strstr(path, "ldplayer")) return true;
        
    // Root / Magisk detection paths
    if (strstr(path, "magisk") || strstr(path, "su") || 
        strstr(path, "superuser") || strstr(path, "zygisk") ||
        strstr(path, "busybox")) return true;
        
    // Frida / Debugger detection paths
    if (strstr(path, "frida") || strstr(path, "gadget") || 
        strstr(path, "xposed") || strstr(path, "lspatch") ||
        strstr(path, "linjector")) return true;
        
    // Our own payload paths (hide from ACE)
    if (strstr(path, "libcrypto") || strstr(path, "libsys_check")) return true;

    return false;
}

int my_openat(int dirfd, const char *pathname, int flags, mode_t mode) {
    if (isSuspiciousPath(pathname)) {
        errno = ENOENT;
        return -1;
    }
    return orig_openat(dirfd, pathname, flags, mode);
}

FILE* my_fopen(const char *pathname, const char *mode) {
    if (isSuspiciousPath(pathname)) {
        errno = ENOENT;
        return nullptr;
    }
    return orig_fopen(pathname, mode);
}

int my_stat(const char *pathname, struct stat *statbuf) {
    if (isSuspiciousPath(pathname)) {
        errno = ENOENT;
        return -1;
    }
    return orig_stat(pathname, statbuf);
}

int my_lstat(const char *pathname, struct stat *statbuf) {
    if (isSuspiciousPath(pathname)) {
        errno = ENOENT;
        return -1;
    }
    return orig_lstat(pathname, statbuf);
}

int my_access(const char *pathname, int mode) {
    if (isSuspiciousPath(pathname)) {
        errno = ENOENT;
        return -1;
    }
    return orig_access(pathname, mode);
}

// ============================================================================
// 3. ANTI-DEBUG & PTRACE
// ============================================================================

static int (*orig_ptrace)(int request, pid_t pid, void *addr, void *data) = nullptr;

int my_ptrace(int request, pid_t pid, void *addr, void *data) {
    // Block any attempt to attach or read our memory
    if (request == PTRACE_ATTACH || request == PTRACE_SEIZE || 
        request == PTRACE_PEEKDATA || request == PTRACE_PEEKTEXT) {
        errno = EPERM;
        return -1;
    }
    return orig_ptrace(request, pid, addr, data);
}

// ============================================================================
// 4. BAN CACHE NUKE (OFFLINE BAN / VIRTUAL DEVICE BAN)
// ============================================================================

void nukeBanCache() {
    const char* paths[] = {
        "/data/data/com.dts.freefiremax/files",
        "/data/data/com.dts.freefiremax/shared_prefs",
        "/data/data/com.dts.freefiremax/databases",
        "/data/data/com.dts.freefireth/files",
        "/data/data/com.dts.freefireth/shared_prefs"
    };

    for (int i = 0; i < 5; i++) {
        DIR* d = opendir(paths[i]);
        if (d) {
            struct dirent* e;
            while ((e = readdir(d)) != NULL) {
                if (strstr(e->d_name, "ban") || strstr(e->d_name, "device") || 
                    strstr(e->d_name, "config") || strstr(e->d_name, "user") ||
                    strstr(e->d_name, "account") || strstr(e->d_name, "cache")) {
                    
                    char full_path[512];
                    snprintf(full_path, sizeof(full_path), "%s/%s", paths[i], e->d_name);
                    // Use the original syscall to bypass our own hook
                    orig_openat(AT_FDCWD, full_path, O_RDONLY, 0);
                    unlink(full_path);
                }
            }
            closedir(d);
        }
    }
    LOGI(OBFUSCATE("[VM-BYPASS] Ban cache nuked."));
}

// ============================================================================
// 5. MEMORY MAP SCRUBBER (HIDE VM LIBS)
// ============================================================================

void scrubMemoryMaps() {
    FILE* f = orig_fopen(OBFUSCATE("/proc/self/maps"), "r");
    if (!f) return;
    
    char line[512];
    while (fgets(line, sizeof(line), f)) {
        if (strstr(line, OBFUSCATE("vbox")) || strstr(line, OBFUSCATE("vphone")) || 
            strstr(line, OBFUSCATE("qemu")) || strstr(line, OBFUSCATE("liblspatch")) ||
            strstr(line, OBFUSCATE("libcrypto"))) {
            
            unsigned long start, end;
            if (sscanf(line, "%lx-%lx", &start, &end) == 2) {
                size_t len = end - start;
                // Overwrite the string in memory so ACE can't read it
                memset((void*)start, 0, len);
            }
        }
    }
    fclose(f);
}

// ============================================================================
// 6. WATCHDOG THREAD (PREVENTS CRASHES & KEEPS BYPASS ALIVE)
// ============================================================================

static void* vm_guard_thread(void*) {
    LOGI(OBFUSCATE("[VM-BYPASS] Universal VM Bypass active."));
    
    // Nuke the ban cache immediately on startup
    nukeBanCache();

    // Apply Hooks
    void* handle = dlopen("libc.so", RTLD_NOW);
    if (handle) {
        void* ptrace_sym = dlsym(handle, OBFUSCATE("ptrace"));
        void* openat_sym = dlsym(handle, OBFUSCATE("openat"));
        void* fopen_sym = dlsym(handle, OBFUSCATE("fopen"));
        void* stat_sym = dlsym(handle, OBFUSCATE("stat"));
        void* lstat_sym = dlsym(handle, OBFUSCATE("lstat"));
        void* access_sym = dlsym(handle, OBFUSCATE("access"));
        
        if (ptrace_sym) DobbyHook(ptrace_sym, (void*)my_ptrace, (void**)&orig_ptrace);
        if (openat_sym) DobbyHook(openat_sym, (void*)my_openat, (void**)&orig_openat);
        if (fopen_sym) DobbyHook(fopen_sym, (void*)my_fopen, (void**)&orig_fopen);
        if (stat_sym) DobbyHook(stat_sym, (void*)my_stat, (void**)&orig_stat);
        if (lstat_sym) DobbyHook(lstat_sym, (void*)my_lstat, (void**)&orig_lstat);
        if (access_sym) DobbyHook(access_sym, (void*)my_access, (void**)&orig_access);
        
        dlclose(handle);
    }

    void* libcutils = dlopen("libcutils.so", RTLD_NOW);
    if (libcutils) {
        void* prop_get_sym = dlsym(libcutils, OBFUSCATE("__system_property_get"));
        void* prop_find_sym = dlsym(libcutils, OBFUSCATE("__system_property_find"));
        
        if (prop_get_sym) DobbyHook(prop_get_sym, (void*)my_system_property_get, (void**)&orig_system_property_get);
        if (prop_find_sym) DobbyHook(prop_find_sym, (void*)my_system_property_find, (void**)&orig_system_property_find);
        
        dlclose(libcutils);
    }

    LOGI(OBFUSCATE("[VM-BYPASS] All hooks applied."));

    while (true) {
        // Constantly scrub memory and nuke caches to prevent bans
        scrubMemoryMaps();
        nukeBanCache();
        sleep(5); 
    }
    return nullptr;
}

__attribute__((constructor))
void vm_bypass_init() {
    pthread_t tid;
    pthread_create(&tid, NULL, vm_guard_thread, NULL);
}
