cmake_minimum_required(VERSION 2.6)

project(soil2)

SET(SOIL2_SRCS
    etc1_utils.c
    image_DXT.c
    image_helper.c
    SOIL2.c)

add_library(${PROJECT_NAME} STATIC ${SOIL2_SRCS})
target_link_libraries(${PROJECT_NAME} ${OPENGL_LIBRARIES} ${GLEW_LIBRARIES})