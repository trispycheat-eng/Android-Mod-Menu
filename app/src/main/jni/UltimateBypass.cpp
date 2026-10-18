#include <jni.h>
#include <unistd.h>

// Removed the auto-start constructor to prevent VM crash on launch.
// The panel logic will still work.

__attribute__((constructor))
void bypass_init() {
    // Do nothing on startup to stop the segfault
}
