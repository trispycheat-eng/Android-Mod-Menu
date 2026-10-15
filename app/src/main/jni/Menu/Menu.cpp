// ============================================================
// BUTTON HANDLER
// ============================================================
void Changes(JNIEnv *env, jclass clazz, jobject obj,
             jint featNum, jstring featName, jint value,
             jlong lValue, jboolean boolean, jstring jstr) {
    switch (featNum) {
        case 1:
            break;
        case 2:
            aimlockHead = boolean;
            break;
        case 3:
            aimSmooth = value;
            break;
        case 4:
            aimFov = value;
            break;
        case 5:
            ignoreKnocked = boolean;
            break;
        case 6:
            visibleCheck = boolean;
            break;
        case 7:
            break;
        case 8:
            espEnemy = boolean;
            break;
        case 9:
            espGlow = boolean;
            break;
        case 10:
            espLine = boolean;
            break;
        case 11:
            espBox = boolean;
            break;
        case 12:
            espRange = value;
            break;
        case 13:
            break;
        case 14:
            antiCheat = boolean;
            break;
        case 15:
            break;
        case 16:
            break;
        case 17:
            btnPressed = boolean;
            break;
        case 18:
            break;
        default:
            break;
    }

    // re-apply patches when any switch flips
    extern void applyPatches();
    applyPatches();
}
