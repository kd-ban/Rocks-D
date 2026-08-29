cmake_minimum_required(VERSION 3.22.1)
project(rocksd_game LANGUAGES C CXX)

set(COCOS_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../../../../engine/cocos2d-x-2.1.4/cocos2dx")

add_library(game SHARED
    game.cpp
    "${COCOS_ROOT}/cocos2d.cpp"
)

target_include_directories(game PRIVATE
    "${COCOS_ROOT}"
    "${COCOS_ROOT}/include"
    "${COCOS_ROOT}/kazmath/include"
    "${COCOS_ROOT}/platform/android"
)

find_library(log-lib log)
find_library(android-lib android)
find_library(glesv2-lib GLESv2)

target_link_libraries(game
    ${log-lib}
    ${android-lib}
    ${glesv2-lib}
    m
    dl
)
