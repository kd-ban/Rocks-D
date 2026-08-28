#include <jni.h>
#include <string>

extern "C" {
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_domtokima_paddvn_MainActivity_nativeStatus(JNIEnv* env, jclass) {
    lua_State* L = luaL_newstate();
    if (!L) return env->NewStringUTF("Lua runtime: FAILED to create state");

    // Stage 3.1: minimal ARM64 Lua smoke test. Do not open OS/io libraries yet.
    const char* script = "return 'Lua ARM64 OK', 40 + 2";
    int rc = luaL_loadstring(L, script);
    if (rc == 0) rc = lua_pcall(L, 0, 2, 0);

    std::string status;
    if (rc == 0) {
        const char* text = lua_tostring(L, -2);
        int value = (int)lua_tointeger(L, -1);
        status = std::string(text ? text : "Lua OK") + " / test=" + std::to_string(value);
    } else {
        const char* error = lua_tostring(L, -1);
        status = std::string("Lua error: ") + (error ? error : "unknown");
    }

    lua_close(L);
    return env->NewStringUTF(status.c_str());
}

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM*, void*) {
    return JNI_VERSION_1_6;
}
