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
add_executable(extinit ${SOURCE_FILES} ${HEADERS})
if (WIN32)
	target_compile_definitions(extinit PRIVATE  _CRT_SECURE_NO_WARNINGS)
endif()

#OpenGL
find_package(OpenGL REQUIRED)
include_directories(${OPENGL_INCLUDE_DIR})

target_link_libraries(extinit ${OPENGL_gl_LIBRARY})

include_directories(${GLFW_INCLUDE_DIRS})
target_link_libraries(extinit ${GLFW_LIBRARIES})

include_directories(${GLEW_INCLUDE_DIRS})
target_link_libraries(extinit ${GLEW_LIBRARIES})

