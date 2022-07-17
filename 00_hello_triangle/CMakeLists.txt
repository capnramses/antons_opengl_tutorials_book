cmake_minimum_required(VERSION 3.6)   # CMake version check
project(antons_tutorials C CXX)               # Create project "simple_example"
set(CMAKE_CXX_STANDARD 11)            # Enable c++11 standard

#add source folder so the program can be compiled at build folder
include_directories(${CMAKE_SOURCE_DIR})

#Headers
file(GLOB HEADERS
  *.h
  )

#Main
set(SOURCE_FILES main.c)
add_executable(hellot ${SOURCE_FILES} ${HEADERS})

#OpenGL
find_package(OpenGL REQUIRED)
include_directories(${OPENGL_INCLUDE_DIR})

target_link_libraries(hellot ${OPENGL_gl_LIBRARY})


#GLFW
if (UNIX)
    find_package(PkgConfig REQUIRED)
    pkg_search_module(GLFW REQUIRED glfw3)
else()
    #find_package(GLFW)
    find_path(GLFW_INCLUDE_DIRS NAMES "GLFW/glfw3.h")
    find_library(GLFW_LIBRARIES NAMES "glfw3.lib")
endif()
include_directories(${GLFW_INCLUDE_DIRS})
target_link_libraries(hellot ${GLFW_LIBRARIES})

#GLEW
#set (GLEW_VERBOSE YES)
#find_package(GLEW )
#if (NOT GLEW_FOUND)
    find_path(GLFW_INCLUDE_DIRS NAMES "GL/glew.h")
    find_path(GLFW_INCLUDE_DIR NAMES "GL/glew.h")
    find_library(GLEW_LIBRARIES NAMES "glew32s.lib") # static
#endif()
include_directories(${GLEW_INCLUDE_DIRS})
target_link_libraries(hellot ${GLEW_LIBRARIES})




