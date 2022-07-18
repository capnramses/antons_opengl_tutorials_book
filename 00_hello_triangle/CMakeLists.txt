#
#
#

#Headers
file(GLOB HEADERS
  *.h
  )

#Main
set(SOURCE_FILES main.c)
add_gl_executable(hellot ${SOURCE_FILES};${HEADERS})
