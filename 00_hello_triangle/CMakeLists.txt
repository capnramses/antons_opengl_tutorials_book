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

# Load the shader strings from text files called test.vertand test.frag(a naming convention is handy).
# Gerdelan, Anton. Anton's OpenGL 4 Tutorials . Kindle Edition.
list (APPEND SOURCE_FILES_EXP1 main_exp1.c fragment.shader.cmake vertex.shader.cmake ${HEADERS})
add_gl_executable(${TARGET}_exp1 "${SOURCE_FILES_EXP1}")

configure_file(fragment.shader.cmake fragment.shader)
configure_file(vertex.shader.cmake vertex.shader)

list (APPEND SOURCE_FILES_EXP2 main_exp2.c ${HEADERS})
add_gl_executable(${TARGET}_exp2 "${SOURCE_FILES_EXP2}")
