cmake_minimum_required(VERSION 3.22.1)
project(rocksd_game LANGUAGES C CXX)

file(GLOB LUA_SOURCES "lua/*.c")
list(FILTER LUA_SOURCES EXCLUDE REGEX ".*/lua\.c$")
list(FILTER LUA_SOURCES EXCLUDE REGEX ".*/luac\.c$")

add_library(game SHARED game.cpp ${LUA_SOURCES})
target_include_directories(game PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/lua)

target_compile_definitions(game PRIVATE LUA_USE_POSIX)
find_library(log-lib log)
find_library(android-lib android)
target_link_libraries(game ${log-lib} ${android-lib} m dl)
