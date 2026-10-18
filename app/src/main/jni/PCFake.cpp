#include <jni.h>
#include <string>
#include <unistd.h>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <dlfcn.h>
#include <sys/system_properties.h>

#include "Includes/Logger.h"
#include "Includes/obfuscate.h"
#include "Dobby/dobby.h"

// Original function pointers
static int (*orig_system_property_get)(const char* name, char* value) = nullptr;
static int (*orig_system_property_get_legacy)(const char* name, char* value) = nullptr;

struct FakeProp {
    const char* name;
    const char* value;
};

// Fake Samsung Galaxy S21 Ultra properties to kill PC Logo and Emulator detection
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
    {"init.svc.qemud", "stopped"},
    {"init.svc.qemu-props", "stopped"},
    {"init.svc.vbox", "stopped"}
};

// Hook for older Android versions
int my_system_property_get_legacy(const char* name, char* value) {
    if (name != nullptr) {
        for (size_t i = 0; i < sizeof(fake_props)/sizeof(fake_props[0]); i++) {
            if (strcmp(name, fake_props[i].name) == 0) {
                strcpy(value, fake_props[i].value);
                return strlen(value);
            }
        }
    }
    return orig_system_property_get_legacy(name, value);
}

// Hook for newer Android versions (Android 8+)
int my_system_property_get(const char* name, char* value) {
    if (name != nullptr) {
        for (size_t i = 0; i < sizeof(fake_props)/sizeof(fake_props[0]); i++) {
            if (strcmp(name, fake_props[i].name) == 0) {
                strcpy(value, fake_props[i].value);
                return strlen(value);
            }
        }
    }
    return orig_system_property_get(name, value);
}

// Initialize and apply hooks
static void apply_pc_spoof_hooks() {
    void* handle = dlopen("libc.so", RTLD_NOW);
    if (!handle) handle = dlopen("libcutils.so", RTLD_NOW);
    
    if (handle) {
        void* prop_get_sym = dlsym(handle, OBFUSCATE("__system_property_get"));
        void* prop_get_legacy_sym = dlsym(handle, OBFUSCATE("__system_property_get_legacy"));
        
        if (prop_get_sym) {
            DobbyHook(prop_get_sym, (void*)my_system_property_get, (void**)&orig_system_property_get);
            LOGI(OBFUSCATE("[PC-FAKE] Hooked __system_property_get"));
        }
        if (prop_get_legacy_sym) {
            DobbyHook(prop_get_legacy_sym, (void*)my_system_property_get_legacy, (void**)&orig_system_property_get_legacy);
            LOGI(OBFUSCATE("[PC-FAKE] Hooked legacy property get"));
        }
        dlclose(handle);
    }
}

__attribute__((constructor))
void pc_fake_init() {
    // Delay slightly to let the app process start
    sleep(1);
    apply_pc_spoof_hooks();
}
