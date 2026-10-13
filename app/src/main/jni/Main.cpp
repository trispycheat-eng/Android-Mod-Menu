#include <jni.h>
#include <pthread.h>
#include <unistd.h>
#include <dlfcn.h>
#include "Includes/Logger.h"
#include "Includes/obfuscate.h"
#include "Menu/Menu.hpp"

// This file is intentionally minimal.
// All features + the Changes() function live in Menu.cpp
// so they link against Setup.cpp correctly.

__attribute__((constructor))
void main_lib() {
    // nothing here — Menu.cpp handles the real init
}
