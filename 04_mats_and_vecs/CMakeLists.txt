#
#
#

set(HEADERS gl_utils.h maths_funcs.h)

set(SOURCE_FILES main.cpp gl_utils.cpp maths_funcs.cpp)

get_filename_component(TARGET ${CMAKE_CURRENT_SOURCE_DIR} NAME)
add_gl_executable(${TARGET} "${SOURCE_FILES};${HEADERS}")

configure_file(test_fs.glsl test_fs.glsl)
configure_file(test_vs.glsl test_vs.glsl)