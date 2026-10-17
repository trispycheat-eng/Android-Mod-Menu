#include <jni.h>
#include <string>
#include <unistd.h>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <dirent.h>
#include <pthread.h>
#include <sys/mman.h>
#include "Includes/Logger.h"
#include "Includes/obfuscate.h"

static void scrubMemory() {
    FILE* f = fopen(OBFUSCATE("/proc/self/maps"), "r");
    if (!f) return;
    char line[512];
    
    while (fgets(line, sizeof(line), f)) {
        // Target JingMatrix specific strings in memory
        if (strstr(line, OBFUSCATE("lspatch")) || 
            strstr(line, OBFUSCATE("lspd")) || 
            strstr(line, OBFUSCATE("loader.dex"))) {
            
            unsigned long start, end;
            if (sscanf(line, "%lx-%lx", &start, &end) == 2) {
                size_t len = end - start;
                if (mprotect((void*)start, len, PROT_READ | PROT_WRITE) == 0) {
                    memset((void*)start, 0, len);
                    LOGI(OBFUSCATE("[RAM-SCRUBBER] Erased JingMatrix trace from RAM!"));
                }
            }
        }
    }
    fclose(f);
}

static void* scrub_thread(void*) {
    LOGI(OBFUSCATE("[RAM-SCRUBBER] Scrubber active."));
    sleep(3); // Wait for JingMatrix loader to finish, then wipe its traces
    while (true) {
        scrubMemory();
        sleep(4); 
    }
    return nullptr;
}

__attribute__((constructor))
void scrub_init() {
    pthread_t tid;
    pthread_create(&tid, NULL, scrub_thread, NULL);
}
