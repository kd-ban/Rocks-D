#include <jni.h>
#include <string>

extern "C" {
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}

static std::string runLua(const char* data, size_t size, const char* chunk) {
    lua_State* L = luaL_newstate();
    if (!L) return "Lua asset: FAILED state";
    luaL_openlibs(L);
    int rc = luaL_loadbuffer(L, data, size, chunk);
    if (rc == 0) rc = lua_pcall(L, 0, 1, 0);
    std::string out;
    if (rc == 0) {
        const char* value = lua_tostring(L, -1);
        out = std::string("Asset Lua: ") + (value ? value : "OK");
    } else {
        const char* err = lua_tostring(L, -1);
        out = std::string("Asset Lua error: ") + (err ? err : "unknown");
    }
    lua_close(L);
    return out;
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_domtokima_paddvn_MainActivity_nativeStatus(JNIEnv* env, jclass) {
    return env->NewStringUTF("Lua ARM64 runtime loaded");
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_domtokima_paddvn_MainActivity_nativeRunLuaBytes(JNIEnv* env, jclass, jbyteArray bytes, jstring chunkName) {
    if (!bytes) return env->NewStringUTF("Asset Lua: NO BYTES");
    jsize len = env->GetArrayLength(bytes);
    if (len <= 0) return env->NewStringUTF("Asset Lua: EMPTY");
    jbyte* data = env->GetByteArrayElements(bytes, nullptr);
    if (!data) return env->NewStringUTF("Asset Lua: BYTE ACCESS FAILED");
    const char* chunk = "bootstrap.lua";
    const char* acquiredChunk = nullptr;
    if (chunkName) {
        acquiredChunk = env->GetStringUTFChars(chunkName, nullptr);
        if (acquiredChunk) chunk = acquiredChunk;
    }
    std::string result = runLua(reinterpret_cast<const char*>(data), static_cast<size_t>(len), chunk);
    if (acquiredChunk) env->ReleaseStringUTFChars(chunkName, acquiredChunk);
    env->ReleaseByteArrayElements(bytes, data, JNI_ABORT);
    return env->NewStringUTF(result.c_str());
}

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM*, void*) { return JNI_VERSION_1_6; }
