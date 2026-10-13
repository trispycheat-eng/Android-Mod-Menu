#include <jni.h>
#include <pthread.h>
#include <unistd.h>
#include <dlfcn.h>
#include "Includes/Logger.h"
#include "Includes/obfuscate.h"
#include "Menu/Menu.hpp"

// Main.cpp stays minimal — all features and Changes() live in Menu.cpp

__attribute__((constructor))
void main_lib() {
    // nothing here — Menu.cpp handles init
}
