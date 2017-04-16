BIN = vcam
CC = g++
FLAGS = -Wall -pedantic
INC = -I ../common/include
LOC_LIB = ../common/linux_x86_64/libGLEW.a -lglfw
SYS_LIB = -lGL
SRC = main.cpp gl_utils.cpp maths_funcs.cpp

all:
	${CC} ${FLAGS} -o ${BIN} ${SRC} ${INC} ${LOC_LIB} ${SYS_LIB}
