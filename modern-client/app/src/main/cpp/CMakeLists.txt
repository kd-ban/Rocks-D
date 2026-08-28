cmake_minimum_required(VERSION 3.22.1)
project(rocksd_game LANGUAGES C CXX)

add_library(game SHARED game.cpp)

find_library(log-lib log)
target_link_libraries(game ${log-lib})
