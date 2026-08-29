#include <jni.h>
#include <string>
#include <vector>

extern "C" {
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}

// Stage 6.3: keep the first Lua execution path deliberately minimal.
// Stage 6.2 proved that the process dies inside nativeRunLuaBytes().
// The previous compatibility bootstrap registered many Cocos stubs before
// executing even a tiny Lua chunk. 6.3 isolates Lua itself first; original
// Cocos bindings will be restored incrementally after this gate is stable.
static std::string runLuaMinimal(const char* data, size_t size) {
    lua_State* L = luaL_newstate();
    if (!L) return "Lua: STATE FAILED";

    // bootstrap.lua only needs base language functionality (assert, tostring,
    // concatenation and tables). Opening the base library is enough and avoids
    // the wider compatibility registration that was active at the crash point.
    lua_pushcfunction(L, luaopen_base);
    lua_pushstring(L, "");
    lua_call(L, 1, 0);

    int rc = luaL_loadbuffer(L, data, size, "@stage6_3.lua");
    if (rc == 0) rc = lua_pcall(L, 0, 1, 0);

    std::string out;
    if (rc == 0) {
        const char* value = lua_tostring(L, -1);
        out = std::string("Lua execute: OK") + (value ? std::string(" / ") + value : "");
    } else {
        const char* err = lua_tostring(L, -1);
        out = std::string("Lua execute: ERROR / ") + (err ? err : "unknown");
    }

    lua_close(L);
    return out;
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_domtokima_paddvn_MainActivity_nativeStatus(JNIEnv* env, jclass) {
    return env->NewStringUTF("Lua ARM64 minimal runtime / Stage 6.3");
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_domtokima_paddvn_MainActivity_nativeRunLuaBytes(JNIEnv* env, jclass, jbyteArray bytes) {
    if (!bytes) return env->NewStringUTF("Lua execute: NO BYTES");

    jsize len = env->GetArrayLength(bytes);
    if (len <= 0 || len > 1024 * 1024)
        return env->NewStringUTF("Lua execute: BAD SIZE");

    std::vector<char> copy(static_cast<size_t>(len));
    env->GetByteArrayRegion(bytes, 0, len, reinterpret_cast<jbyte*>(copy.data()));
    if (env->ExceptionCheck()) {
        env->ExceptionClear();
        return env->NewStringUTF("Lua execute: JNI COPY ERROR");
    }

    std::string result = runLuaMinimal(copy.data(), copy.size());
    return env->NewStringUTF(result.c_str());
}

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM*, void*) {
    return JNI_VERSION_1_6;
}
