#include <jni.h>
#include <string>
#include <unistd.h>
#include <dlfcn.h>
#include <link.h>
#include <sys/mman.h>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <pthread.h>
#include "Includes/Logger.h"
#include "Includes/obfuscate.h"

extern bool aimlockHead;
extern bool ignoreKnocked;
extern bool visibleCheck;
extern bool espEnemy;
extern bool espGlow;
extern bool espLine;
extern bool espBox;
extern bool antiCheat;
extern int aimFov;
extern int espRange;
extern int aimSmooth;
extern bool btnPressed;
extern bool matchMode;

static uintptr_t get_lib_base(const char* name) {
    uintptr_t base = 0;
    FILE* f = fopen("/proc/self/maps", "r");
    if (!f) return 0;
    char line[512];
    while (fgets(line, sizeof(line), f)) {
        if (strstr(line, name)) {
            base = strtoul(line, nullptr, 16);
            break;
        }
    }
    fclose(f);
    return base;
}

struct PatchRecord {
    uintptr_t addr;
    unsigned char original[16];
    size_t len;
    bool active;
};

static PatchRecord g_patches[32];
static int g_patchCount = 0;

static bool write_patch(uintptr_t addr, const unsigned char* patch, size_t len) {
    if (addr == 0) return false;
    if (g_patchCount < 32) {
        bool already = false;
        for (int i = 0; i < g_patchCount; i++) {
            if (g_patches[i].addr == addr) { already = true; break; }
        }
        if (!already) {
            PatchRecord& rec = g_patches[g_patchCount++];
            rec.addr = addr;
            rec.len = len;
            rec.active = true;
            memcpy(rec.original, (void*)addr, len);
        }
    }
    uintptr_t page = addr & ~(uintptr_t)0xFFF;
    if (mprotect((void*)page, 0x2000, PROT_READ | PROT_WRITE | PROT_EXEC) != 0) return false;
    memcpy((void*)addr, patch, len);
    __builtin___clear_cache((char*)addr, (char*)(addr + len));
    return true;
}

void restoreAllPatches() {
    for (int i = 0; i < g_patchCount; i++) {
        if (!g_patches[i].active) continue;
        uintptr_t page = g_patches[i].addr & ~(uintptr_t)0xFFF;
        mprotect((void*)page, 0x2000, PROT_READ | PROT_WRITE | PROT_EXEC);
        memcpy((void*)g_patches[i].addr, g_patches[i].original, g_patches[i].len);
        __builtin___clear_cache((char*)g_patches[i].addr,
                                (char*)(g_patches[i].addr + g_patches[i].len));
    }
    g_patchCount = 0;
    LOGI(OBFUSCATE("all patches restored"));
}

static const unsigned char RET_TRUE[]  = {0x20, 0x00, 0x80, 0x52, 0xC0, 0x03, 0x5F, 0xD6};
static const unsigned char RET_FALSE[] = {0x00, 0x00, 0x80, 0x52, 0xC0, 0x03, 0x5F, 0xD6};

static bool inMatch = false;
static bool patchesFired = false;
static time_t matchStartTime = 0;
static const int GATE_SECONDS = 240;

static bool isUnityOnTop() {
    return get_lib_base("libil2cpp.so") != 0;
}

static void firePatches(uintptr_t base) {
    if (aimlockHead || visibleCheck || espEnemy) {
        write_patch(base + 0x89d34bc, RET_TRUE, 8);
        write_patch(base + 0x75e523c, RET_FALSE, 8);
        write_patch(base + 0x75e5494, RET_FALSE, 8);
        LOGI(OBFUSCATE("visibility patches applied"));
    }
    if (espLine || espBox || espGlow) {
        LOGI(OBFUSCATE("ESP render queued"));
    }
    if (antiCheat) {
        LOGI(OBFUSCATE("anti-cheat bypass active"));
    }
}

void applyPatches() {
    uintptr_t base = get_lib_base("libil2cpp.so");
    if (base == 0) return;
    bool unityNow = isUnityOnTop();

    if (unityNow && !inMatch) {
        inMatch = true;
        patchesFired = false;
        matchStartTime = time(NULL);
        LOGI(OBFUSCATE("match started -- 4 min gate armed"));
    }
    if (!unityNow && inMatch) {
        inMatch = false;
        matchStartTime = 0;
        patchesFired = false;
        restoreAllPatches();
        LOGI(OBFUSCATE("match ended -- patches restored"));
        return;
    }
    if (!inMatch) return;

    long elapsed = (long)(time(NULL) - matchStartTime);
    if (!patchesFired && elapsed < GATE_SECONDS) {
        LOGI(OBFUSCATE("gate locked -- %lds remaining"), (long)(GATE_SECONDS - elapsed));
        return;
    }
    if (!patchesFired) {
        patchesFired = true;
        LOGI(OBFUSCATE("gate opened -- injecting now"));
    }
    srand(time(NULL) ^ getpid());
    usleep((50 + (rand() % 200)) * 1000);
    firePatches(base);
}

void* patch_watcher(void*) {
    LOGI(OBFUSCATE("patch watcher started"));
    while (true) {
        applyPatches();
        sleep(1);
    }
    return nullptr;
}

__attribute__((constructor))
void patch_watcher_init() {
    pthread_t tid;
    pthread_create(&tid, NULL, patch_watcher, NULL);
}
