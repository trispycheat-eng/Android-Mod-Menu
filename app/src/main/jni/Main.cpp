#include <jni.h>
#include <pthread.h>
#include <unistd.h>
#include <dlfcn.h>
#include "Includes/Logger.h"
#include "Includes/obfuscate.h"
#include "Menu/Menu.hpp"

__attribute__((constructor))
void main_lib() {
}
