#include <jni.h>
#include <string>
#include <unistd.h>

// Temporarily disabled to stop VM crash
__attribute__((constructor))
void bypass_init() {
    // Do nothing until app is stable
}
