#
#
#

list(APPEND SOURCE_FILES 
	main.c
	gl_utils.c
	gl_utils.h
	test_fs.glsl
	test_vs.glsl
)

get_filename_component(TARGET ${CMAKE_CURRENT_SOURCE_DIR} NAME)
add_gl_executable(${TARGET} "${SOURCE_FILES};${HEADERS}")

configure_file(test_fs.glsl test_fs.glsl)
configure_file(test_vs.glsl test_vs.glsl)
