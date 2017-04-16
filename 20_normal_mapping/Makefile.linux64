BIN = nmap
CC = g++
FLAGS = -Wall -pedantic
INC = -I ../common/include
LOC_LIB = ../common/linux_x86_64/libGLEW.a -lglfw ../common/linux_x86_64/libassimp.a
SYS_LIB = -lGL  -lz
SRC = main.cpp maths_funcs.cpp gl_utils.cpp 

all:
	${CC} ${FLAGS} -o ${BIN} ${SRC} ${INC} ${LOC_LIB} ${SYS_LIB}

