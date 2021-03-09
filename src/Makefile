CC = gcc
CFLAGS = `pkg-config fuse --cflags --libs`

main: main.c
	${CC} -o main main.c ${CFLAGS}