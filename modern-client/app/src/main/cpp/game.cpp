#include <jni.h>
#include <string>
#include <vector>

extern "C" {
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}

static std::string runLua(const char* data, size_t size) {
    lua_State* L = luaL_newstate();
    if (!L) return "Lua: STATE FAILED";
    int rc = luaL_loadbuffer(L, data, size, "@bootstrap.lua");
    if (rc == 0) rc = lua_pcall(L, 0, 0, 0);
    std::string out;
    if (rc == 0) out = "Lua execute: OK";
    else {
        const char* err = lua_tostring(L, -1);
        out = std::string("Lua execute: ERROR / ") + (err ? err : "unknown");
    }
    lua_close(L);
    return out;
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_domtokima_paddvn_MainActivity_nativeStatus(JNIEnv* env, jclass) {
    return env->NewStringUTF("Lua ARM64 runtime loaded");
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_domtokima_paddvn_MainActivity_nativeRunLuaBytes(JNIEnv* env, jclass, jbyteArray bytes) {
    if (!bytes) return env->NewStringUTF("Lua execute: NO BYTES");
    jsize len = env->GetArrayLength(bytes);
    if (len <= 0 || len > 1024 * 1024) return env->NewStringUTF("Lua execute: BAD SIZE");
    std::vector<char> copy(static_cast<size_t>(len));
    env->GetByteArrayRegion(bytes, 0, len, reinterpret_cast<jbyte*>(copy.data()));
    if (env->ExceptionCheck()) {
        env->ExceptionClear();
        return env->NewStringUTF("Lua execute: JNI COPY ERROR");
    }
    std::string result = runLua(copy.data(), copy.size());
    return env->NewStringUTF(result.c_str());
}

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM*, void*) { return JNI_VERSION_1_6; }
