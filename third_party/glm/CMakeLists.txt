cmake_minimum_required(VERSION 2.6)

project(glm)

file(GLOB_RECURSE GLM_SRCS
     RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}/glm"
     *.cpp *.h* *.inl)

add_custom_target(glm ${GLM_SRCS})
