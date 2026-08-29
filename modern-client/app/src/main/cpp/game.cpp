#include <jni.h>
#include <string>
#include <vector>

extern "C" {
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}

static int safeAssert(lua_State* L) {
    if (!lua_toboolean(L, 1)) return luaL_error(L, "assertion failed");
    lua_settop(L, 1);
    return 1;
}

static int noop(lua_State* L) {
    (void)L;
    return 0;
}

static int returnZero(lua_State* L) {
    lua_pushnumber(L, 0);
    return 1;
}

static int returnSelf(lua_State* L) {
    lua_pushvalue(L, 1);
    return 1;
}

static void addMethod(lua_State* L, const char* name, lua_CFunction fn) {
    lua_pushcfunction(L, fn);
    lua_setfield(L, -2, name);
}

static void registerCCDirector(lua_State* L) {
    // Minimal cocos2d-x 2.x compatibility object so original conf.lua can advance.
    lua_newtable(L);                  // CCDirector
    addMethod(L, "sharedDirector", returnSelf);
    addMethod(L, "getWinSize", returnZero);
    addMethod(L, "getVisibleSize", returnZero);
    addMethod(L, "getVisibleOrigin", returnZero);
    addMethod(L, "setDisplayStats", noop);
    addMethod(L, "setAnimationInterval", noop);
    addMethod(L, "getRunningScene", returnZero);
    addMethod(L, "runWithScene", noop);
    addMethod(L, "replaceScene", noop);
    lua_pushvalue(L, -1);
    lua_setglobal(L, "CCDirector");
    lua_pop(L, 1);
}

static std::string runLua(const char* data, size_t size) {
    lua_State* L = luaL_newstate();
    if (!L) return "Lua: STATE FAILED";

    lua_pushcfunction(L, safeAssert);
    lua_setglobal(L, "assert");
    registerCCDirector(L);

    int rc = luaL_loadbuffer(L, data, size, "@bootstrap.lua");
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
    return env->NewStringUTF("Lua ARM64 runtime loaded / Stage 5.2");
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
