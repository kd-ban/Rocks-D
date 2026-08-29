#include <jni.h>
#include <GLES2/gl2.h>
#include <atomic>

static std::atomic<bool> gPaused{false};
static int gWidth = 0;
static int gHeight = 0;

extern "C" JNIEXPORT void JNICALL
Java_com_domtokima_paddvn_MainActivity_nativeInit(JNIEnv*, jclass) {
    gPaused = false;
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glClearColor(0.02f, 0.02f, 0.03f, 1.0f);
}

extern "C" JNIEXPORT void JNICALL
Java_com_domtokima_paddvn_MainActivity_nativeResize(JNIEnv*, jclass, jint width, jint height) {
    gWidth = width;
    gHeight = height;
    glViewport(0, 0, width, height);
}

extern "C" JNIEXPORT void JNICALL
Java_com_domtokima_paddvn_MainActivity_nativeRender(JNIEnv*, jclass) {
    if (gPaused) return;
    if (gWidth <= 0 || gHeight <= 0) return;
    glClear(GL_COLOR_BUFFER_BIT);
}

extern "C" JNIEXPORT void JNICALL
Java_com_domtokima_paddvn_MainActivity_nativePause(JNIEnv*, jclass) {
    gPaused = true;
}

extern "C" JNIEXPORT void JNICALL
Java_com_domtokima_paddvn_MainActivity_nativeResume(JNIEnv*, jclass) {
    gPaused = false;
}

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM*, void*) {
    return JNI_VERSION_1_6;
}
