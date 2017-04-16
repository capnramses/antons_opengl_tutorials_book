CC = g++
FLAGS = -Wall -pedantic
INC = -I ../common/include -I ../common/include/freetype
SYS_LIB = -lGL 

all: generator viewer

generator:
	${CC} ${FLAGS} -o generate generator_main.cpp  ${INC} -lfreetype ${SYS_LIB}

viewer:
	${CC} ${FLAGS} -o view viewer_main.cpp maths_funcs.cpp  ${INC} ../common/linux_x86_64/libGLEW.a -lglfw ${SYS_LIB}
