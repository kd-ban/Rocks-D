cmake_minimum_required(VERSION 3.22.1)
project(rocksd_game LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

set(COCOS_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../../../../engine/cocos2d-x-2.1.4/cocos2dx")

add_library(game SHARED
    game.cpp
    "${COCOS_ROOT}/cocos2d.cpp"
    "${COCOS_ROOT}/CCConfiguration.cpp"
    "${COCOS_ROOT}/CCScheduler.cpp"
    "${COCOS_ROOT}/CCCamera.cpp"
    "${COCOS_ROOT}/CCDirector.cpp"
    "${COCOS_ROOT}/base_nodes/CCAtlasNode.cpp"
    "${COCOS_ROOT}/base_nodes/CCNode.cpp"
    "${COCOS_ROOT}/layers_scenes_transitions_nodes/CCScene.cpp"
    "${COCOS_ROOT}/layers_scenes_transitions_nodes/CCTransition.cpp"
    "${COCOS_ROOT}/cocoa/CCAffineTransform.cpp"
    "${COCOS_ROOT}/cocoa/CCGeometry.cpp"
    "${COCOS_ROOT}/cocoa/CCAutoreleasePool.cpp"
    "${COCOS_ROOT}/cocoa/CCDictionary.cpp"
    "${COCOS_ROOT}/cocoa/CCNS.cpp"
    "${COCOS_ROOT}/cocoa/CCObject.cpp"
    "${COCOS_ROOT}/cocoa/CCSet.cpp"
    "${COCOS_ROOT}/cocoa/CCString.cpp"
    "${COCOS_ROOT}/cocoa/CCZone.cpp"
    "${COCOS_ROOT}/cocoa/CCArray.cpp"
    "${COCOS_ROOT}/cocoa/CCDataVisitor.cpp"
    "${COCOS_ROOT}/actions/CCAction.cpp"
    "${COCOS_ROOT}/actions/CCActionManager.cpp"
    "${COCOS_ROOT}/keypad_dispatcher/CCKeypadDispatcher.cpp"
    "${COCOS_ROOT}/keypad_dispatcher/CCKeypadDelegate.cpp"
    "${COCOS_ROOT}/touch_dispatcher/CCTouch.cpp"
    "${COCOS_ROOT}/touch_dispatcher/CCTouchHandler.cpp"
    "${COCOS_ROOT}/touch_dispatcher/CCTouchDispatcher.cpp"
    "${COCOS_ROOT}/script_support/CCScriptSupport.cpp"
    "${COCOS_ROOT}/textures/CCTexture2D.cpp"
    "${COCOS_ROOT}/textures/CCTextureCache.cpp"
    "${COCOS_ROOT}/textures/CCTextureAtlas.cpp"
    "${COCOS_ROOT}/textures/CCTextureETC.cpp"
    "${COCOS_ROOT}/textures/CCTexturePVR.cpp"
    "${COCOS_ROOT}/label_nodes/CCLabelAtlas.cpp"
    "${COCOS_ROOT}/label_nodes/CCLabelBMFont.cpp"
    "${COCOS_ROOT}/platform/CCFileUtils.cpp"
    "${COCOS_ROOT}/platform/CCEGLViewProtocol.cpp"
    "${COCOS_ROOT}/platform/android/CCEGLView.cpp"
    "${COCOS_ROOT}/platform/android/CCAccelerometer.cpp"
    "${COCOS_ROOT}/platform/android/CCFileUtilsAndroid.cpp"
    "${COCOS_ROOT}/platform/android/CCImage.cpp"
    "${COCOS_ROOT}/platform/android/CCCommon.cpp"
    "${COCOS_ROOT}/platform/android/jni/JniHelper.cpp"
    "${COCOS_ROOT}/shaders/ccShaders.cpp"
    "${COCOS_ROOT}/shaders/CCGLProgram.cpp"
    "${COCOS_ROOT}/shaders/ccGLStateCache.cpp"
    "${COCOS_ROOT}/shaders/CCShaderCache.cpp"
    "${COCOS_ROOT}/support/CCPointExtension.cpp"
    "${COCOS_ROOT}/support/TransformUtils.cpp"
    "${COCOS_ROOT}/support/ccUtils.cpp"
    "${COCOS_ROOT}/support/data_support/ccCArray.cpp"
    "${COCOS_ROOT}/kazmath/src/aabb.c"
    "${COCOS_ROOT}/kazmath/src/mat3.c"
    "${COCOS_ROOT}/kazmath/src/mat4.c"
    "${COCOS_ROOT}/kazmath/src/plane.c"
    "${COCOS_ROOT}/kazmath/src/quaternion.c"
    "${COCOS_ROOT}/kazmath/src/ray2.c"
    "${COCOS_ROOT}/kazmath/src/utility.c"
    "${COCOS_ROOT}/kazmath/src/vec2.c"
    "${COCOS_ROOT}/kazmath/src/vec3.c"
    "${COCOS_ROOT}/kazmath/src/vec4.c"
    "${COCOS_ROOT}/kazmath/src/GL/mat4stack.c"
    "${COCOS_ROOT}/kazmath/src/GL/matrix.c"
)

target_include_directories(game PRIVATE
    "${COCOS_ROOT}"
    "${COCOS_ROOT}/include"
    "${COCOS_ROOT}/kazmath/include"
    "${COCOS_ROOT}/platform/android"
    "${COCOS_ROOT}/platform/android/jni"
)

target_compile_definitions(game PRIVATE USE_FILE32API)
target_compile_options(game PRIVATE -Wno-psabi)

find_library(log-lib log)
find_library(android-lib android)
find_library(glesv2-lib GLESv2)
find_library(z-lib z)
find_library(jnigraphics-lib jnigraphics)

target_link_libraries(game
    ${log-lib}
    ${android-lib}
    ${glesv2-lib}
    ${z-lib}
    ${jnigraphics-lib}
    m
    dl
)
