#include <jni.h>
#include <string>
#include <unistd.h>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <dirent.h>
#include <pthread.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "Includes/Logger.h"
#include "Includes/obfuscate.h"

// Find memory regions and zero them out
static void scrubMemory() {
    FILE* f = fopen(OBFUSCATE("/proc/self/maps"), "r");
    if (!f) return;
    char line[512];
    
    while (fgets(line, sizeof(line), f)) {
        // Look for our specific string in the memory maps
        if (strstr(line, OBFUSCATE("lspatch")) || 
            strstr(line, OBFUSCATE("LSPatch")) || 
            strstr(line, OBFUSCATE("loader.dex"))) {
            
            unsigned long start, end;
            if (sscanf(line, "%lx-%lx", &start, &end) == 2) {
                size_t len = end - start;
                // Make it writable
                if (mprotect((void*)start, len, PROT_READ | PROT_WRITE) == 0) {
                    // Nuke it (Overwrite with zeros)
                    memset((void*)start, 0, len);
                    LOGI(OBFUSCATE("[RAM-SCRUBBER] Erased LSPatch trace from memory!"));
                }
            }
        }
    }
    fclose(f);
}

static void* scrub_thread(void*) {
    LOGI(OBFUSCATE("[RAM-SCRUBBER] Scrubber thread started."));
    sleep(2); // Wait for game to load
    while (true) {
        scrubMemory();
        sleep(3); // Keep scrubbing every 3 seconds in case it reloads
    }
    return nullptr;
}

__attribute__((constructor))
void scrub_init() {
    pthread_t tid;
    pthread_create(&tid, NULL, scrub_thread, NULL);
}
