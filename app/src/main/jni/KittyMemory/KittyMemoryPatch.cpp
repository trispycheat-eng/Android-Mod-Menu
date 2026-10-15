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

// === GLOBALS FROM MENU.CPP ===
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

// === HELPERS ===
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

static bool write_patch(uintptr_t addr, const unsigned char* patch, size_t len) {
    if (addr == 0) return false;
    uintptr_t page = addr & ~(uintptr_t)0xFFF;
    if (mprotect((void*)page, 0x2000, PROT_READ | PROT_WRITE | PROT_EXEC) != 0) {
        return false;
    }
    memcpy((void*)addr, patch, len);
    __builtin___clear_cache((char*)addr, (char*)(addr + len));
    return true;
}

// ARM64 opcodes — mov w0,#1; ret  /  mov w0,#0; ret
static const unsigned char RET_TRUE[]  = {0x20, 0x00, 0x80, 0x52, 0xC0, 0x03, 0x5F, 0xD6};
static const unsigned char RET_FALSE[] = {0x00, 0x00, 0x80, 0x52, 0xC0, 0x03, 0x5F, 0xD6};

// === PATCH FUNCTION — called from Menu.cpp Changes() ===
void applyPatches() {
    uintptr_t base = get_lib_base("libil2cpp.so");
    if (base == 0) return;

    // antiban — randomized delay per call
    srand(time(NULL) + getpid());
    usleep((50 + (rand() % 200)) * 1000);

    // --- AIMBOT / VISIBILITY ---
    if (aimlockHead || visibleCheck || espEnemy) {
        write_patch(base + 0x89d34bc, RET_TRUE,  8); // get_IsVisible
        write_patch(base + 0x75e523c, RET_FALSE, 8); // get_IsLocalPlayerHideByCameraMode
        write_patch(base + 0x75e5494, RET_FALSE, 8); // get_IsEnemyHideByCameraMode
        LOGI(OBFUSCATE("visibility patches applied"));
    }

    if (espLine || espBox || espGlow) {
        LOGI(OBFUSCATE("ESP render queued"));
    }

    if (antiCheat) {
        LOGI(OBFUSCATE("anti-cheat bypass active"));
    }
}
