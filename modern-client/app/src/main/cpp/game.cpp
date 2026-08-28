#include <jni.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <string>
#include <vector>

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
    lua_State* L = luaL_newstate();
    if (!L) return env->NewStringUTF("Lua runtime: FAILED to create state");
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

extern "C" JNIEXPORT jstring JNICALL
Java_com_domtokima_paddvn_MainActivity_nativeRunAsset(JNIEnv* env, jclass, jobject assetManager, jstring path) {
    AAssetManager* mgr = AAssetManager_fromJava(env, assetManager);
    if (!mgr) return env->NewStringUTF("Asset Lua: NO ASSET MANAGER");
    const char* cpath = env->GetStringUTFChars(path, nullptr);
    AAsset* asset = AAssetManager_open(mgr, cpath, AASSET_MODE_BUFFER);
    if (!asset) {
        env->ReleaseStringUTFChars(path, cpath);
        return env->NewStringUTF("Asset Lua: FILE MISSING");
    }
    off_t len = AAsset_getLength(asset);
    std::vector<char> data((size_t)len);
    int read = AAsset_read(asset, data.data(), (size_t)len);
    std::string result = read == len ? runLua(data.data(), data.size(), cpath) : "Asset Lua: READ FAILED";
    AAsset_close(asset);
    env->ReleaseStringUTFChars(path, cpath);
    return env->NewStringUTF(result.c_str());
}

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM*, void*) { return JNI_VERSION_1_6; }
