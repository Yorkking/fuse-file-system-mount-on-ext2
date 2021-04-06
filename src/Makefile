CC = gcc
FUSE_FLAG = `pkg-config fuse --cflags --libs`
CFLAGS = -I.
PMDK_FLAG = -lpmemobj -lpmem -pthread
OBJ = main.o control.o util.o
	
main: $(OBJ)
	$(CC) -o main $(OBJ) $(PMDK_FLAG) $(FUSE_FLAG) $(CFLAGS)

main.o: main.c control.h
	$(CC) -c main.c $(FUSE_FLAG)
control.o: control.c control.h util.h
	$(CC) -c control.c $(PMDK_FLAG)
util.o: util.c util.h
	$(CC) -c util.c $(PMDK_FLAG)

.PHONY: clean
clean:
	rm -f ./*.o