CC = gcc
CFLAGS = `pkg-config fuse --cflags --libs`
_CFLAGS = -I.
PMDK_FLAG = -lpmemobj -lpmem -pthread
OBJ = main.o util.o

	
main: $(OBJ)
	$(CC) -o main main.c $(CFLAGS) util.c  $(PMDK_FLAG) $(_CFLAGS) 

.PHONY: clean
clean:
	rm -f ./*.o 