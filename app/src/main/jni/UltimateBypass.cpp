#include <jni.h>
#include <string>
#include <unistd.h>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <dirent.h>
#include <pthread.h>

#include "Includes/Logger.h"
#include "Includes/obfuscate.h"

static void runSuCommand(const char* cmd) {
    char full_cmd[512];
    snprintf(full_cmd, sizeof(full_cmd), "su -c '%s'", cmd);
    system(full_cmd);
}

static void hideVM() {
    // 1. Create a fake build.prop
    runSuCommand("echo 'ro.product.model=SM-G998B' > /data/local/tmp/build.prop");
    runSuCommand("echo 'ro.product.brand=samsung' >> /data/local/tmp/build.prop");
    runSuCommand("echo 'ro.product.manufacturer=samsung' >> /data/local/tmp/build.prop");
    runSuCommand("echo 'ro.hardware=exynos2100' >> /data/local/tmp/build.prop");
    runSuCommand("echo 'ro.kernel.qemu=0' >> /data/local/tmp/build.prop");
    runSuCommand("echo 'ro.vphonegaga=0' >> /data/local/tmp/build.prop");
    runSuCommand("echo 'ro.vbox=0' >> /data/local/tmp/build.prop");
    runSuCommand("echo 'ro.secure=1' >> /data/local/tmp/build.prop");

    // 2. Bind mount the fake build.prop over the real one (requires root)
    runSuCommand("mount -o bind /data/local/tmp/build.prop /system/build.prop");

    // 3. Create a fake cpuinfo to hide QEMU
    runSuCommand("echo 'Hardware : exynos2100' > /data/local/tmp/cpuinfo");
    runSuCommand("echo 'Processor : AArch64 Processor rev 1 (aarch64)' >> /data/local/tmp/cpuinfo");
    runSuCommand("mount -o bind /data/local/tmp/cpuinfo /proc/cpuinfo");

    // 4. Kill ACE processes
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
