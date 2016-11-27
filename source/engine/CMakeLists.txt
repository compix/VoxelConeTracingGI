cmake_minimum_required(VERSION 2.8)
project(engine)
cmake_policy(SET CMP0015 NEW)

file(GLOB_RECURSE
     SRC_LIST
     RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} 
     *.cpp *.h*)

add_library(${PROJECT_NAME} STATIC ${SRC_LIST})

target_link_libraries(${PROJECT_NAME} imgui soil2 ${ASSIMP_LIBRARY} ${SDL2_LIBRARY} ${OPENGL_LIBRARIES} ${GLEW_LIBRARIES})

# Create source groups for Visual Studio filters
# Get all directories first:
set(dir_list "")
foreach(file_path ${SRC_LIST})
        get_filename_component(dir_path ${file_path} PATH)
        set(dir_list ${dir_list} ${dir_path})
endforeach()
list(REMOVE_DUPLICATES dir_list)

file(GLOB sources *.cpp *.h*)
source_group(\\ FILES ${sources})

foreach(dir ${dir_list})
        set(abs_dir "${CMAKE_CURRENT_SOURCE_DIR}/${dir}")
        file(GLOB sources ${abs_dir}/*.cpp ${abs_dir}/*.h*)
        string(REPLACE "/" "\\\\" Group ${dir})
        source_group(${Group} FILES ${sources})
endforeach()
