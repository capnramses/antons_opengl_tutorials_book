#
#
#

#Headers
file(GLOB HEADERS
  *.h
  )

#Main
set(SOURCE_FILES main.c)
get_filename_component(TARGET ${CMAKE_CURRENT_SOURCE_DIR} NAME)
add_gl_executable(${TARGET} ${SOURCE_FILES};${HEADERS})
