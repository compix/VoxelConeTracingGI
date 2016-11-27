cmake_minimum_required(VERSION 2.6)

project(imgui)

file(GLOB_RECURSE IMGUI_SRCS
     RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}"
     *.cpp *.h* *.inl)

add_library(${PROJECT_NAME} STATIC ${IMGUI_SRCS})