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

static int safePrint(lua_State* L) {
    int n = lua_gettop(L);
    (void)n;
    return 0;
}

static int safeCollectGarbage(lua_State* L) {
    const char* mode = lua_tostring(L, 1);
    if (!mode) return 0;
    if (std::string(mode) == "count") {
        lua_pushnumber(L, 0);
        return 1;
    }
    return 0;
}

static int noop(lua_State* L) {
    (void)L;
    return 0;
}

static int returnZero(lua_State* L) {
    lua_pushnumber(L, 0);
    return 1;
}

static int returnOne(lua_State* L) {
    lua_pushnumber(L, 1);
    return 1;
}

static int returnSelf(lua_State* L) {
    lua_pushvalue(L, 1);
    return 1;
}

static int returnWinSize(lua_State* L) {
    lua_newtable(L);
    lua_pushnumber(L, 1280);
    lua_setfield(L, -2, "width");
    lua_pushnumber(L, 720);
    lua_setfield(L, -2, "height");
    return 1;
}

static int returnWritablePath(lua_State* L) {
    lua_pushstring(L, "/data/data/com.domtokima.paddvn/files/");
    return 1;
}

static int returnEmptyString(lua_State* L) {
    lua_pushstring(L, "");
    return 1;
}

static int returnPCL(lua_State* L) {
    lua_pushstring(L, "ANDROID_VIETNAM_VI");
    return 1;
}

static void addMethod(lua_State* L, const char* name, lua_CFunction fn) {
    lua_pushcfunction(L, fn);
    lua_setfield(L, -2, name);
}

static void registerCCDirector(lua_State* L) {
    lua_newtable(L);
    addMethod(L, "sharedDirector", returnSelf);
    addMethod(L, "getWinSize", returnWinSize);
    addMethod(L, "getVisibleSize", returnWinSize);
    addMethod(L, "getVisibleOrigin", returnWinSize);
    addMethod(L, "setDisplayStats", noop);
    addMethod(L, "setAnimationInterval", noop);
    addMethod(L, "getRunningScene", returnZero);
    addMethod(L, "runWithScene", noop);
    addMethod(L, "replaceScene", noop);
    lua_pushvalue(L, -1);
    lua_setglobal(L, "CCDirector");
    lua_pop(L, 1);
}

static void registerCCFileUtils(lua_State* L) {
    lua_newtable(L);
    addMethod(L, "sharedFileUtils", returnSelf);
    addMethod(L, "getWritablePath", returnWritablePath);
    lua_pushvalue(L, -1);
    lua_setglobal(L, "CCFileUtils");
    lua_pop(L, 1);
}

static void registerCCUserDefault(lua_State* L) {
    lua_newtable(L);
    addMethod(L, "sharedUserDefault", returnSelf);
    addMethod(L, "getStringForKey", returnEmptyString);
    addMethod(L, "setStringForKey", noop);
    addMethod(L, "flush", noop);
    lua_pushvalue(L, -1);
    lua_setglobal(L, "CCUserDefault");
    lua_pop(L, 1);
}

static void registerCCBReader(lua_State* L) {
    lua_newtable(L);
    addMethod(L, "getResolutionScale", returnOne);
    lua_pushvalue(L, -1);
    lua_setglobal(L, "CCBReader");
    lua_pop(L, 1);
}

static void registerGlobal(lua_State* L) {
    lua_newtable(L);
    addMethod(L, "instance", returnSelf);
    addMethod(L, "getPCLStr", returnPCL);

    lua_pushnumber(L, 0);
    lua_setfield(L, -2, "screenType");
    lua_pushstring(L, "Arial");
    lua_setfield(L, -2, "MainFont");

    lua_pushvalue(L, -1);
    lua_setglobal(L, "Global");
    lua_pop(L, 1);
}

static void registerSafeRuntime(lua_State* L) {
    lua_pushcfunction(L, safeAssert);
    lua_setglobal(L, "assert");
    lua_pushcfunction(L, safePrint);
    lua_setglobal(L, "print");
    lua_pushcfunction(L, safeCollectGarbage);
    lua_setglobal(L, "collectgarbage");

    registerCCDirector(L);
    registerCCFileUtils(L);
    registerCCUserDefault(L);
    registerCCBReader(L);
    registerGlobal(L);
}

static std::string runLua(const char* data, size_t size) {
    lua_State* L = luaL_newstate();
    if (!L) return "Lua: STATE FAILED";

    registerSafeRuntime(L);

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
    return env->NewStringUTF("Lua ARM64 runtime loaded / Stage 5.3");
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
