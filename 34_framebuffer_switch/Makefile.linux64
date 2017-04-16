BIN = fbuffer64
CC = g++ -g
FLAGS = -Wall -pedantic -g
INC = -I ../common/include
LOC_LIB = ../common/linux_x86_64/libGLEW.a -lglfw
SYS_LIB = -lGL  -lz
SRC = main.cpp maths_funcs.cpp gl_utils.cpp obj_parser.cpp

all:
	${CC} ${FLAGS} -o ${BIN} ${SRC} ${INC} ${LOC_LIB} ${SYS_LIB}

