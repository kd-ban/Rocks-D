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

static int noop(lua_State* L) { (void)L; return 0; }
static int returnZero(lua_State* L) { lua_pushnumber(L, 0); return 1; }
static int returnOne(lua_State* L) { lua_pushnumber(L, 1); return 1; }
static int returnSelf(lua_State* L) { lua_pushvalue(L, 1); return 1; }
static int returnEmptyString(lua_State* L) { lua_pushstring(L, ""); return 1; }
static int returnPCL(lua_State* L) { lua_pushstring(L, "ANDROID_VIETNAM_VI"); return 1; }
static int returnWritablePath(lua_State* L) {
    lua_pushstring(L, "/data/data/com.domtokima.paddvn/files/");
    return 1;
}

static int returnWinSize(lua_State* L) {
    lua_newtable(L);
    lua_pushnumber(L, 1280); lua_setfield(L, -2, "width");
    lua_pushnumber(L, 720); lua_setfield(L, -2, "height");
    return 1;
}

static int makePoint(lua_State* L) {
    double x = lua_tonumber(L, 1);
    double y = lua_tonumber(L, 2);
    lua_newtable(L);
    lua_pushnumber(L, x); lua_setfield(L, -2, "x");
    lua_pushnumber(L, y); lua_setfield(L, -2, "y");
    return 1;
}

static int makeColor(lua_State* L) {
    lua_newtable(L);
    const char* keys[] = {"r","g","b","a"};
    for (int i = 0; i < 4; ++i) {
        lua_pushnumber(L, lua_tonumber(L, i + 1));
        lua_setfield(L, -2, keys[i]);
    }
    return 1;
}

static int createObject(lua_State* L) {
    lua_newtable(L);
    lua_pushcfunction(L, noop); lua_setfield(L, -2, "addChild");
    lua_pushcfunction(L, noop); lua_setfield(L, -2, "setPosition");
    lua_pushcfunction(L, noop); lua_setfield(L, -2, "setAnchorPoint");
    lua_pushcfunction(L, noop); lua_setfield(L, -2, "setScale");
    lua_pushcfunction(L, noop); lua_setfield(L, -2, "setVisible");
    lua_pushcfunction(L, noop); lua_setfield(L, -2, "setString");
    return 1;
}

static void addMethod(lua_State* L, const char* name, lua_CFunction fn) {
    lua_pushcfunction(L, fn);
    lua_setfield(L, -2, name);
}

static void openSafeStdlibs(lua_State* L) {
    // luaL_openlibs crashed in an earlier ARM64 stage, so only open the
    // non-I/O libraries the recovered client commonly needs.
    lua_pushcfunction(L, luaopen_base); lua_pushstring(L, ""); lua_call(L, 1, 0);
    lua_pushcfunction(L, luaopen_table); lua_pushstring(L, LUA_TABLIBNAME); lua_call(L, 1, 0);
    lua_pushcfunction(L, luaopen_string); lua_pushstring(L, LUA_STRLIBNAME); lua_call(L, 1, 0);
    lua_pushcfunction(L, luaopen_math); lua_pushstring(L, LUA_MATHLIBNAME); lua_call(L, 1, 0);
}

static void registerCCDirector(lua_State* L) {
    lua_newtable(L);
    addMethod(L, "sharedDirector", returnSelf);
    addMethod(L, "getWinSize", returnWinSize);
    addMethod(L, "getVisibleSize", returnWinSize);
    addMethod(L, "getVisibleOrigin", makePoint);
    addMethod(L, "setDisplayStats", noop);
    addMethod(L, "setAnimationInterval", noop);
    addMethod(L, "getRunningScene", createObject);
    addMethod(L, "runWithScene", noop);
    addMethod(L, "replaceScene", noop);
    addMethod(L, "stopAnimation", noop);
    addMethod(L, "startAnimation", noop);
    lua_pushvalue(L, -1); lua_setglobal(L, "CCDirector"); lua_pop(L, 1);
}

static void registerSimpleClass(lua_State* L, const char* name) {
    lua_newtable(L);
    addMethod(L, "create", createObject);
    lua_pushvalue(L, -1); lua_setglobal(L, name); lua_pop(L, 1);
}

static void registerCCFileUtils(lua_State* L) {
    lua_newtable(L);
    addMethod(L, "sharedFileUtils", returnSelf);
    addMethod(L, "getWritablePath", returnWritablePath);
    addMethod(L, "addSearchPath", noop);
    addMethod(L, "isFileExist", returnOne);
    lua_pushvalue(L, -1); lua_setglobal(L, "CCFileUtils"); lua_pop(L, 1);
}

static void registerCCUserDefault(lua_State* L) {
    lua_newtable(L);
    addMethod(L, "sharedUserDefault", returnSelf);
    addMethod(L, "getStringForKey", returnEmptyString);
    addMethod(L, "getIntegerForKey", returnZero);
    addMethod(L, "getBoolForKey", returnZero);
    addMethod(L, "setStringForKey", noop);
    addMethod(L, "setIntegerForKey", noop);
    addMethod(L, "setBoolForKey", noop);
    addMethod(L, "flush", noop);
    lua_pushvalue(L, -1); lua_setglobal(L, "CCUserDefault"); lua_pop(L, 1);
}

static void registerCCBReader(lua_State* L) {
    lua_newtable(L);
    addMethod(L, "getResolutionScale", returnOne);
    lua_pushvalue(L, -1); lua_setglobal(L, "CCBReader"); lua_pop(L, 1);
}

static void registerGlobal(lua_State* L) {
    lua_newtable(L);
    addMethod(L, "instance", returnSelf);
    addMethod(L, "getPCLStr", returnPCL);
    lua_pushnumber(L, 0); lua_setfield(L, -2, "screenType");
    lua_pushstring(L, "Arial"); lua_setfield(L, -2, "MainFont");
    lua_pushvalue(L, -1); lua_setglobal(L, "Global"); lua_pop(L, 1);
}

static void registerCocosHelpers(lua_State* L) {
    lua_pushcfunction(L, makePoint); lua_setglobal(L, "ccp");
    lua_pushcfunction(L, makeColor); lua_setglobal(L, "ccc4");
    registerSimpleClass(L, "CCScene");
    registerSimpleClass(L, "CCLayer");
    registerSimpleClass(L, "CCLayerColor");
    registerSimpleClass(L, "CCSprite");
    registerSimpleClass(L, "CCLabelTTF");
    registerSimpleClass(L, "CCNode");
}

static void registerSafeRuntime(lua_State* L) {
    openSafeStdlibs(L);
    lua_pushcfunction(L, safeAssert); lua_setglobal(L, "assert");
    registerCCDirector(L);
    registerCCFileUtils(L);
    registerCCUserDefault(L);
    registerCCBReader(L);
    registerGlobal(L);
    registerCocosHelpers(L);
}

static std::string runLua(const char* data, size_t size) {
    lua_State* L = luaL_newstate();
    if (!L) return "Lua: STATE FAILED";
    registerSafeRuntime(L);

    int rc = luaL_loadbuffer(L, data, size, "@original.lua");
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
    return env->NewStringUTF("Lua+Cocos ARM64 compatibility / Stage 6.0");
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
