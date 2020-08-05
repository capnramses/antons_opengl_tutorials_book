BIN = hotreload
CC = gcc
FLAGS = -Wall -pedantic -DGLEW_STATIC
INC = -I ../common -I ../common/include
LOC_LIB = ../common/GL/glew.c
SYS_LIB = -lglfw -lGL
SRC = main.c

all:
	${CC} ${FLAGS} -o ${BIN} ${SRC} ${INC} ${LOC_LIB} ${SYS_LIB}

