# Rocks-D ARM64 migration

Goal: rebuild the old Android client so it can run on modern 64-bit Android while keeping the existing game assets/Lua and private-server endpoint.

## Confirmed from the original APK

- Package: `com.domtokima.paddvn`
- Original native ABI: `armeabi` only
- Native library: `lib/armeabi/libgame.so`
- ELF: 32-bit ARM EABI5
- Engine traces: cocos2d-x 2.1.4
- Lua traces: LuaJIT 2.0.1 / Lua 5.1
- Required system libs: GLESv2, log, z, stdc++, m, c, dl
- JNI entry points include the old cocos2d-x renderer plus `com.octopus.HT.util.OPJ2CUtil` callbacks.

## Why renaming the folder is not enough

`libgame.so` is an ARM32 ELF binary. Moving it to `lib/arm64-v8a/` does not convert it to ARM64. A modern client needs a newly compiled 64-bit native library (or a newer compatible client base).

## Migration plan

1. Recreate a buildable cocos2d-x/Lua client for `arm64-v8a`.
2. Reimplement the game-specific native/JNI bridge used by the recovered Lua client.
3. Keep the original `assets/` and encrypted `.op` Lua pipeline.
4. Preserve the private-server endpoints currently patched to `http://192.168.8.59/pirate/public`.
5. Restore Auto Guest and network signing/response decoding.
6. Build and test an installable ARM64 APK before migrating gameplay modules incrementally.

This directory is the workspace for the modern-client rebuild. It is not yet a completed replacement for the old `libgame.so`.
