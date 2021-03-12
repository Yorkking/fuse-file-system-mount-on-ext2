CC = gcc
CFLAGS = `pkg-config fuse --cflags --libs`
_CFLAGS = -I.
OBJ = main.o util.o

	
main: $(OBJ)
	$(CC) -o main main.c $(CFLAGS) util.c $(_CFLAGS)

.PHONY: clean
clean:
	rm -f ./*.o 