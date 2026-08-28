#include <jni.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
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
Java_com_domtokima_paddvn_MainActivity_nativeRunAsset(JNIEnv* env, jclass, jobject assetManager, jstring path) {
    if (!assetManager || !path) return env->NewStringUTF("Asset Lua: INVALID JNI INPUT");
    AAssetManager* mgr = AAssetManager_fromJava(env, assetManager);
    if (!mgr) return env->NewStringUTF("Asset Lua: NO ASSET MANAGER");
    const char* cpath = env->GetStringUTFChars(path, nullptr);
    if (!cpath) return env->NewStringUTF("Asset Lua: PATH ERROR");
    AAsset* asset = AAssetManager_open(mgr, cpath, AASSET_MODE_STREAMING);
    if (!asset) {
        env->ReleaseStringUTFChars(path, cpath);
        return env->NewStringUTF("Asset Lua: FILE MISSING");
    }
    const off_t len = AAsset_getLength(asset);
    std::string data;
    if (len > 0) {
        data.resize((size_t)len);
        size_t total = 0;
        while (total < (size_t)len) {
            int n = AAsset_read(asset, &data[total], (size_t)len - total);
            if (n <= 0) break;
            total += (size_t)n;
        }
        data.resize(total);
    }
    AAsset_close(asset);
    std::string chunk(cpath);
    env->ReleaseStringUTFChars(path, cpath);
    if (data.empty()) return env->NewStringUTF("Asset Lua: READ FAILED");
    std::string result = runLua(data.data(), data.size(), chunk.c_str());
    return env->NewStringUTF(result.c_str());
}

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM*, void*) { return JNI_VERSION_1_6; }
