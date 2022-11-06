cmake_minimum_required(VERSION 3.6)   # CMake version check
project(antons_tutorials C CXX)               # Create project "simple_example"
set(CMAKE_CXX_STANDARD 11)            # Enable c++11 standard

#add source folder so the program can be compiled at build folder
include_directories(${CMAKE_SOURCE_DIR})

#Headers
file(GLOB HEADERS
  *.h
  *.cpp
  )

#Main
set(SOURCE_FILES main.cpp)
add_executable(phongtex ${SOURCE_FILES} ${HEADERS})

#OpenGL
find_package(OpenGL REQUIRED)
include_directories(${OPENGL_INCLUDE_DIR})
target_link_libraries(phongtex ${OPENGL_gl_LIBRARY})

#ASSIMP
find_package(Assimp REQUIRED)
include_directories(${ASSIMP_INCLUDE_DIR})
target_link_libraries(phongtex ${ASSIMP_LIBRARIES})


#GLFW
find_package(PkgConfig REQUIRED)
pkg_search_module(GLFW REQUIRED glfw3)
include_directories(${GLFW_INCLUDE_DIRS})
target_link_libraries(phongtex ${GLFW_LIBRARIES})

#GLEW
find_package(GLEW REQUIRED)
if (GLEW_FOUND)
    include_directories(${GLEW_INCLUDE_DIRS})
    target_link_libraries(phongtex ${GLEW_LIBRARIES})
endif()

file (GLOB AUXFILES
	*.glsl
	*.obj
	*.dae
	*.png
	)
foreach (t ${AUXFILES})
	get_filename_component(n ${t} NAME)
	configure_file(${n} ${n} COPYONLY)
endforeach()

